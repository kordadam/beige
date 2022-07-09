#include "VulkanImage.hpp"

#include "VulkanDefines.hpp"

namespace beige {
namespace renderer {
namespace vulkan {

Image::Image(
    VkAllocationCallbacks* allocationCallbacks,
    std::shared_ptr<Device> device,
    const VkImageType& imageType,
    const uint32_t width,
    const uint32_t height,
    const VkFormat& format,
    const VkImageTiling& imageTiling,
    const VkImageUsageFlags& imageUsageFlags,
    const VkMemoryPropertyFlags& memoryPropertyFlags,
    const bool createView,
    const VkImageAspectFlags& imageAspectFlags
) :
m_allocationCallbacks { allocationCallbacks },
m_device { device },
m_image { 0 },
m_deviceMemory { 0 },
m_imageView { 0 },
m_width { width },
m_height { height } {
    const VkExtent3D extent {
        width, // width
        height, // height
        1u // depth // TODO: Support configurable depth
    };

    const VkImageCreateInfo imageCreateInfo {
        VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, // sType
        nullptr, // pNext
        0u, // flags
        VK_IMAGE_TYPE_2D, // imageType
        format, // format
        extent, // extent
        4u, // mipLevels // TODO: Support mip mapping
        1u, // arrayLayers // TODO: Support number of layers in the image
        VK_SAMPLE_COUNT_1_BIT, // samples // TODO: Configurable sample count
        imageTiling, // tiling
        imageUsageFlags, // usage
        VK_SHARING_MODE_EXCLUSIVE, // sharingMode // TODO: Configurable sharing mode
        0u, // queueFamilyIndexCount
        nullptr, // pQueueFamilyIndices
        VK_IMAGE_LAYOUT_UNDEFINED // initialLayout
    };

    const VkDevice logicalDevice { m_device->getLogicalDevice() };

    VULKAN_CHECK(
        vkCreateImage(
            logicalDevice,
            &imageCreateInfo,
            m_allocationCallbacks,
            &m_image
        )
    );

    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(
        logicalDevice,
        m_image,
        &memoryRequirements
    );

    std::optional<uint32_t> memoryTypeIndex{ m_device->findMemoryIndex(memoryRequirements.memoryTypeBits, memoryPropertyFlags) };
    
    if (!memoryTypeIndex.has_value()) {
        core::Logger::error("Required memory type not found, image not valid!");
    }

    const VkMemoryAllocateInfo memoryAllocateInfo {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, // sType
        nullptr, // pNext
        memoryRequirements.size, // allocationSize
        memoryTypeIndex.value() // memoryTypeIndex
    };

    VULKAN_CHECK(
        vkAllocateMemory(
            logicalDevice,
            &memoryAllocateInfo,
            m_allocationCallbacks,
            &m_deviceMemory
        )
    );

    VULKAN_CHECK(
        vkBindImageMemory(
            logicalDevice,
            m_image,
            m_deviceMemory,
            0u // TODO: Configurable memory offset
        )
    );

    if (createView) {
        createImageView(format, imageAspectFlags);
    }
}

Image::~Image() {
    const VkDevice logicalDevice { m_device->getLogicalDevice() };

    if (m_imageView != 0) {
        vkDestroyImageView(
            logicalDevice,
            m_imageView,
            m_allocationCallbacks
        );
    }

    if (m_deviceMemory != 0) {
        vkFreeMemory(
            logicalDevice,
            m_deviceMemory,
            m_allocationCallbacks
        );
    }

    if (m_image != 0) {
        vkDestroyImage(
            logicalDevice,
            m_image,
            m_allocationCallbacks
        );
    }
}

auto Image::getImageView() const -> const VkImageView& {
    return m_imageView;
}

auto Image::createImageView(
    const VkFormat& format,
    const VkImageAspectFlags& imageAspectFlags
) -> void {
    const VkImageSubresourceRange imageSubresourceRange {
        imageAspectFlags, // aspectMask
        0u, // baseMipLevel // TODO: Make configurable
        1u, // levelCount // TODO: Make configurable
        0u, // baseArrayLayer // TODO: Make configurable
        1u // layerCount // TODO: Make configurable
    };

    const VkImageViewCreateInfo imageViewCreateInfo {
        VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, // sType
        nullptr, // pNext
        0u, // flags
        m_image, // image
        VK_IMAGE_VIEW_TYPE_2D, // TODO: Make configurable
        format, // format
        { }, // components
        imageSubresourceRange // subresourceRange
    };

    VULKAN_CHECK(
        vkCreateImageView(
            m_device->getLogicalDevice(),
            &imageViewCreateInfo,
            m_allocationCallbacks,
            &m_imageView
        )
    );
}

} // namespace vulkan
} // namespace renderer
} // namespace beige
