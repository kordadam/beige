#pragma once

#include "VulkanDevice.hpp"

#include "VulkanImage.hpp"

#include <vulkan/vulkan.h>

#include <vector>
#include <optional>
#include <memory>

namespace beige {
namespace renderer {
namespace vulkan {

class VulkanSwapchain final {
public:
    VulkanSwapchain(
        const uint32_t framebufferWidth,
        const uint32_t framebufferHeight,
        const VkSurfaceKHR& surface,
        VkAllocationCallbacks* allocationCallbacks,
        std::shared_ptr<VulkanDevice> device
    );
    ~VulkanSwapchain();

    auto getSurfaceFormat() const -> const VkSurfaceFormatKHR&;

    auto recreate() -> void;

    auto acquireNextImageIndex(
        const uint64_t timeoutInNs,
        const VkSemaphore& imageAvailableSemaphore,
        const VkFence& fence
    ) -> std::optional<uint32_t>;

    auto present(
        const VkQueue& graphicsQueue,
        const VkQueue& presentQueue,
        const VkSemaphore& renderCompleteSemaphore,
        const uint32_t presentImageIndex
    ) -> void;

private:
    uint32_t m_framebufferWidth;
    uint32_t m_framebufferHeight;
    VkSurfaceKHR m_surface;
    VkAllocationCallbacks* m_allocationCallbacks;
    std::shared_ptr<VulkanDevice> m_device;
    VkSurfaceFormatKHR m_surfaceFormat;
    uint32_t m_maxFramesInFlight;
    VkSwapchainKHR m_swapchain;
    std::vector<VkImage> m_images;
    std::vector<VkImageView> m_imageViews;
    std::unique_ptr<Image> m_depthAttachment;

    uint32_t m_imageIndex;
    uint32_t m_currentFrame;
    bool m_recreatingSwapchain;

    auto create() -> void;
    auto destroy() -> void;
};

} // namespace vulkan
} // namespace renderer
} // namespace beige
