#include "App.hpp"

#include "Logger.hpp"

namespace beige {
namespace core {

App::App(std::unique_ptr<IGame> game) :
m_isRunning { true },
m_isSuspended { false },
m_previousTimeStamp { },
m_input { },
m_platform { game->getAppConfig(), m_input },
m_game { std::move(game) } {
    m_input.subscribe(
        [&](const KeyEventCode& keyEventCode, const Key& key) {
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
    m_previousTimeStamp = std::chrono::high_resolution_clock::now();

    while (m_isRunning) {
        if (!m_platform.pumpMessages()) {
            m_isRunning = false;
        }

        if (!m_isSuspended) {
            const auto currentTimeStamp { std::chrono::high_resolution_clock::now() };
            const float deltaTime {
                std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(
                    currentTimeStamp - m_previousTimeStamp
                ).count()
            };

            if (!m_game->update(deltaTime)) {
                Logger::fatal("Game update failed, shutting down!");
            }

            m_previousTimeStamp = currentTimeStamp;
        }
    }

    return true;
}

} // namespace core
} // namespace beige
