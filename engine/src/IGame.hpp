#pragma once

#include "Defines.hpp"
#include "core/AppTypes.hpp"

#include <glm/glm.hpp>

namespace beige {

class BEIGE_API IGame {
public:
    struct State {
        glm::mat4x4 view;
        glm::vec3 cameraPosition;
        glm::vec3 cameraEuler;
        glm::vec3 cameraUp;
        glm::vec3 cameraRight;
        glm::vec3 cameraLook;
        bool cameraViewDirty;

        // TODO: Temporary.
        uint32_t textureIndex;
        // TODO: End temporary.
    };

    IGame(const core::AppConfig& appConfig) :
    m_appConfig { appConfig },
    m_state { } { }

    virtual ~IGame() = default;

    virtual auto update(const float deltaTime) -> bool = 0;
    virtual auto render(const float deltaTime) -> bool = 0;
    virtual auto onResize(const uint32_t width, const uint32_t height) -> void = 0;

    auto getAppConfig() const -> const core::AppConfig& { return m_appConfig; }
    auto getState() const -> const State& { return m_state; }

protected:
    core::AppConfig m_appConfig;
    State m_state;
};

} // namespace beige
