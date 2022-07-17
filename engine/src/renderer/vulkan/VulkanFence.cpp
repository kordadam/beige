#include "VulkanFence.hpp"

#include "VulkanDefines.hpp"

namespace beige {
namespace renderer {
namespace vulkan {

Fence::Fence(
    VkAllocationCallbacks* allocationCallbacks,
    std::shared_ptr<Device> device,
    const bool createSignaled
) :
m_allocationCallbacks{ allocationCallbacks },
m_device { device },
m_fence { VK_NULL_HANDLE },
m_isSignaled { createSignaled } {
    const VkDevice logicalDevice { m_device->getLogicalDevice() };

    const VkFenceCreateInfo fenceCreateInfo {
        VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,               // sType
        nullptr,                                           // pNext
        createSignaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0u // flags
    };

    VULKAN_CHECK(
        vkCreateFence(
            logicalDevice,
            &fenceCreateInfo,
            m_allocationCallbacks,
            &m_fence
        )
    );
}

Fence::~Fence() {
    const VkDevice logicalDevice { m_device->getLogicalDevice() };

    vkDestroyFence(
        logicalDevice,
        m_fence,
        m_allocationCallbacks
    );
}

auto Fence::getFence() const -> const VkFence& {
    return m_fence;
}

auto Fence::wait(const uint64_t timeoutInNs) -> bool {
    if (!m_isSignaled) {
        const VkDevice logicalDevice { m_device->getLogicalDevice() };
        const VkResult result {
            vkWaitForFences(
                logicalDevice,
                1u,
                &m_fence,
                VK_TRUE,
                timeoutInNs
            )
        };

        switch (result) {
        case VK_SUCCESS:
            m_isSignaled = true;
            return true;
        case VK_TIMEOUT:
            core::Logger::warn("Fence::wait - timeout");
            break;
        case VK_ERROR_DEVICE_LOST:
            core::Logger::error("Fence::wait - device lost");
            break;
        case VK_ERROR_OUT_OF_HOST_MEMORY:
            core::Logger::error("Fence::wait - out of host memory");
            break;
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            core::Logger::error("Fence::wait - out of device memory");
            break;
        default:
            core::Logger::error("Fence::wait - unknown error has occured");
            break;
        }
    } else {
        // If already signaled, do not wait
        return true;
    }

    return false;
}

auto Fence::reset() -> void {
    if (m_isSignaled) {
        const VkDevice logicalDevice { m_device->getLogicalDevice() };
        VULKAN_CHECK(
            vkResetFences(
                logicalDevice,
                1u,
                &m_fence
            )
        );

        m_isSignaled = false;
    }
}

} // namespace vulkan
} // namespace renderer
} // namespace beige
