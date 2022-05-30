#pragma once

#include "../Defines.hpp"
#include "Input.hpp"
#include "../platform/Platform.hpp"
#include "../IGame.hpp"

#include <memory>
#include <chrono>

namespace beige {
namespace core {

class BEIGE_API App final {
public:
    App(std::unique_ptr<IGame> game);
    ~App();

    auto run() -> bool;

private:
    using TimeStamp = std::chrono::high_resolution_clock::time_point;

    bool m_isRunning;
    bool m_isSuspended;
    TimeStamp m_previousTimeStamp;
    Input m_input;
    platform::Platform m_platform;
    std::unique_ptr<IGame> m_game;
};

} // namespace core
} // namespace beige
