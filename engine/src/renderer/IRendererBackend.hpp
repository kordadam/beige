#pragma once

#include "../Defines.hpp"
#include "../platform/Platform.hpp"
#include "RendererTypes.hpp"

namespace beige {
namespace renderer {

class IRendererBackend {
public:
    IRendererBackend() = default;
    virtual ~IRendererBackend() = default;

    virtual auto onResized(const uint16_t width, const uint16_t height) -> void = 0;
    virtual auto beginFrame(const float deltaTime) -> bool = 0;
    virtual auto updateGlobalState(
        const math::Matrix4x4& projection,
        const math::Matrix4x4& view,
        const math::Vector3& viewPosition,
        const math::Vector4& ambientColor,
        const int32_t mode
    ) -> void = 0;
    virtual auto endFrame(const float deltaTime) -> bool = 0;
    virtual auto drawFrame(const Packet& packet) -> bool = 0;
};

} // namespace renderer
} // namespace beige
