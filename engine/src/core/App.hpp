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
    std::vector<platform::Platform::Event::Subscription> m_platformSubscriptions;

    bool m_isRunning;
    bool m_isSuspended;
    float m_lastTime;

    uint32_t m_windowWidth;
    uint32_t m_windowHeight;

    std::shared_ptr<Input> m_input;
    std::shared_ptr<platform::Platform> m_platform;
    std::unique_ptr<Clock> m_clock;
    std::unique_ptr<renderer::RendererFrontend> m_rendererFrontend;
    std::unique_ptr<IGame> m_game;
};

} // namespace core
} // namespace beige
