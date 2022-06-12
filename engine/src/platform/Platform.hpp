#pragma once

#include "../Defines.hpp"
#include "../core/AppTypes.hpp"
#include "../core/Input.hpp"
#include "PlatformTypes.hpp"

#include <string>
#include <optional>

#ifdef BEIGE_PLATFORM_WIN32
#include <windows.h>
#include <windowsx.h>
#include <stdlib.h>
#endif // BEIGE_PLATFORM_WIN32

// For surface creation
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>
#include "../renderer/vulkan/VulkanTypes.hpp"

namespace beige {
namespace platform {

struct State {
#ifdef BEIGE_PLATFORM_WIN32
    HINSTANCE hInstance;
    HWND hnwd;
#endif // BEIGE_PLATFORM_WIN32

    VkSurfaceKHR surface;
};

class Platform final {
public:
    Platform(
        const core::AppConfig& appConfig,
        core::Input& input
    );
    ~Platform();

    static auto consoleWrite(const std::string& message, const ConsoleColor consoleColor) -> void;

    auto pumpMessages() -> bool;
    auto getVulkanRequiredExtensionNames() -> std::vector<const char*>;
    auto createVulkanSurface(const renderer::vulkan::Context& vulkanContext) -> std::optional<VkSurfaceKHR>;

#ifdef BEIGE_PLATFORM_WIN32
    auto processMessage(HWND hwnd, uint32_t message, WPARAM wParam, LPARAM lParam) -> LRESULT;
#endif

private:
    State m_state;
    core::Input& m_input;
};

} // namespace platform
} // namespace beige
