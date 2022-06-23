#pragma once

#include "../../platform/Platform.hpp"

#include <vulkan/vulkan.h>

#include <memory>

namespace beige {
namespace renderer {
namespace vulkan {

class Surface final {
public:
    Surface(
        VkAllocationCallbacks* allocationCallbacks,
        const VkInstance& instance,
        std::shared_ptr<platform::Platform> platform
    );

    ~Surface();

    auto getHandle() const -> const VkSurfaceKHR&;

private:
    VkSurfaceKHR m_handle;

    VkAllocationCallbacks* m_allocationCallbacks;
    VkInstance m_instance;
    std::shared_ptr<platform::Platform> m_platform;


};

} // namespace vulkan
} // namespace renderer
} // namespace beige
