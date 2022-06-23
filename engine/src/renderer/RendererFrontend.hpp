#pragma once

#include "RendererTypes.hpp"
#include "IRendererBackend.hpp"

#include <cstdint>
#include <memory>

namespace beige {
namespace renderer {

class RendererFrontend final {
public:
    RendererFrontend(
        const std::string& appName,
        const uint32_t width,
        const uint32_t height,
        std::shared_ptr<platform::Platform> platform
    );
    ~RendererFrontend();

    auto onResized(const uint16_t width, const uint16_t height) -> void;
    auto drawFrame(const Packet& packet) -> bool;

private:
    std::unique_ptr<IRendererBackend> m_rendererBackend;
    uint64_t m_frameCount;

    auto beginFrame(const float deltaTime) -> bool;
    auto endFrame(const float deltaTime) -> bool;
};

} // namespace renderer
} // namespace beige
