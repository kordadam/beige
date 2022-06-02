#include "Clock.hpp"

namespace beige {
namespace core {

Clock::Clock() :
m_startTime { std::chrono::high_resolution_clock::now() },
m_elapsedTime { 0.0f } {

}

Clock::~Clock() {

}

auto Clock::getElapsedTime() -> float {
    return m_elapsedTime;
}

auto Clock::update() -> void {
    m_elapsedTime = std::chrono::duration_cast<std::chrono::duration<float>>(
        std::chrono::high_resolution_clock::now() - m_startTime
    ).count();
}


} // namespace core
} // namespace beige
