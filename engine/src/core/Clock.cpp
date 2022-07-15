#include "Clock.hpp"

namespace beige {
namespace core {

Clock::Clock(std::shared_ptr<platform::Platform> platform) :
m_platform { platform },
m_startTime { m_platform->getAbsoluteTime() },
m_elapsedTime { 0.0 } {

}

Clock::~Clock() {

}

auto Clock::getElapsedTime() -> double {
    return m_elapsedTime;
}

auto Clock::start() -> void {
    m_startTime = m_platform->getAbsoluteTime();
    m_elapsedTime = 0.0;
}

auto Clock::stop() -> void {
    m_startTime = 0.0;
}

auto Clock::update() -> void {
    if (m_startTime != 0.0) {
        m_elapsedTime = m_platform->getAbsoluteTime() - m_startTime;
    }
}


} // namespace core
} // namespace beige
