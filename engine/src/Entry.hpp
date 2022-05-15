#pragma once

#include "IGame.hpp"
#include "core/App.hpp"
#include "core/Logger.hpp"

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
        const std::string exceptionMessage { exception.what() };
        beige::core::Logger::fatal("Application failed to create: " + exceptionMessage);
        return 1;
    }

    if (!app->run()) {
        beige::core::Logger::info("Application did not shutdown gracefully!");
        return 2;
    }

    return 0;
}
