#include "VulkanSwapchain.hpp"

#include "../../core/Logger.hpp"
#include "VulkanDefines.hpp"

#include <algorithm>
#include <array>

namespace beige {
namespace renderer {
namespace vulkan {

Swapchain::Swapchain(
    const uint32_t width,
    const uint32_t height,
    VkAllocationCallbacks* allocationCallbacks,
    std::shared_ptr<Surface> surface,
    std::shared_ptr<Device> device
) :
m_allocationCallbacks { allocationCallbacks },
m_surface{ surface },
m_device { device },
m_surfaceFormat { },
m_maxFramesInFlight { 0u },
m_swapchain { 0 },
m_images { },
m_imageViews { },
m_imageIndex { 0u },
m_currentFrame { 0u },
m_depthAttachment { nullptr } {
    // Simply create a new one
    create(width, height);
}

Swapchain::~Swapchain() {
    destroy();
}

auto Swapchain::getSurfaceFormat() const -> const VkSurfaceFormatKHR& {
    return m_surfaceFormat;
}

auto Swapchain::getImages() const -> const std::vector<VkImage>& {
    return m_images;
}

auto Swapchain::getImageViews() const -> const std::vector<VkImageView>& {
    return m_imageViews;
}

auto Swapchain::getDepthAttachment() const -> const std::shared_ptr<Image>& {
    return m_depthAttachment;
}

auto Swapchain::getMaxFramesInFlight() const -> const uint32_t {
    return m_maxFramesInFlight;
}

auto Swapchain::getCurrentFrame() const -> const uint32_t {
    return m_currentFrame;
}

auto Swapchain::recreate(const uint32_t width, const uint32_t height) -> void {
    destroy();
    create(width, height);
}

auto Swapchain::acquireNextImageIndex(
    const uint32_t width,
    const uint32_t height,
    const uint64_t timeoutInNs,
    const VkSemaphore& imageAvailableSemaphore,
    const VkFence& fence
) -> std::optional<uint32_t> {
    uint32_t imageIndex { 0u };

    const VkResult result {
        vkAcquireNextImageKHR(
            m_device->getLogicalDevice(),
            m_swapchain,
            timeoutInNs,
            imageAvailableSemaphore,
            fence,
            &imageIndex
        )
    };

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        // Trigger swapchain recreation, then boot out of the render loop
        recreate(width, height);
        return std::nullopt;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        core::Logger::fatal("Failed to acquire swapchain image!");
        return std::nullopt;
    }

    return std::optional<uint32_t>(imageIndex);
}

auto Swapchain::present(
    const uint32_t width,
    const uint32_t height,
    const VkQueue& graphicsQueue,
    const VkQueue& presentQueue,
    const VkSemaphore& renderCompleteSemaphore,
    const uint32_t presentImageIndex
) -> void {
    // Return the image to the swapchain for presentation
    VkPresentInfoKHR presentInfo {
        VK_STRUCTURE_TYPE_PRESENT_INFO_KHR, // sType
        nullptr, // pNext
        1u, // waitSemaphoreCount
        &renderCompleteSemaphore, // pWaitSemaphores
        1u, // swapchainCount
        &m_swapchain, // pSwapchains
        &presentImageIndex, // pImageIndices
        nullptr // pResults
    };

    const VkResult result { vkQueuePresentKHR(presentQueue, &presentInfo) };

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        // Swapchain is out of date, suboptimal or a framebuffer resize has occured, trigger swapchain recreation
        recreate(width, height);
    } else if (result != VK_SUCCESS) {
        core::Logger::fatal("Failed to present swap chain image!");
    }

    // Increment and loop the index
    m_currentFrame = (m_currentFrame + 1u) % m_maxFramesInFlight;
}

auto Swapchain::create(const uint32_t width, const uint32_t height) -> void {
    VkExtent2D imageExtent { width, height };
    m_maxFramesInFlight = 2u;

    const Device::SwapchainSupport swapchainSupport {
        m_device->getSwapchainSupport()
    };

    const std::vector<VkSurfaceFormatKHR>::const_iterator surfaceFormat {
        std::find_if(
            swapchainSupport.surfaceFormats.begin(),
            swapchainSupport.surfaceFormats.end(),
            [](const VkSurfaceFormatKHR& surfaceFormat) -> bool {
                return (
                    surfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM &&
                    surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
                );
            }
        )
    };

    m_surfaceFormat =
        surfaceFormat != swapchainSupport.surfaceFormats.end()
        ? *surfaceFormat
        : swapchainSupport.surfaceFormats.front();

    const VkPresentModeKHR presentMode {
        std::find_if(
            swapchainSupport.presentModes.begin(),
            swapchainSupport.presentModes.end(),
            [](const VkPresentModeKHR& presentMode) -> bool {
                return presentMode == VK_PRESENT_MODE_MAILBOX_KHR;
            }
        ) != swapchainSupport.presentModes.end()
        ? VK_PRESENT_MODE_MAILBOX_KHR
        : VK_PRESENT_MODE_FIFO_KHR
    };

    // Requery swapchain support
    m_device->querySwapchainSupport(m_device->getPhysicalDevice());

    if (swapchainSupport.surfaceCapabilities.currentExtent.width != UINT32_MAX) {
        imageExtent = swapchainSupport.surfaceCapabilities.currentExtent;
    }

    const VkExtent2D minImageExtent { swapchainSupport.surfaceCapabilities.minImageExtent };
    const VkExtent2D maxImageExtent { swapchainSupport.surfaceCapabilities.maxImageExtent };

    imageExtent.width = std::clamp<uint32_t>(imageExtent.width, minImageExtent.width, maxImageExtent.width);
    imageExtent.height = std::clamp<uint32_t>(imageExtent.height, minImageExtent.height, maxImageExtent.height);

    const uint32_t minImageCount {
        swapchainSupport.surfaceCapabilities.maxImageCount > 0u &&
        swapchainSupport.surfaceCapabilities.minImageCount + 1u > swapchainSupport.surfaceCapabilities.maxImageCount
        ? swapchainSupport.surfaceCapabilities.maxImageCount
        : swapchainSupport.surfaceCapabilities.minImageCount + 1u
    };

    VkSwapchainCreateInfoKHR swapchainCreateInfo { };
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.surface = m_surface->getHandle();
    swapchainCreateInfo.minImageCount = minImageCount;
    swapchainCreateInfo.imageFormat = m_surfaceFormat.format;
    swapchainCreateInfo.imageColorSpace = m_surfaceFormat.colorSpace;
    swapchainCreateInfo.imageExtent = imageExtent;
    swapchainCreateInfo.imageArrayLayers = 1u;
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    const uint32_t graphicsQueueIndex { m_device->getGraphicsQueueIndex().value() };
    const uint32_t presentQueueIndex { m_device->getPresentQueueIndex().value() };

    if (graphicsQueueIndex != presentQueueIndex) {
        const std::array<uint32_t, 2u> queueFamilyIndices { graphicsQueueIndex, presentQueueIndex };
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainCreateInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilyIndices.size());
        swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices.data();
    } else {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchainCreateInfo.queueFamilyIndexCount = 0u;
        swapchainCreateInfo.pQueueFamilyIndices = nullptr;
    }

    swapchainCreateInfo.preTransform = swapchainSupport.surfaceCapabilities.currentTransform;
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfo.presentMode = presentMode;
    swapchainCreateInfo.clipped = VK_TRUE;
    swapchainCreateInfo.oldSwapchain = 0;

    const VkDevice logicalDevice { m_device->getLogicalDevice() };

    VULKAN_CHECK(
        vkCreateSwapchainKHR(
            logicalDevice,
            &swapchainCreateInfo,
            m_allocationCallbacks,
            &m_swapchain
        )
    );

    // Start with a zero frame index
    m_currentFrame = 0u;

    // Images
    uint32_t imageCount { 0u };
    VULKAN_CHECK(
        vkGetSwapchainImagesKHR(
            logicalDevice,
            m_swapchain,
            &imageCount,
            nullptr
        )
    );

    m_images.resize(imageCount);

    VULKAN_CHECK(
        vkGetSwapchainImagesKHR(
            logicalDevice,
            m_swapchain,
            &imageCount,
            m_images.data()
        )
    );

    m_imageViews.resize(imageCount);

    for (size_t i { 0u }; i < imageCount; i++) {
        const VkImageSubresourceRange imageSubresourceRange {
            VK_IMAGE_ASPECT_COLOR_BIT, // aspectMask
            0u, // baseMipLevel
            1u, // levelCount
            0u, // baseArrayLayer
            1u // layerCount
        };

        const VkImageViewCreateInfo imageViewCreateInfo {
            VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, // sType
            nullptr, // pNext
            0u, // flags
            m_images.at(i), // image
            VK_IMAGE_VIEW_TYPE_2D, // viewType
            m_surfaceFormat.format, // format
            { }, // components
            imageSubresourceRange // subresourceRange
        };

        VULKAN_CHECK(
            vkCreateImageView(
                logicalDevice,
                &imageViewCreateInfo,
                m_allocationCallbacks,
                &m_imageViews.at(i)
            )
        );
    }

    // Depth resources
    m_device->detectDepthFormat();

    // Create depth image and view
    m_depthAttachment = std::make_shared<Image>(
        m_allocationCallbacks,
        m_device,
        VK_IMAGE_TYPE_2D,
        imageExtent.width,
        imageExtent.height,
        m_device->getDepthFormat(),
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        true,
        VK_IMAGE_ASPECT_DEPTH_BIT
    );

    core::Logger::info("Swapchain created successfully!");
}

auto Swapchain::destroy() -> void {
    const VkDevice logicalDevice { m_device->getLogicalDevice() };

    vkDeviceWaitIdle(logicalDevice);

    m_depthAttachment.reset();

    std::for_each(
        m_imageViews.begin(),
        m_imageViews.end(),
        [&](const VkImageView& imageView) -> void {
            vkDestroyImageView(
                logicalDevice,
                imageView,
                m_allocationCallbacks
            );
        }
    );

    vkDestroySwapchainKHR(
        logicalDevice,
        m_swapchain,
        m_allocationCallbacks
    );
}

} // namespace vulkan
} // namespace renderer
} // namespace beige
