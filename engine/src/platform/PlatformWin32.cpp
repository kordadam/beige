#include "Platform.hpp"

#ifdef BEIGE_PLATFORM_WIN32

#include <map>
#include <string>
#include <iostream>

namespace beige {
namespace platform {

Platform::Platform(const core::AppConfig& appConfig) {

}

Platform::~Platform() {

}

auto Platform::consoleWrite(const std::string& message, const ConsoleColor consoleColor) -> void {
    const HANDLE consoleHandle { GetStdHandle(STD_OUTPUT_HANDLE) };
    SetConsoleTextAttribute(consoleHandle, static_cast<WORD>(consoleColor));
    std::cout << message << "\n";
    SetConsoleTextAttribute(consoleHandle, static_cast<WORD>(ConsoleColor::White));
}

auto Platform::platformPumpMessages() -> bool {
    return true;
}

auto Platform::shutdown() -> void {

}

} // namespace platform
} // namespace beige

#endif
