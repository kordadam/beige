#include "RendererFrontend.hpp"

#include "vulkan/VulkanBackend.hpp"
#include "../core/Logger.hpp"

namespace beige {
namespace renderer {

RendererFrontend::RendererFrontend(
    const std::string& appName,
    const uint32_t width,
    const uint32_t height,
    std::shared_ptr<platform::Platform> platform
) :
m_rendererBackend { std::make_unique<vulkan::VulkanBackend>(appName, width, height, platform) } {

}

RendererFrontend::~RendererFrontend() {

}

auto RendererFrontend::onResized(const uint16_t width, const uint16_t height) -> void {
    m_rendererBackend->onResized(width, height);
}

auto RendererFrontend::beginFrame(const float deltaTime) -> bool {
    return m_rendererBackend->beginFrame(deltaTime);
}

auto RendererFrontend::endFrame(const float deltaTime) -> bool {
    const bool result { m_rendererBackend->endFrame(deltaTime) };
    m_rendererBackend->setFrameCount(m_rendererBackend->getFrameCount() + 1u);
    return result;
}

auto RendererFrontend::drawFrame(const Packet& packet) -> bool {
    // If the begin frame returned successfully, mid-frame operations may continue
    if (beginFrame(packet.deltaTime)) {

        const bool result { endFrame(packet.deltaTime) };

        if (!result) {
            core::Logger::error("RendererFrontend::drawFrame - failed to ending frame, application shutting down...");
            return false;
        }
    }

    return m_rendererBackend->drawFrame(packet);
}

} // namespace renderer
} // namespace beige
