#pragma once

#include "../IRendererBackend.hpp"

#include "VulkanSurface.hpp"
#include "VulkanDevice.hpp"
#include "VulkanSwapchain.hpp"
#include "VulkanRenderPass.hpp"
#include "VulkanFramebuffer.hpp"
#include "VulkanFence.hpp"
#include "VulkanBuffer.hpp"
#include "shaders/VulkanShaderObject.hpp"

namespace beige {
namespace renderer {
namespace vulkan {

class Backend final : public IBackend {
public:
    Backend(
        const std::string& appName,
        const uint32_t width,
        const uint32_t height,
        std::shared_ptr<platform::Platform> platform
    );

    ~Backend();

    auto onResized(const uint16_t width, const uint16_t height) -> void override;
    auto beginFrame(const float deltaTime) -> bool override;
    auto updateGlobalState(
        const math::Matrix4x4& projection,
        const math::Matrix4x4& view,
        const math::Vector3& viewPosition,
        const math::Vector4& ambientColor,
        const int32_t mode
    ) -> void override;
    auto endFrame(const float deltaTime) -> bool override;
    auto updateObject(
        const math::Matrix4x4& model
    ) -> void override;

private:
    std::shared_ptr<platform::Platform> m_platform;

    uint32_t m_framebufferWidth;
    uint32_t m_framebufferHeight;
    uint64_t m_framebufferSizeGeneration;
    uint64_t m_framebufferSizeLastGeneration;

    VkAllocationCallbacks* m_allocationCallbacks;
    VkInstance m_instance;

#if defined(BEIGE_DEBUG)
    VkDebugUtilsMessengerEXT m_debugUtilsMessenger;
#endif // BEIGE_DEBUG

    std::shared_ptr<Surface> m_surface;
    std::shared_ptr<Device> m_device;
    std::shared_ptr<Swapchain> m_swapchain;
    bool m_recreatingSwapchain;
    std::shared_ptr<RenderPass> m_mainRenderPass;

    std::unique_ptr<Buffer> m_objectVertexBuffer;
    std::unique_ptr<Buffer> m_objectIndexBuffer;

    uint32_t m_imageIndex;
    std::vector<std::shared_ptr<Framebuffer>> m_framebuffers; // Framebuffers used for on-screen rendering
    std::vector<std::shared_ptr<CommandBuffer>> m_graphicsCommandBuffers;
    std::shared_ptr<ShaderObject> m_shaderObject;

    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_queueCompleteSemaphores;
    std::vector<std::shared_ptr<Fence>> m_inFlightFences;
    std::vector<std::shared_ptr<Fence>> m_imagesInFlight; // Holds pointers to fences which exist and are owned elsewhere

    uint64_t m_geometryVertexOffset;
    uint64_t m_geometryIndexOffset;

    auto regenerateFramebuffers() -> void;
    auto createCommandBuffers() -> void;
    auto recreateSwapchain() -> bool;
    auto createBuffers() -> void;
    auto uploadDataRange(
        const VkCommandPool& commandPool,
        const VkFence& fence,
        const VkQueue& queue,
        const VkBuffer& buffer,
        const uint64_t offset,
        const uint64_t size,
        void* data
    ) -> void;
};

} // namespace vulkan
} // namespace renderer
} // namespace beige
