#include "VulkanUtils.hpp"

namespace beige {
namespace renderer {
namespace vulkan {

auto Utils::resultToString(const VkResult& result, const bool getExtended)->std::string {
    // https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkResult.html
    switch (result) {
    default: [[fallthrough]];
    case VK_SUCCESS:
        return !getExtended ? "VK_SUCCESS" : "VK_SUCCESS (command successfully completed)";
    case VK_NOT_READY:
        return !getExtended ? "VK_NOT_READY" : "VK_NOT_READY (a fence or query has not yet completed)";
    case VK_TIMEOUT:
        return !getExtended ? "VK_TIMEOUT" : "VK_TIMEOUT (a wait operation has not completed in the specified time)";
    case VK_EVENT_SET:
        return !getExtended ? "VK_EVENT_SET" : "VK_EVENT_SET (an event is signaled)";
    }

    // TODO: Add all error codes
}

auto Utils::isResultSuccess(const VkResult& result) -> bool {
    switch (result) {
    case VK_SUCCESS: [[fallthrough]];
    case VK_NOT_READY: [[fallthrough]];
    case VK_TIMEOUT: [[fallthrough]];
    case VK_EVENT_SET: [[fallthrough]];
    case VK_EVENT_RESET: [[fallthrough]];
    case VK_INCOMPLETE: [[fallthrough]];
    case VK_SUBOPTIMAL_KHR: [[fallthrough]];
    case VK_THREAD_IDLE_KHR: [[fallthrough]];
    case VK_THREAD_DONE_KHR: [[fallthrough]];
    case VK_OPERATION_DEFERRED_KHR: [[fallthrough]];
    case VK_OPERATION_NOT_DEFERRED_KHR: [[fallthrough]];
    case VK_PIPELINE_COMPILE_REQUIRED_EXT:
        return true;
    default:
        return false;
    }

    // TODO: return true by default, add other error codes
}

} // namespace vulkan
} // namespace renderer
} // namespace beige
