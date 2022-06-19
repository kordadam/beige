#pragma once

#include "VulkanDevice.hpp"

#include <vulkan/vulkan.h>

#include <memory>

namespace beige {
namespace renderer {
namespace vulkan {

class Fence final {
public:
    Fence(
        VkAllocationCallbacks* allocationCallbacks,
        std::shared_ptr<VulkanDevice> device,
        const bool createSignaled
    );

    ~Fence();

    auto getFence() const -> const VkFence&;

    auto wait(const uint64_t timeoutInNs) -> bool;
    auto reset() -> void;

private:
    VkAllocationCallbacks* m_allocationCallbacks;
    std::shared_ptr<VulkanDevice> m_device;

    VkFence m_fence;
    bool m_isSignaled;
};

} // namespace vulkan
} // namespace renderer
} // namespace beige
