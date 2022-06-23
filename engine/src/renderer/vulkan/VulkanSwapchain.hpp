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

class Swapchain final {
public:
    Swapchain(
        const uint32_t width,
        const uint32_t height,
        VkAllocationCallbacks* allocationCallbacks,
        std::shared_ptr<Surface> surface,
        std::shared_ptr<Device> device
    );
    ~Swapchain();

    auto getSurfaceFormat() const -> const VkSurfaceFormatKHR&;
    auto getImages() const -> const std::vector<VkImage>&;
    auto getImageViews() const -> const std::vector<VkImageView>&;
    auto getDepthAttachment() const -> const std::shared_ptr<Image>&;
    auto getMaxFramesInFlight() const -> const uint32_t;
    auto getCurrentFrame() const -> const uint32_t;

    auto recreate(const uint32_t width, const uint32_t height) -> void;

    auto acquireNextImageIndex(
        const uint32_t width,
        const uint32_t height,
        const uint64_t timeoutInNs,
        const VkSemaphore& imageAvailableSemaphore,
        const VkFence& fence
    ) -> std::optional<uint32_t>;

    auto present(
        const uint32_t width,
        const uint32_t height,
        const VkQueue& graphicsQueue,
        const VkQueue& presentQueue,
        const VkSemaphore& renderCompleteSemaphore,
        const uint32_t presentImageIndex
    ) -> void;

private:
    VkAllocationCallbacks* m_allocationCallbacks;
    std::shared_ptr<Surface> m_surface;
    std::shared_ptr<Device> m_device;

    VkSurfaceFormatKHR m_surfaceFormat;
    uint32_t m_maxFramesInFlight;
    VkSwapchainKHR m_swapchain;
    std::vector<VkImage> m_images;
    std::vector<VkImageView> m_imageViews;
    std::shared_ptr<Image> m_depthAttachment;

    uint32_t m_imageIndex;
    uint32_t m_currentFrame;

    auto create(const uint32_t width, const uint32_t height) -> void;
    auto destroy() -> void;
};

} // namespace vulkan
} // namespace renderer
} // namespace beige
