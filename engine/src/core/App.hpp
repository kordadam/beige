#pragma once

#include "../Defines.hpp"
#include "Input.hpp"
#include "../platform/Platform.hpp"
#include "../renderer/RendererFrontend.hpp"
#include "../IGame.hpp"
#include "Clock.hpp"

#include <memory>

namespace beige {
namespace core {

class BEIGE_API App final {
public:
    App(std::unique_ptr<IGame> game);
    ~App();

    auto run() -> bool;

private:
    std::vector<Input::KeyEvent::Subscription> m_keyEventSubscriptions;
    std::vector<Input::MouseEvent::Subscription> m_mouseEventSubscriptions;
    bool m_isRunning;
    bool m_isSuspended;
    Clock m_clock;
    float m_lastTime;
    Input m_input;
    platform::Platform m_platform;
    renderer::RendererFrontend m_rendererFrontend;
    std::unique_ptr<IGame> m_game;
};

} // namespace core
} // namespace beige
