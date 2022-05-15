#include "Game.hpp"

#include <core/Logger.hpp>

#include <iostream>

Game::Game() :
IGame({ 100u, 100u, 1280u, 720u, "Beige Testbed" }) {
    beige::core::Logger::info("Game object has been created!");
}

Game::~Game() {
    beige::core::Logger::info("Game object has been destroyed!");
}

auto Game::update(const float deltaTime) -> bool {
    return true;
}

auto Game::render(const float deltaTime) -> bool {
    return true;
}

auto Game::onResize(const uint32_t width, const uint32_t height) -> void {

}
