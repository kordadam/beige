#pragma once

#include "../platform/Platform.hpp"

namespace beige {
namespace core {

class Clock final {
public:
    Clock(std::shared_ptr<platform::Platform> platform);
    ~Clock();

    auto getElapsedTime() -> double;
    auto start() -> void;
    auto stop() -> void;
    auto update() -> void;

private:
    std::shared_ptr<platform::Platform> m_platform;

    double m_startTime;
    double m_elapsedTime;
};

} // namespace core
} // namespace beige
