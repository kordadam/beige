#pragma once

#include <vulkan/vulkan.h>

namespace beige {
namespace renderer {
namespace vulkan {

struct Context {
    VkAllocationCallbacks* allocationCallbacks;
    VkInstance instance;
    VkSurfaceKHR surface;

#if defined(BEIGE_DEBUG)
    VkDebugUtilsMessengerEXT debugUtilsMessenger;
#endif
};

} // namespace vulkan
} // namespace renderer
} // namespace beige
