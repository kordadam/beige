#pragma once

#include "../IRendererBackend.hpp"

#include "VulkanDevice.hpp"
#include "VulkanSwapchain.hpp"
#include "VulkanRenderPass.hpp"

namespace beige {
namespace renderer {
namespace vulkan {

class VulkanBackend final : public IRendererBackend {
public:
    VulkanBackend(
        const std::string& appName,
        std::shared_ptr<platform::Platform> platform
    );

    ~VulkanBackend();

    auto onResized(const uint16_t width, const uint16_t height) -> void override;
    auto beginFrame(const float deltaTime) -> bool override;
    auto endFrame(const float deltaTime) -> bool override;
    auto drawFrame(const Packet& packet) -> bool override;

private:
    uint32_t m_framebufferWidth;
    uint32_t m_framebufferHeight;
    VkAllocationCallbacks* m_allocationCallbacks;
    VkInstance m_instance;
    VkSurfaceKHR m_surface;

#if defined(BEIGE_DEBUG)
    VkDebugUtilsMessengerEXT m_debugUtilsMessenger;
#endif // BEIGE_DEBUG

    std::shared_ptr<VulkanDevice> m_device;
    std::shared_ptr<VulkanSwapchain> m_swapchain;
    std::shared_ptr<RenderPass> m_mainRenderPass;
    std::vector<std::shared_ptr<CommandBuffer>> m_graphicsCommandBuffers;

    auto createCommandBuffers() -> void;
};

} // namespace vulkan
} // namespace renderer
} // namespace beige
