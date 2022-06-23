#include "VulkanSurface.hpp"

#include "../../core/Logger.hpp"

namespace beige {
namespace renderer {
namespace vulkan {

Surface::Surface(
    VkAllocationCallbacks* allocationCallbacks,
    const VkInstance& instance,
    std::shared_ptr<platform::Platform> platform
) :
m_handle { 0 },
m_allocationCallbacks { allocationCallbacks },
m_instance { instance },
m_platform { platform } {
    core::Logger::info("Creating Vulkan surface...");

    std::optional<VkSurfaceKHR> surface {
        m_platform->createVulkanSurface(m_instance, m_allocationCallbacks)
    };

    if (surface.has_value()) {
        m_handle = surface.value();
    } else {
        throw std::exception("Failed to create platform surface!");
    }

    core::Logger::info("Vulkan surface created!");
}

Surface::~Surface() {
    vkDestroySurfaceKHR(
        m_instance,
        m_handle,
        m_allocationCallbacks
    );
}

auto Surface::getHandle() const -> const VkSurfaceKHR& {
    return m_handle;
}

} // namespace vulkan
} // namespace renderer
} // namespace beige
