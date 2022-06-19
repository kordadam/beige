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

namespace beige {
namespace platform {

struct State {
#ifdef BEIGE_PLATFORM_WIN32
    HINSTANCE hInstance;
    HWND hnwd;
#endif // BEIGE_PLATFORM_WIN32

    VkSurfaceKHR surface;
};

enum class EventCode : uint32_t {
    WindowResized
};

class Platform final :
public core::Event<EventCode, uint32_t, uint32_t> {
public:
    using Event = core::Event<EventCode, uint32_t, uint32_t>;

    Platform(
        const core::AppConfig& appConfig,
        std::shared_ptr<core::Input> input
    );
    ~Platform();

    static auto consoleWrite(const std::string& message, const ConsoleColor consoleColor) -> void;

    auto pumpMessages() -> bool;
    auto Sleep(const uint64_t timeInMs) -> void;
    auto getVulkanRequiredExtensionNames() -> std::vector<const char*>;
    auto createVulkanSurface(
        const VkInstance& instance,
        const VkAllocationCallbacks* allocationCallbacks
    ) -> std::optional<VkSurfaceKHR>;

#ifdef BEIGE_PLATFORM_WIN32
    auto processMessage(HWND hwnd, uint32_t message, WPARAM wParam, LPARAM lParam) -> LRESULT;
#endif

private:
    State m_state;
    std::shared_ptr<core::Input> m_input;
};

} // namespace platform
} // namespace beige
