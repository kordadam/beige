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
m_rendererBackend {
    std::make_unique<vulkan::VulkanBackend>(
        appName,
        width,
        height,
        platform
    )
},
m_frameCount { 0u } {

}

RendererFrontend::~RendererFrontend() {

}

auto RendererFrontend::onResized(const uint16_t width, const uint16_t height) -> void {
    m_rendererBackend->onResized(width, height);
}

auto RendererFrontend::drawFrame(const Packet& packet) -> bool {
    // If the begin frame returned successfully, mid-frame operations may continue.
    if (beginFrame(packet.deltaTime)) {

        static float z { -1.0f };
        z -= 0.01f;

        const math::Matrix4x4 projection {
            math::Matrix4x4::perspective(
                math::Quaternion::degToRad(45.0f),
                1280.0f / 720.0f,
                0.1f,
                1000.0f
            )
        };

        const math::Matrix4x4 view {
            math::Matrix4x4::translation(math::Vector3(0.0f, 0.0f, z))
        };

        m_rendererBackend->updateGlobalState(
            projection,
            view,
            math::Vector3::zero(),
            math::Vector4::one(),
            0
        );

        // End the frame. if this fails, it is likely unrecoverable.
        const bool result { endFrame(packet.deltaTime) };

        if (!result) {
            core::Logger::error("RendererFrontend::drawFrame - failed to ending frame, application shutting down...");
            return false;
        }
    }

    return m_rendererBackend->drawFrame(packet);
}

auto RendererFrontend::beginFrame(const float deltaTime) -> bool {
    return m_rendererBackend->beginFrame(deltaTime);
}

auto RendererFrontend::endFrame(const float deltaTime) -> bool {
    const bool result { m_rendererBackend->endFrame(deltaTime) };
    m_frameCount++;
    return result;
}

} // namespace renderer
} // namespace beige
