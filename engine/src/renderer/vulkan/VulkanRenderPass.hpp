#pragma once

#include "VulkanSwapchain.hpp"
#include "VulkanCommandBuffer.hpp"

#include <vulkan/vulkan.h>

#include <memory>

namespace beige {
namespace renderer {
namespace vulkan {

class RenderPass final {
public:
    enum class State : uint32_t {
        Ready,
        Recording,
        InRenderPass,
        RecordingEnded,
        Submitted,
        NotAllocated
    };

    RenderPass(
        VkAllocationCallbacks* allocationCallbacks,
        std::shared_ptr<VulkanSwapchain> swapchain,
        std::shared_ptr<VulkanDevice> device,
        const float x,
        const float y,
        const float w,
        const float h,
        const float r,
        const float g,
        const float b,
        const float a,
        const float depth,
        const uint32_t stencil
    );

    ~RenderPass();

    auto getRenderPass() const -> const VkRenderPass&;
    auto setX(const float x) -> void;
    auto setY(const float y) -> void;
    auto setW(const float w) -> void;
    auto setH(const float h) -> void;

    auto begin(
        std::shared_ptr<CommandBuffer> commandBuffer,
        const VkFramebuffer& framebuffer
    ) -> void;
    auto end(std::shared_ptr<CommandBuffer> commandBuffer) -> void;

private:
    VkAllocationCallbacks* m_allocationCallbacks;
    std::shared_ptr<VulkanSwapchain> m_swapchain;
    std::shared_ptr<VulkanDevice> m_device;

    VkRenderPass m_renderPass;
    float m_x;
    float m_y;
    float m_w;
    float m_h;
    float m_r;
    float m_g;
    float m_b;
    float m_a;

    float m_depth;
    uint32_t m_stencil;

    State m_state;
};

} // namespace vulkan
} // namespace renderer
} // namespace beige
