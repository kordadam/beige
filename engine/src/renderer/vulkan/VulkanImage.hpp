#pragma once

#include "VulkanDevice.hpp"
#include "VulkanCommandBuffer.hpp"

#include <vulkan/vulkan.h>

#include <memory>

namespace beige {
namespace renderer {
namespace vulkan {

class Image final {
public:
    Image(
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
    );
    ~Image();

    auto getImageView() const -> const VkImageView&;

    auto transitionLayout(
        const VkCommandBuffer& commandBuffer,
        const VkFormat& format,
        const VkImageLayout& oldLayout,
        const VkImageLayout& newLayout
    ) -> void;

    auto copyFromBuffer(
        const VkBuffer& buffer,
        const VkCommandBuffer& commandBuffer
    ) -> void;

private:
    VkAllocationCallbacks* m_allocationCallbacks;
    std::shared_ptr<Device> m_device;

    VkImage m_handle;
    VkDeviceMemory m_deviceMemory;
    VkImageView m_imageView;
    uint32_t m_width;
    uint32_t m_height;

    auto createImageView(
        const VkFormat& format,
        const VkImageAspectFlags& imageAspectFlags
    ) -> void;
};

} // namespace vulkan
} // namespace renderer
} // namespace beige
