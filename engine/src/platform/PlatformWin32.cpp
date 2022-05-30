#include "Platform.hpp"

#ifdef BEIGE_PLATFORM_WIN32

#include <map>
#include <string>
#include <iostream>

namespace beige {
namespace platform {

static Platform* platform { nullptr };
LRESULT CALLBACK processMessageCallback(HWND hwnd, uint32_t message, WPARAM wParam, LPARAM lParam);

Platform::Platform(
    const core::AppConfig& appConfig,
    core::Input& input
) :
m_state { },
m_input { input } {
    m_state.hInstance = GetModuleHandleA(0);

    const HICON icon{ LoadIcon(m_state.hInstance, IDI_APPLICATION) };
    WNDCLASSA windowClass{ 0 };
    windowClass.style = CS_DBLCLKS;
    windowClass.lpfnWndProc = processMessageCallback;
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = 0;
    windowClass.hInstance = m_state.hInstance;
    windowClass.hIcon = icon;
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowClass.hbrBackground = NULL; // Transparent
    windowClass.lpszClassName = "NotidokWindowClass";

    if (!RegisterClassA(&windowClass)) {
        const std::string message{ "Window registration failed!" };
        MessageBoxA(0, message.c_str(), "Error", MB_ICONEXCLAMATION | MB_OK);
        throw std::exception(message.c_str());
    }

    const uint32_t windowStyle{ WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_THICKFRAME };
    const uint32_t windowExStyle{ WS_EX_APPWINDOW };
    RECT borderRect{ 0, 0, 0, 0 };
    AdjustWindowRectEx(&borderRect, windowStyle, 0, windowExStyle);

    const uint32_t xWindow { appConfig.xStartPos + borderRect.left };
    const uint32_t yWindow { appConfig.yStartPos + borderRect.top };
    const uint32_t windowWidth { appConfig.startWidth + (borderRect.right - borderRect.left) };
    const uint32_t windowHeight { appConfig.startHeight + (borderRect.bottom - borderRect.top) };

    const HWND hwnd {
        CreateWindowExA(
            windowExStyle,
            "NotidokWindowClass",
            appConfig.name.c_str(),
            windowStyle,
            xWindow,
            yWindow,
            windowWidth,
            windowHeight,
            0,
            0,
            m_state.hInstance,
            0
        )
    };

    if (hwnd == 0) {
        const std::string message{ "Window creation failed!" };
        MessageBoxA(0, message.c_str(), "Error", MB_ICONEXCLAMATION | MB_OK);
        throw std::exception(message.c_str());
    }
    else {
        m_state.hnwd = hwnd;
    }

    const bool shouldActivate { true };
    const int32_t showWindowCommandFlags { shouldActivate ? SW_SHOW : SW_SHOWNOACTIVATE };
    ShowWindow(m_state.hnwd, showWindowCommandFlags);

    platform = this;
}

Platform::~Platform() {
    if (m_state.hnwd != 0) {
        DestroyWindow(m_state.hnwd);
        m_state.hnwd = 0;
    }
}

auto Platform::consoleWrite(const std::string& message, const ConsoleColor consoleColor) -> void {
    const HANDLE consoleHandle { GetStdHandle(STD_OUTPUT_HANDLE) };
    SetConsoleTextAttribute(consoleHandle, static_cast<WORD>(consoleColor));
    std::cout << message << "\n";
    SetConsoleTextAttribute(consoleHandle, static_cast<WORD>(ConsoleColor::White));
}

auto Platform::pumpMessages() -> bool {
    MSG message;
    while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&message);
        DispatchMessageA(&message);
    }

    return true;
}

auto Platform::shutdown() -> void {

}

auto Platform::processMessage(HWND hwnd, uint32_t message, WPARAM wParam, LPARAM lParam) -> LRESULT {
    switch (message) {
    case WM_ERASEBKGND: {
        // Notify the OS that erasing will be handled by the application to prevent flicker
        return 1;
    }
    case WM_CLOSE: {
        // TODO: Fire an event for the application to quit
        return 0;
    }
    case WM_DESTROY: {
        PostQuitMessage(0);
        return 0;
    }
    case WM_SIZE: {
        // RECT rect;
        // GetClientRect(hwnd, &rect);
        // u32 width = rect.right - rect.left;
        // u32 height = rect.bottom - rect.top;
        // TODO: Fire and event for window resize
        break;
    }
    case WM_KEYDOWN: [[fallthrough]];
    case WM_SYSKEYDOWN: [[fallthrough]];
    case WM_KEYUP: [[fallthrough]];
    case WM_SYSKEYUP: {
        const core::Key key{ static_cast<core::Key>(wParam) };
        const bool isPressed{ message == WM_KEYDOWN || message == WM_SYSKEYDOWN };
        m_input.processKey(key, isPressed);
        break;
    }
    case WM_MOUSEMOVE: {
        const int32_t xPosition{ static_cast<int32_t>(GET_X_LPARAM(lParam)) };
        const int32_t yPosition{ static_cast<int32_t>(GET_Y_LPARAM(lParam)) };
        m_input.processMouseMove(xPosition, yPosition);
        break;
    }
    case WM_MOUSEWHEEL: {
        int32_t zDelta{ static_cast<int32_t>(GET_WHEEL_DELTA_WPARAM(wParam)) };
        if (zDelta != 0) {
            zDelta = (zDelta < 0) ? -1 : 1;
        }
        m_input.processMouseWheel(zDelta);
        break;
    }
    case WM_LBUTTONDOWN: [[fallthrough]];
    case WM_MBUTTONDOWN: [[fallthrough]];
    case WM_RBUTTONDOWN: [[fallthrough]];
    case WM_LBUTTONUP: [[fallthrough]];
    case WM_MBUTTONUP: [[fallthrough]];
    case WM_RBUTTONUP: {
        const bool isPressed { message == WM_LBUTTONDOWN || message == WM_MBUTTONDOWN || message == WM_RBUTTONDOWN };
        core::Button button { core::Button::Invalid };

        switch (message) {
        case WM_LBUTTONDOWN: [[fallthrough]];
        case WM_LBUTTONUP: {
            button = core::Button::Left;
            break;
        }
        case WM_MBUTTONDOWN: [[fallthrough]];
        case WM_MBUTTONUP: {
            button = core::Button::Middle;
            break;
        }
        case WM_RBUTTONDOWN: [[fallthrough]];
        case WM_RBUTTONUP: {
            button = core::Button::Right;
            break;
        }
        }

        if (button != core::Button::Invalid) {
            m_input.processButton(button, isPressed);
        }
        break;
    }
    }

    return DefWindowProcA(hwnd, message, wParam, lParam);
}

LRESULT CALLBACK processMessageCallback(HWND hwnd, uint32_t message, WPARAM wParam, LPARAM lParam) {
    if (platform != nullptr) {
        return platform->processMessage(hwnd, message, wParam, lParam);
    }

    return DefWindowProcA(hwnd, message, wParam, lParam);
}

} // namespace platform
} // namespace beige

#endif