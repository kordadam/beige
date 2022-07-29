#include "RendererFrontend.hpp"

#include "vulkan/VulkanBackend.hpp"
#include "../core/Logger.hpp"

#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include <iostream>

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
m_frameCount { 0u },
m_nearClip { 0.01f },
m_farClip { 1000.0f },
m_projection {
    glm::perspective(
        glm::radians(45.0f),
        static_cast<float>(width) / static_cast<float>(height),
        m_nearClip,
        m_farClip
    )
},
m_view { glm::mat4x4(1.0f) },
m_testDiffuse { } {

}

Frontend::~Frontend() {

}

auto Frontend::onResized(const uint16_t width, const uint16_t height) -> void {
    m_projection = glm::perspective(
        glm::radians(45.0f),
        static_cast<float>(width) / static_cast<float>(height),
        m_nearClip,
        m_farClip
    );

    m_backend->onResized(width, height);
}

auto Frontend::drawFrame(const Packet& packet) -> bool {
    // If the begin frame returned successfully, mid-frame operations may continue.
    if (beginFrame(packet.deltaTime)) {
        m_backend->updateGlobalState(
            m_projection,
            m_view,
            glm::vec3(0.0f),
            glm::vec4(1.0f),
            0
        );

        static float angle { 0.01f };
        angle += 0.001f;

        const GeometryRenderData geometryRenderData {
            0u, // TODO: Actual objectId;
            glm::rotate(glm::mat4x4(1.0f), angle, glm::vec3(0.0f, 0.0f, 1.0f)),
            { m_testDiffuse }
        };

        m_backend->updateObject(geometryRenderData);

        // End the frame. if this fails, it is likely unrecoverable.
        const bool result { endFrame(packet.deltaTime) };

        if (!result) {
            core::Logger::error("RendererFrontend::drawFrame - failed to ending frame, application shutting down...");
            return false;
        }
    }

    return true;
}

auto Frontend::setView(const glm::mat4x4& view) -> void {
    m_view = view;
}

auto Frontend::beginFrame(const float deltaTime) -> bool {
    return m_backend->beginFrame(deltaTime);
}

auto Frontend::endFrame(const float deltaTime) -> bool {
    const bool result { m_backend->endFrame(deltaTime) };
    m_frameCount++;
    return result;
}

auto Frontend::createTexture(
    const std::string& name,
    const int32_t width,
    const int32_t height,
    const int32_t channelCount,
    const void* pixels,
    const bool hasTransparency
) -> std::shared_ptr<resources::ITexture> {
    return m_backend->createTexture(
        name,
        width,
        height,
        channelCount,
        pixels,
        hasTransparency
    );
}

} // namespace renderer
} // namespace beige
