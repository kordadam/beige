#pragma once

#include "../Defines.hpp"
#include "../IGame.hpp"

#include <memory>

namespace beige {
namespace core {

class BEIGE_API App final {
public:
    App(std::unique_ptr<IGame> game);
    ~App();

    auto run() -> bool;

private:
    bool m_isRunning;
    bool m_isSuspended;
    std::unique_ptr<IGame> m_game;
};

} // namespace core
} // namespace beige
