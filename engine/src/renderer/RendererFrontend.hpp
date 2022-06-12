#pragma once

#include "RendererTypes.hpp"
#include "IRendererBackend.hpp"

#include <cstdint>
#include <memory>

namespace beige {
namespace renderer {

class RendererFrontend final {
public:
    RendererFrontend(const std::string& appName, platform::Platform& platform);
    ~RendererFrontend();

    auto onResized(const uint16_t width, const uint16_t height) -> void;
    auto beginFrame(const float deltaTime) -> bool;
    auto endFrame(const float deltaTime) -> bool;
    auto drawFrame(const Packet& packet) -> bool;

private:
    std::unique_ptr<IRendererBackend> m_rendererBackend;
};

} // namespace renderer
} // namespace beige
