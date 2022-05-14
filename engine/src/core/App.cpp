#include "App.hpp"

namespace beige {
namespace core {

App::App(std::unique_ptr<IGame> game) :
m_isRunning { true },
m_isSuspended { false },
m_game { std::move(game) } {

}

App::~App() {

}

auto App::run() -> bool {
    return true;
}

} // namespace core
} // namespace beige
