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
        const uint32_t width,
        const uint32_t height,
        std::shared_ptr<platform::Platform> platform
    ) :
    m_platform { platform },
    m_frameCount { 0u } {

    };
    virtual ~IRendererBackend() = default;

    auto getFrameCount() const -> uint64_t { return m_frameCount; }
    auto setFrameCount(const uint64_t frameCount) -> void { m_frameCount = frameCount; }

    virtual auto onResized(const uint16_t width, const uint16_t height) -> void = 0;
    virtual auto beginFrame(const float deltaTime) -> bool = 0;
    virtual auto endFrame(const float deltaTime) -> bool = 0;
    virtual auto drawFrame(const Packet& packet) -> bool = 0;

protected:
    std::shared_ptr<platform::Platform> m_platform;
    uint64_t m_frameCount;
};

} // namespace renderer
} // namespace beige
