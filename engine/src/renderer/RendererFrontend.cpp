#include "RendererFrontend.hpp"

#include "vulkan/VulkanBackend.hpp"
#include "../core/Logger.hpp"

namespace beige {
namespace renderer {

Frontend::Frontend(
    const std::string& appName,
    const uint32_t width,
    const uint32_t height,
    std::shared_ptr<platform::Platform> platform
) :
m_backend {
    std::make_unique<vulkan::Backend>(
        appName,
        width,
        height,
        platform
    )
},
m_frameCount { 0u } {

}

Frontend::~Frontend() {

}

auto Frontend::onResized(const uint16_t width, const uint16_t height) -> void {
    m_backend->onResized(width, height);
}

auto Frontend::drawFrame(const Packet& packet) -> bool {
    // If the begin frame returned successfully, mid-frame operations may continue.
    if (beginFrame(packet.deltaTime)) {

        const math::Matrix4x4 projection {
            math::Matrix4x4::perspective(
                math::Quaternion::degToRad(45.0f),
                1280.0f / 720.0f,
                0.1f,
                1000.0f
            )
        };
        static float z { 0.0f };
        z += 0.005f;
        math::Matrix4x4 view {
            math::Matrix4x4::translation(math::Vector3(0.0f, 0.0f, z))
        };
        view.inverse();
        static float angle { 0.01f };
        angle += 0.001f;
        const math::Quaternion rotation {
            math::Vector3::forward(),
            angle,
            false
        };
        const math::Matrix4x4 model {
            rotation, math::Vector3::zero()
        };

        m_backend->updateGlobalState(
            projection,
            view,
            math::Vector3::zero(),
            math::Vector4::one(),
            0
        );
        m_backend->updateObject(model);

        // End the frame. if this fails, it is likely unrecoverable.
        const bool result { endFrame(packet.deltaTime) };

        if (!result) {
            core::Logger::error("RendererFrontend::drawFrame - failed to ending frame, application shutting down...");
            return false;
        }
    }

    return true;
}

auto Frontend::beginFrame(const float deltaTime) -> bool {
    return m_backend->beginFrame(deltaTime);
}

auto Frontend::endFrame(const float deltaTime) -> bool {
    const bool result { m_backend->endFrame(deltaTime) };
    m_frameCount++;
    return result;
}

} // namespace renderer
} // namespace beige
