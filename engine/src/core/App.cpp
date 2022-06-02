#include "App.hpp"

#include "Logger.hpp"

namespace beige {
namespace core {

App::App(std::unique_ptr<IGame> game) :
m_isRunning { true },
m_isSuspended { false },
m_clock { },
m_lastTime { 0.0f },
m_input { },
m_platform { game->getAppConfig(), m_input },
m_game { std::move(game) } {
    m_input.subscribe(
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
    );

    m_input.subscribe(
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
    );
}

App::~App() {

}

auto App::run() -> bool {
    m_clock.update();
    m_lastTime = m_clock.getElapsedTime();

    while (m_isRunning) {
        if (!m_platform.pumpMessages()) {
            m_isRunning = false;
        }

        if (!m_isSuspended) {
            m_clock.update();
            const float currentTime { m_clock.getElapsedTime() };
            const float deltaTime { currentTime - m_lastTime };

            if (!m_game->update(deltaTime)) {
                Logger::fatal("Game update failed, shutting down!");
            }

            m_lastTime = currentTime;
        }
    }

    return true;
}

} // namespace core
} // namespace beige
