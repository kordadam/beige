#pragma once

#include "../IRendererBackend.hpp"

#include "VulkanTypes.hpp"

namespace beige {
namespace renderer {
namespace vulkan {

class VulkanBackend final : public IRendererBackend {
public:
    VulkanBackend(const std::string& appName, platform::Platform& platform);
    ~VulkanBackend();

    auto onResized(const uint16_t width, const uint16_t height) -> void override;
    auto beginFrame(const float deltaTime) -> bool override;
    auto endFrame(const float deltaTime) -> bool override;
    auto drawFrame(const Packet& packet) -> bool override;

private:
    Context m_context;
};

} // namespace vulkan
} // namespace renderer
} // namespace beige
