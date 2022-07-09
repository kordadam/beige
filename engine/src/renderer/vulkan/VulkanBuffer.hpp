#pragma once

#include "VulkanDevice.hpp"
#include "VulkanImage.hpp"

#include <vulkan/vulkan.h>

namespace beige {
namespace renderer {
namespace vulkan {

class Buffer final {
public:
    Buffer(
        VkAllocationCallbacks* allocationCallbacks,
        std::shared_ptr<Device> device,
        const uint64_t totalSize,
        const VkBufferUsageFlags bufferUsageFlags,
        const uint32_t memoryPropertyFlags,
        const bool bindOnCreate
    );

    ~Buffer();

    auto resize(
        const uint64_t newSize,
        const VkQueue& queue,
        const VkCommandPool& commandPool
    ) -> bool;

    auto bind(const uint64_t offset) -> void;

    auto lockMemory(
        const uint64_t offset,
        const uint64_t size,
        const uint32_t flags
    ) -> void*;

    auto unlockMemory() -> void;

    auto loadData(
        const uint64_t offset,
        const uint64_t size,
        const uint32_t flags,
        const void* data
    ) -> void;

    auto copyTo(
        const VkCommandPool& commandPool,
        const VkFence& fence,
        const VkQueue& queue,
        const VkBuffer& source,
        const uint64_t sourceOffset,
        const VkBuffer& dest,
        const uint64_t destOffset,
        const uint64_t size
    ) -> void;

private:
    VkAllocationCallbacks* m_allocationCallbacks;
    std::shared_ptr<Device> m_device;

    uint64_t m_totalSize;
    VkBuffer m_handle;
    VkBufferUsageFlags m_bufferUsageFlags;
    bool m_isLocked;
    VkDeviceMemory m_deviceMemory;
    uint32_t m_memoryIndex;
    uint32_t m_memoryPropertyFlags;
};

} // namespace vulkan
} // namespace renderer
} // namespace beige
