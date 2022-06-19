#pragma once

#include <chrono>

namespace beige {
namespace core {

class Clock final {
public:
    Clock();
    ~Clock();

    auto getElapsedTime() -> float;
    auto getAbsoluteTime() -> float;
    auto update() -> void;

private:
    std::chrono::high_resolution_clock::time_point m_startTime;
    float m_elapsedTime;
};

} // namespace core
} // namespace beige
