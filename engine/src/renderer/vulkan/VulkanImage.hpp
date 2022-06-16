#pragma once

#include "VulkanDevice.hpp"

#include <vulkan/vulkan.h>

#include <memory>

namespace beige {
namespace renderer {
namespace vulkan {

class Image final {
public:
    Image(
        VkAllocationCallbacks* allocationCallbacks,
        std::shared_ptr<VulkanDevice> device,
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

private:
    VkAllocationCallbacks* m_allocationCallbacks;
    std::shared_ptr<VulkanDevice> m_device;

    VkImage m_image;
    VkDeviceMemory m_deviceMemory;
    VkImageView m_imageView;
    uint32_t m_width;
    uint32_t m_height;

    auto createImageView(
        const VkFormat& format,
        const VkImageAspectFlags& imageAspectFlags
    ) -> void;

    auto findMemoryIndex(
        const uint32_t typeFilter,
        const uint32_t propertyFlags
    ) -> int32_t;
};

} // namespace vulkan
} // namespace renderer
} // namespace beige
