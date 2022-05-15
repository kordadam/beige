#pragma once

#include "../Defines.hpp"
#include "../core/AppTypes.hpp"
#include "PlatformTypes.hpp"

#include <string>

#ifdef BEIGE_PLATFORM_WIN32
#include <windows.h>
#include <windowsx.h>
#include <stdlib.h>
#endif // BEIGE_PLATFORM_WIN32

namespace beige {
namespace platform {

struct State {
#ifdef BEIGE_PLATFORM_WIN32
    HINSTANCE hInstance;
    HWND hnwd;
#endif // BEIGE_PLATFORM_WIN32
};

class Platform final {
public:
    Platform(const core::AppConfig& appConfig);
    ~Platform();

    static auto consoleWrite(const std::string& message, const ConsoleColor consoleColor) -> void;

    auto platformPumpMessages() -> bool;
    auto shutdown() -> void;

private:
    State m_state;
};

} // namespace platform
} // namespace beige
