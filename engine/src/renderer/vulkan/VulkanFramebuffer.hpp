#pragma once

#include "VulkanRenderPass.hpp"

#include <vulkan/vulkan.h>

#include <memory>

namespace beige {
namespace renderer {
namespace vulkan {

class Framebuffer final {
public:
    Framebuffer(
        VkAllocationCallbacks* allocationCallbacks,
        std::shared_ptr<VulkanDevice> device,
        std::shared_ptr<RenderPass> renderPass,
        const uint32_t width,
        const uint32_t height,
        const std::vector<VkImageView>& imageViews
    );

    ~Framebuffer();

private:
    VkAllocationCallbacks* m_allocationCallbacks;
    std::shared_ptr<VulkanDevice> m_device;
    std::shared_ptr<RenderPass> m_renderPass;

    VkFramebuffer m_framebuffer;
    std::vector<VkImageView> m_imageViews;
};

} // namespace vulkan
} // namespace renderer
} // namespace beige
