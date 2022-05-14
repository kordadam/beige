#pragma once

#include "Defines.hpp"
#include "core/AppTypes.hpp"

namespace beige {

class BEIGE_API IGame {
public:
    IGame(const core::AppConfig& appConfig) : m_appConfig { appConfig } { }
    virtual ~IGame() = default;

    virtual auto update(const float deltaTime) -> bool = 0;
    virtual auto render(const float deltaTime) -> bool = 0;
    virtual auto onResize(const uint32_t width, const uint32_t height) -> void = 0;

    auto getAppConfig() const -> const core::AppConfig& { return m_appConfig; }

protected:
    core::AppConfig m_appConfig;
};

} // namespace beige
