#pragma once

#include "../Defines.hpp"
#include "../platform/Platform.hpp"
#include "RendererTypes.hpp"

namespace beige {
namespace renderer {

class IRendererBackend {
public:
    IRendererBackend(
        const std::string& appName,
        std::shared_ptr<platform::Platform> platform
    ) :
    m_platform { platform } {

    };
    virtual ~IRendererBackend() = default;

    virtual auto onResized(const uint16_t width, const uint16_t height) -> void = 0;
    virtual auto beginFrame(const float deltaTime) -> bool = 0;
    virtual auto endFrame(const float deltaTime) -> bool = 0;
    virtual auto drawFrame(const Packet& packet) -> bool = 0;

protected:
    std::shared_ptr<platform::Platform> m_platform;
};

} // namespace renderer
} // namespace beige
