#include "Game.hpp"

#include <iostream>

Game::Game() :
IGame({ 100u, 100u, 1280u, 720u, "Beige Testbed" }) {
    std::cout << "Game object has been created!\n";
}

Game::~Game() {
    std::cout << "Game object has been destroyed!\n";
}

auto Game::update(const float deltaTime) -> bool {
    return true;
}

auto Game::render(const float deltaTime) -> bool {
    return true;
}

auto Game::onResize(const uint32_t width, const uint32_t height) -> void {

}
