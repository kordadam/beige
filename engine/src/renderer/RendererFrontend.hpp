#pragma once

#include "RendererTypes.hpp"
#include "IRendererBackend.hpp"

#include <cstdint>
#include <memory>

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

    auto setView(const math::Matrix4x4& view) -> void;

    auto createTexture(
        const std::string& name,
        const bool autoRelease,
        const int32_t width,
        const int32_t height,
        const int32_t channelCount,
        const std::vector<std::byte>& pixels,
        const bool hasTransparency
    ) -> std::shared_ptr<resources::ITexture>;
    auto destroyTexture(std::shared_ptr<resources::ITexture> texture) -> void;

private:
    std::unique_ptr<IBackend> m_backend;
    uint64_t m_frameCount;

    float m_nearClip;
    float m_farClip;
    math::Matrix4x4 m_projection;
    math::Matrix4x4 m_view;

    auto beginFrame(const float deltaTime) -> bool;
    auto endFrame(const float deltaTime) -> bool;
};

} // namespace renderer
} // namespace beige
