#include "VulkanBuffer.hpp"

#include "VulkanDefines.hpp"
#include "VulkanUtils.hpp"
#include "VulkanCommandBuffer.hpp"

namespace beige {
namespace renderer {
namespace vulkan {

Buffer::Buffer(
    VkAllocationCallbacks* allocationCallbacks,
    std::shared_ptr<Device> device,
    const uint64_t totalSize,
    const VkBufferUsageFlags bufferUsageFlags,
    const uint32_t memoryPropertyFlags,
    const bool bindOnCreate
) :
m_allocationCallbacks { allocationCallbacks },
m_device { device },
m_totalSize { totalSize },
m_handle { VK_NULL_HANDLE },
m_bufferUsageFlags { bufferUsageFlags },
m_isLocked { false },
m_deviceMemory { VK_NULL_HANDLE },
m_memoryIndex { 0u },
m_memoryPropertyFlags { memoryPropertyFlags } {
    const VkBufferCreateInfo bufferCreateInfo {
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, // sType
        nullptr,                              // pNext
        0u,                                   // flags
        m_totalSize,                          // size
        m_bufferUsageFlags,                   // usage
        VK_SHARING_MODE_EXCLUSIVE,            // sharingMode
        0u,                                   // queueFamilyIndexCount
        nullptr                               // pQueueFamilyIndices
    };

    const VkDevice logicalDevice { m_device->getLogicalDevice() };

    VULKAN_CHECK(
        vkCreateBuffer(
            logicalDevice,
            &bufferCreateInfo,
            m_allocationCallbacks,
            &m_handle
        )
    );

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(logicalDevice, m_handle, &memoryRequirements);
    const std::optional<uint32_t> memoryIndex {
        m_device->findMemoryIndex(memoryRequirements.memoryTypeBits, m_memoryPropertyFlags)
    };

    if (memoryIndex.has_value()) {
        m_memoryIndex = memoryIndex.value();
    } else {
        throw std::exception("Unable to create Vulkan buffer because the required memory type index was not found!");
    }

    const VkMemoryAllocateInfo memoryAllocateInfo {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, // sType
        nullptr,                                // pNext
        memoryRequirements.size,                // allocationSize
        m_memoryIndex                           // memoryTypeIndex
    };

    const VkResult result {
        vkAllocateMemory(
            logicalDevice,
            &memoryAllocateInfo,
            m_allocationCallbacks,
            &m_deviceMemory
        )
    };

    if (result != VK_SUCCESS) {
        const std::string message {
            "Unable to create Vulkan buffer because the required memory allocation failed: " + Utils::resultToString(result, true) + "!"
        };
        throw std::exception(message.c_str());
    }

    if (bindOnCreate) {
        bind(0u);
    }
}

Buffer::~Buffer() {
    const VkDevice logicalDevice { m_device->getLogicalDevice() };

    if (m_deviceMemory != VK_NULL_HANDLE) {
        vkFreeMemory(logicalDevice, m_deviceMemory, m_allocationCallbacks);
    }

    if (m_handle != VK_NULL_HANDLE) {
        vkDestroyBuffer(logicalDevice, m_handle, m_allocationCallbacks);
    }
}

auto Buffer::resize(
    const uint64_t newSize,
    const VkQueue& queue,
    const VkCommandPool& commandPool
) -> bool {
    const VkBufferCreateInfo bufferCreateInfo {
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, // sType
        nullptr,                              // pNext
        0u,                                   // flags
        newSize,                              // size
        m_bufferUsageFlags,                   // usage
        VK_SHARING_MODE_EXCLUSIVE,            // sharingMode
        0u,                                   // queueFamilyIndexCount
        nullptr                               // pQueueFamilyIndices
    };

    const VkDevice logicalDevice { m_device->getLogicalDevice() };
    VkBuffer newBuffer;

    VULKAN_CHECK(
        vkCreateBuffer(
            logicalDevice,
            &bufferCreateInfo,
            m_allocationCallbacks,
            &newBuffer
        )
    );

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(logicalDevice, newBuffer, &memoryRequirements);

    const VkMemoryAllocateInfo memoryAllocateInfo {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, // sType
        nullptr,                                // pNext
        memoryRequirements.size,                // allocationSize
        m_memoryIndex                           // memoryTypeIndex
    };

    VkDeviceMemory newDeviceMemory;

    const VkResult result {
        vkAllocateMemory(
            logicalDevice,
            &memoryAllocateInfo,
            m_allocationCallbacks,
            &newDeviceMemory
        )
    };

    if (result != VK_SUCCESS) {
        core::Logger::error(
            "Unable to resize Vulkan buffer because the required memory allocation failed: " + Utils::resultToString(result, true) + "!"
        );
        return false;
    }

    VULKAN_CHECK(vkBindBufferMemory(logicalDevice, newBuffer, newDeviceMemory, 0u));

    copyTo(
        commandPool,
        VK_NULL_HANDLE,
        queue,
        m_handle,
        0u,
        newBuffer,
        0u,
        m_totalSize
    );

    vkDeviceWaitIdle(logicalDevice);

    if (m_deviceMemory != VK_NULL_HANDLE) {
        vkFreeMemory(logicalDevice, m_deviceMemory, m_allocationCallbacks);
        m_deviceMemory = VK_NULL_HANDLE;
    }

    if (m_handle != VK_NULL_HANDLE) {
        vkDestroyBuffer(logicalDevice, m_handle, m_allocationCallbacks);
        m_handle = VK_NULL_HANDLE;
    }

    m_totalSize = newSize;
    m_deviceMemory = newDeviceMemory;
    m_handle = newBuffer;

    return true;
}

auto Buffer::bind(const uint64_t offset) -> void {
    const VkDevice logicalDevice { m_device->getLogicalDevice() };

    VULKAN_CHECK(
        vkBindBufferMemory(
            logicalDevice,
            m_handle,
            m_deviceMemory,
            offset
        )
    );
}

auto Buffer::lockMemory(
    const uint64_t offset,
    const uint64_t size,
    const uint32_t flags
) -> void* {
    const VkDevice logicalDevice { m_device->getLogicalDevice() };
    void* data;
    VULKAN_CHECK(
        vkMapMemory(logicalDevice, m_deviceMemory, offset, size, flags, &data)
    );
    return data;
}

auto Buffer::unlockMemory() -> void {
    const VkDevice logicalDevice { m_device->getLogicalDevice() };
    vkUnmapMemory(logicalDevice, m_deviceMemory);
}

auto Buffer::loadData(
    const uint64_t offset,
    const uint64_t size,
    const uint32_t flags,
    const void* data
) -> void {
    const VkDevice logicalDevice { m_device->getLogicalDevice() };
    void* dest;
    VULKAN_CHECK(
        vkMapMemory(logicalDevice, m_deviceMemory, offset, size, flags, &dest)
    );

    std::memcpy(dest, data, size);

    vkUnmapMemory(logicalDevice, m_deviceMemory);
}

auto Buffer::copyTo(
    const VkCommandPool& commandPool,
    const VkFence& fence,
    const VkQueue& queue,
    const VkBuffer& source,
    const uint64_t sourceOffset,
    const VkBuffer& dest,
    const uint64_t destOffset,
    const uint64_t size
) -> void {
    vkQueueWaitIdle(queue);

    CommandBuffer commandBuffer { m_device };
    commandBuffer.allocateAndBeginSingleUse(commandPool);

    const VkBufferCopy copyRegion {
        sourceOffset, // srcOffset
        destOffset,   // dstOffset
        size          // size
    };

    vkCmdCopyBuffer(commandBuffer.getCommandBuffer(), source, dest, 1u, &copyRegion);

    commandBuffer.endSingleUse(commandPool, queue);
}

} // namespace vulkan
} // namespace renderer
} // namespace beige
