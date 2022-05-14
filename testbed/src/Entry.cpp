#include <Entry.hpp>

#include "Game.hpp"

const std::unique_ptr<beige::IGame> beige::createGame() {
    return std::move(std::make_unique<Game>());
}
