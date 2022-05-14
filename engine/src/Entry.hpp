#pragma once

#include "IGame.hpp"
#include "core/App.hpp"

#include <memory>
#include <iostream>

namespace beige {

extern const std::unique_ptr<IGame> createGame();

} // namespace beige

int main() {
    std::unique_ptr<beige::core::App> app;

    try {
        std::unique_ptr<beige::IGame> game { beige::createGame() };
        app = std::make_unique<beige::core::App>(std::move(game));
    } catch (const std::exception& exception) {
        std::cout << "Application failed to create!\n";
        std::cout << exception.what() << "\n"; // TODO: replace cout by Logger::fatal
        return 1;
    }

    if (!app->run()) {
        std::cout << "Application did not shutdown gracefully!\n"; // TODO: replace cout by Logger::info
        return 2;
    }

    return 0;
}
