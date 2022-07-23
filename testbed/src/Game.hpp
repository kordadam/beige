#pragma once

#include <IGame.hpp>
#include <core/Input.hpp>

namespace bc = beige::core;

class Game final : public beige::IGame {
public:
    Game();
    ~Game();

    auto update(const float deltaTime) -> bool override;
    auto render(const float deltaTime) -> bool override;
    auto onResize(const uint32_t width, const uint32_t height) -> void override;

private:
    std::shared_ptr<bc::Input> m_input;

    auto recalculateView() -> void;
    auto cameraYaw(const float amount) -> void;
    auto cameraPitch(const float amount) -> void;
    auto cameraRoll(const float amount) -> void;
};
