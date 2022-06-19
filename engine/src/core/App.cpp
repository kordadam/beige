#include "App.hpp"

#include "Logger.hpp"

#include <algorithm>

namespace beige {
namespace core {

App::App(std::unique_ptr<IGame> game) :
m_isRunning { true },
m_isSuspended { false },
m_lastTime { 0.0f },
m_windowWidth { game->getAppConfig().startWidth },
m_windowHeight { game->getAppConfig().startHeight },
m_input { std::make_shared<Input>() },
m_platform { std::make_shared<platform::Platform>(game->getAppConfig(), m_input) },
m_clock { std::make_unique<Clock>() },
m_rendererFrontend {
    std::make_unique<renderer::RendererFrontend>(
        game->getAppConfig().name,
        m_windowWidth,
        m_windowHeight,
        m_platform
    )
},
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

    m_platformSubscriptions.push_back(
        m_platform->subscribe(
            [&](
                const platform::EventCode& eventCode,
                const uint32_t& width,
                const uint32_t& height
            ) -> void {
                if (eventCode == platform::EventCode::WindowResized) {
                    if (m_windowWidth != width || m_windowHeight != height) {
                        m_windowWidth = width;
                        m_windowHeight = height;

                        core::Logger::info("Window resize to " + std::to_string(width) + " x " + std::to_string(height));

                        // Handle minimization
                        if (m_windowWidth == 0u || m_windowHeight == 0u) {
                            m_isSuspended = true;
                        } else {
                            if (m_isSuspended) {
                                core::Logger::info("Window restores, resuming application...");
                                m_isSuspended = false;
                            }

                            m_game->onResize(m_windowWidth, m_windowHeight);
                            m_rendererFrontend->onResized(m_windowWidth, m_windowHeight);
                        }
                    }
                }
            }
        )
    );
}

App::~App() {
    std::for_each(
        m_platformSubscriptions.begin(),
        m_platformSubscriptions.end(),
        [&](const platform::Platform::Event::Subscription& subscription) -> void {
            m_platform->unsubscribe(subscription);
        }
    );

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

    uint32_t frameCount { 0u };
    const float targetFrameInSeconds = 1.0f / 60.0f;

    while (m_isRunning) {
        if (!m_platform->pumpMessages()) {
            m_isRunning = false;
        }

        if (!m_isSuspended) {
            m_clock->update();
            const float currentTime { m_clock->getElapsedTime() };
            const float deltaTime { currentTime - m_lastTime };
            const float frameStartTime { m_clock->getAbsoluteTime() }; // Get absolute time

            if (!m_game->update(deltaTime)) {
                Logger::fatal("Game update failed, shutting down!");
                break;
            }

            if (!m_game->render(deltaTime)) {
                Logger::fatal("Game render failed, shutting down!");
                break;
            }

            // TODO: Refactor packet creation
            const renderer::Packet packet {
                deltaTime
            };

            m_rendererFrontend->drawFrame(packet);

            // Figure out how long the frame took and, if below
            const float frameEndTime { m_clock->getAbsoluteTime() }; // Get absolute time
            const float frameElapsedTime { frameEndTime - frameStartTime };
            const float remainingInSeconds { targetFrameInSeconds - frameElapsedTime };

            if (remainingInSeconds > 0.0f) {
                const uint64_t remainingInMs { static_cast<uint64_t>(remainingInSeconds * 1000.0f) };
                const float isFrameLimit { false };
                if (remainingInMs > 0u && isFrameLimit) {
                    m_platform->Sleep(remainingInMs);
                }
                frameCount++;
            }

            m_input->update(deltaTime);

            m_lastTime = currentTime;
        }
    }

    return true;
}

} // namespace core
} // namespace beige
