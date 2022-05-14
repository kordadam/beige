#pragma once

#include <IGame.hpp>

class Game final : public beige::IGame {
public:
    Game();
    ~Game();

    auto update(const float deltaTime) -> bool override;
    auto render(const float deltaTime) -> bool override;
    auto onResize(const uint32_t width, const uint32_t height) -> void override;
};
