#include "RendererFrontend.hpp"
#include "vulkan/VulkanBackend.hpp"

namespace beige {
namespace renderer {

RendererFrontend::RendererFrontend(
    const std::string& appName,
    std::shared_ptr<platform::Platform> platform
) :
m_rendererBackend { std::make_unique<vulkan::VulkanBackend>(appName, platform) } {

}

RendererFrontend::~RendererFrontend() {

}

auto RendererFrontend::onResized(const uint16_t width, const uint16_t height) -> void {

}

auto RendererFrontend::beginFrame(const float deltaTime) -> bool {
    return true;
}

auto RendererFrontend::endFrame(const float deltaTime) -> bool {
    return true;
}

auto RendererFrontend::drawFrame(const Packet& packet) -> bool {
    return true;
}

} // namespace renderer
} // namespace beige
