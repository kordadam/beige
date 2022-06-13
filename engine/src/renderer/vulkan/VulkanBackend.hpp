#pragma once

#include "../IRendererBackend.hpp"

#include "VulkanDevice.hpp"

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
    VkAllocationCallbacks* m_allocationCallbacks;
    VkInstance m_instance;
    VkSurfaceKHR m_surface;

#if defined(BEIGE_DEBUG)
    VkDebugUtilsMessengerEXT m_debugUtilsMessenger;
#endif

    std::unique_ptr<VulkanDevice> m_device;
};

} // namespace vulkan
} // namespace renderer
} // namespace beige
