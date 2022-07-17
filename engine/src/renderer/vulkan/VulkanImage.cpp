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
m_handle { VK_NULL_HANDLE },
m_deviceMemory { VK_NULL_HANDLE },
m_imageView { VK_NULL_HANDLE },
m_width { width },
m_height { height } {
    const VkExtent3D extent {
        width,  // width
        height, // height
        1u      // depth // TODO: Support configurable depth
    };

    const VkImageCreateInfo imageCreateInfo {
        VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, // sType
        nullptr,                             // pNext
        0u,                                  // flags
        VK_IMAGE_TYPE_2D,                    // imageType
        format,                              // format
        extent,                              // extent
        4u,                                  // mipLevels // TODO: Support mip mapping
        1u,                                  // arrayLayers // TODO: Support number of layers in the image
        VK_SAMPLE_COUNT_1_BIT,               // samples // TODO: Configurable sample count
        imageTiling,                         // tiling
        imageUsageFlags,                     // usage
        VK_SHARING_MODE_EXCLUSIVE,           // sharingMode // TODO: Configurable sharing mode
        0u,                                  // queueFamilyIndexCount
        nullptr,                             // pQueueFamilyIndices
        VK_IMAGE_LAYOUT_UNDEFINED            // initialLayout
    };

    const VkDevice logicalDevice { m_device->getLogicalDevice() };

    VULKAN_CHECK(
        vkCreateImage(
            logicalDevice,
            &imageCreateInfo,
            m_allocationCallbacks,
            &m_handle
        )
    );

    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(
        logicalDevice,
        m_handle,
        &memoryRequirements
    );

    std::optional<uint32_t> memoryTypeIndex {
        m_device->findMemoryIndex(memoryRequirements.memoryTypeBits, memoryPropertyFlags)
    };
    
    if (!memoryTypeIndex.has_value()) {
        core::Logger::error("Required memory type not found, image not valid!");
    }

    const VkMemoryAllocateInfo memoryAllocateInfo {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, // sType
        nullptr,                                // pNext
        memoryRequirements.size,                // allocationSize
        memoryTypeIndex.value()                 // memoryTypeIndex
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
            m_handle,
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

    if (m_imageView != VK_NULL_HANDLE) {
        vkDestroyImageView(
            logicalDevice,
            m_imageView,
            m_allocationCallbacks
        );
    }

    if (m_deviceMemory != VK_NULL_HANDLE) {
        vkFreeMemory(
            logicalDevice,
            m_deviceMemory,
            m_allocationCallbacks
        );
    }

    if (m_handle != VK_NULL_HANDLE) {
        vkDestroyImage(
            logicalDevice,
            m_handle,
            m_allocationCallbacks
        );
    }
}

auto Image::getImageView() const -> const VkImageView& {
    return m_imageView;
}

auto Image::transitionLayout(
    const VkCommandBuffer& commandBuffer,
    const VkFormat& format,
    const VkImageLayout& oldLayout,
    const VkImageLayout& newLayout
) -> void {
    const VkImageSubresourceRange imageSubresourceRange {
        VK_IMAGE_ASPECT_COLOR_BIT, // aspectMask
        0u,                        // baseMipLevel
        1u,                        // levelCount
        0u,                        // baseArrayLayer
        1u                         // layerCount
    };

    VkImageMemoryBarrier imageMemoryBarrier {
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,    // sType
        nullptr,                                   // pNext
        VK_ACCESS_NONE,                            // srcAccessMask
        VK_ACCESS_NONE,                            // dstAccessMask
        oldLayout,                                 // oldLayout
        newLayout,                                 // newLayout
        m_device->getGraphicsQueueIndex().value(), // srcQueueFamilyIndex
        m_device->getGraphicsQueueIndex().value(), // dstQueueFamilyIndex
        m_handle,                                  // image
        imageSubresourceRange,                     // subresourceRange
    };

    VkPipelineStageFlags srcStage;
    VkPipelineStageFlags dstStage;

    // Don't care about old layout - transition to optimal layout (for underlying implementation).
    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_NONE;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        // Don't care what stage the pipeline is in at the start.
        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

        // Used for copying.
        dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        // Transitioning from a transfer destination layout to a shader-readonly layout.
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        // From a copying stage to...
        srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

        // The fragment stage.
        dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        core::Logger::fatal("Unsupported layout transition!");
        return;
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        srcStage,
        dstStage,
        0u,
        0u,
        nullptr,
        0u,
        nullptr,
        1u,
        &imageMemoryBarrier
    );
}

auto Image::copyFromBuffer(
    const VkBuffer& buffer,
    const VkCommandBuffer& commandBuffer
) -> void {
    // Region to copy.
    const VkImageSubresourceLayers imageSubresourceLayers {
        VK_IMAGE_ASPECT_COLOR_BIT, // aspectMask
        0u,                        // mipLevel
        0u,                        // baseArrayLayer
        1u                         // layerCount
    };

    const VkOffset3D offset {
        0, // x
        0, // y
        0  // z
    };

    const VkExtent3D extent {
        m_width,  // width
        m_height, // height
        1u        // depth
    };

    const VkBufferImageCopy bufferImageCopy {
        0u,                     // bufferOffset
        0u,                     // bufferRowLength
        0u,                     // bufferImageHeight
        imageSubresourceLayers, // imageSubresource
        offset,                 // imageOffset
        extent                  // imageExtent
    };

    vkCmdCopyBufferToImage(
        commandBuffer,
        buffer,
        m_handle,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1u,
        &bufferImageCopy
    );
}

auto Image::createImageView(
    const VkFormat& format,
    const VkImageAspectFlags& imageAspectFlags
) -> void {
    const VkImageSubresourceRange imageSubresourceRange {
        imageAspectFlags, // aspectMask
        0u,               // baseMipLevel // TODO: Make configurable
        1u,               // levelCount // TODO: Make configurable
        0u,               // baseArrayLayer // TODO: Make configurable
        1u                // layerCount // TODO: Make configurable
    };

    const VkImageViewCreateInfo imageViewCreateInfo {
        VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, // sType
        nullptr,                                  // pNext
        0u,                                       // flags
        m_handle,                                 // image
        VK_IMAGE_VIEW_TYPE_2D,                    // TODO: Make configurable
        format,                                   // format
        { },                                      // components
        imageSubresourceRange                     // subresourceRange
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
