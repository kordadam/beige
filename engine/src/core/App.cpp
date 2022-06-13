#include "App.hpp"

#include "Logger.hpp"

#include <algorithm>

namespace beige {
namespace core {

App::App(std::unique_ptr<IGame> game) :
m_isRunning { true },
m_isSuspended { false },
m_lastTime { 0.0f },
m_input { std::make_shared<Input>() },
m_platform { std::make_shared<platform::Platform>(game->getAppConfig(), m_input) },
m_clock { std::make_unique<Clock>() },
m_rendererFrontend { std::make_unique<renderer::RendererFrontend>(game->getAppConfig().name, m_platform) },
m_game { std::move(game) } {
    m_keyEventSubscriptions.push_back(
        m_input->KeyEvent::subscribe(
            [&](const KeyEventCode& keyEventCode, const Key& key) -> void {
                switch (keyEventCode) {
                case KeyEventCode::Pressed: {
                    if (key == Key::Escape) {
                        m_isRunning = false;
                    }
                    else {
                        Logger::debug("Key pressed!");
                    }
                    break;
                }
                case KeyEventCode::Released: {
                    Logger::debug("Key released!");
                    break;
                }
                }
            }
        )
    );

    m_mouseEventSubscriptions.push_back(
        m_input->MouseEvent::subscribe(
            [](const MouseEventCode& mouseEventCode, const MouseState& mouseState) -> void {
                switch (mouseEventCode) {
                case MouseEventCode::Pressed: {
                    Logger::debug("Mouse button pressed!");
                    break;
                }
                case MouseEventCode::Released: {
                    Logger::debug("Mouse button released!");
                    break;
                }
                case MouseEventCode::Moved: {
                    break;
                }
                case MouseEventCode::Wheel: {
                    break;
                }
                }
            }
        )
    );
}

App::~App() {
    std::for_each(
        m_keyEventSubscriptions.begin(),
        m_keyEventSubscriptions.end(),
        [&](const Input::KeyEvent::Subscription& subscription) -> void {
            m_input->KeyEvent::unsubscribe(subscription);
        }
    );

    std::for_each(
        m_mouseEventSubscriptions.begin(),
        m_mouseEventSubscriptions.end(),
        [&](const Input::MouseEvent::Subscription& subscription) -> void {
            m_input->MouseEvent::unsubscribe(subscription);
        }
    );
}

auto App::run() -> bool {
    m_clock->update();
    m_lastTime = m_clock->getElapsedTime();

    while (m_isRunning) {
        if (!m_platform->pumpMessages()) {
            m_isRunning = false;
        }

        if (!m_isSuspended) {
            m_clock->update();
            const float currentTime { m_clock->getElapsedTime() };
            const float deltaTime { currentTime - m_lastTime };

            if (!m_game->update(deltaTime)) {
                Logger::fatal("Game update failed, shutting down!");
                break;
            }

            if (!m_game->render(deltaTime)) {
                Logger::fatal("Game render failed, shutting down!");
                break;
            }

            m_input->update(deltaTime);

            m_lastTime = currentTime;
        }
    }

    return true;
}

} // namespace core
} // namespace beige
