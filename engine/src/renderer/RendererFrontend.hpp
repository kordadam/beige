#pragma once

#include "RendererTypes.hpp"
#include "IRendererBackend.hpp"
#include "../resources/ITexture.hpp"

#include <cstdint>
#include <memory>
#include <array>

namespace beige {
namespace renderer {

class Frontend final {
public:
    Frontend(
        const std::string& appName,
        const uint32_t width,
        const uint32_t height,
        std::shared_ptr<platform::Platform> platform
    );
    ~Frontend();

    auto onResized(const uint16_t width, const uint16_t height) -> void;
    auto drawFrame(const Packet& packet) -> bool;

    auto setView(const glm::mat4x4& view) -> void;

    // TODO: Temporary public.
    // TODO: Temporary.
    std::shared_ptr<resources::ITexture> m_testDiffuse;
    // TODO: End temporary.

    auto loadTexture(
        const std::string& textureName,
        std::shared_ptr<resources::ITexture>& texture
    ) -> bool;
    // TODO: End temporary public.

private:
    std::unique_ptr<IBackend> m_backend;
    uint64_t m_frameCount;

    float m_nearClip;
    float m_farClip;
    glm::mat4x4 m_projection;
    glm::mat4x4 m_view;

    std::shared_ptr<resources::ITexture> m_defaultTexture;

    auto beginFrame(const float deltaTime) -> bool;
    auto endFrame(const float deltaTime) -> bool;
};

} // namespace renderer
} // namespace beige
