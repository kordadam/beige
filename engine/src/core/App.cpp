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
m_input { Input::getInstance() },
m_platform { std::make_shared<platform::Platform>(game->getAppConfig()) },
m_clock { std::make_unique<Clock>(m_platform) },
m_rendererFrontend {
    std::make_unique<renderer::Frontend>(
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
    m_isRunning = true;
    m_clock->start();
    m_clock->update();
    m_lastTime = m_clock->getElapsedTime();

    uint32_t frameCount { 0u };
    const double targetFrameInSeconds = 1.0 / 60.0;

    while (m_isRunning) {
        if (!m_platform->pumpMessages()) {
            m_isRunning = false;
        }

        if (!m_isSuspended) {
            // Update clock and get delta time.
            m_clock->update();
            const double currentTime { m_clock->getElapsedTime() };
            const double deltaTime { currentTime - m_lastTime };
            const double frameStartTime { m_platform->getAbsoluteTime() };

            if (!m_game->update(static_cast<float>(deltaTime))) {
                Logger::fatal("Game update failed, shutting down!");
                break;
            }

            // HACK
            m_rendererFrontend->setView(m_game->getState().view);

            static uint32_t currentTextureIndex = 0u;
            if (m_game->getState().textureIndex == 1u && currentTextureIndex != 1u) {
                currentTextureIndex = 1u;
                m_rendererFrontend->loadTexture("wall", m_rendererFrontend->m_testDiffuse);
            } else if (m_game->getState().textureIndex == 2u && currentTextureIndex != 2u) {
                currentTextureIndex = 2u;
                m_rendererFrontend->loadTexture("grass", m_rendererFrontend->m_testDiffuse);
            } else if (m_game->getState().textureIndex == 3u && currentTextureIndex != 3u) {
                currentTextureIndex = 3u;
                m_rendererFrontend->loadTexture("dirt", m_rendererFrontend->m_testDiffuse);
            }

            if (!m_game->render(static_cast<float>(deltaTime))) {
                Logger::fatal("Game render failed, shutting down!");
                break;
            }

            // TODO: Refactor packet creation
            const renderer::Packet packet {
                deltaTime
            };

            m_rendererFrontend->drawFrame(packet);

            // Figure out how long the frame took and, if below
            const double frameEndTime { m_platform->getAbsoluteTime() };
            const double frameElapsedTime { frameEndTime - frameStartTime };
            const double remainingInS { targetFrameInSeconds - frameElapsedTime};

            if (remainingInS > 0.0) {
                uint64_t remainingInMs { static_cast<uint64_t>(remainingInS) * 1000u };
                const float isFrameLimit { false };
                if (remainingInMs > 0u && isFrameLimit) {
                    m_platform->Sleep(remainingInMs - 1u);
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
