#include "VulkanCommandBuffer.hpp"

#include "VulkanDefines.hpp"

namespace beige {
namespace renderer {
namespace vulkan {

CommandBuffer::CommandBuffer(
    std::shared_ptr<VulkanDevice> device
) :
m_device { device },
m_commandBuffer { 0 },
m_state { State::Ready } {

}

CommandBuffer::~CommandBuffer() {

}

auto CommandBuffer::getCommandBuffer() const -> const VkCommandBuffer& {
    return m_commandBuffer;
}

auto CommandBuffer::setState(const State state) -> void {
    m_state = state;
}

auto CommandBuffer::allocate(
    const VkCommandPool& commandPool,
    const bool isPrimary
) -> void {
    const VkDevice logicalDevice { m_device->getLogicalDevice() };

    const VkCommandBufferLevel commandBufferLevel {
        isPrimary
        ? VK_COMMAND_BUFFER_LEVEL_PRIMARY
        : VK_COMMAND_BUFFER_LEVEL_SECONDARY
    };

    const VkCommandBufferAllocateInfo commandBufferAllocateInfo {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, // sType
        nullptr, // pNext
        commandPool, // commandPool
        commandBufferLevel, // level
        1u //commandBufferCount;
    };

    m_state = State::NotAllocated;

    VULKAN_CHECK(
        vkAllocateCommandBuffers(
            logicalDevice,
            &commandBufferAllocateInfo,
            &m_commandBuffer
        )
    );

    m_state = State::Ready;
}

auto CommandBuffer::free(const VkCommandPool& commandPool) -> void {
    const VkDevice logicalDevice { m_device->getLogicalDevice() };

    vkFreeCommandBuffers(
        logicalDevice,
        commandPool,
        1u,
        &m_commandBuffer
    );

    m_commandBuffer = 0;
    m_state = State::NotAllocated;
}

auto CommandBuffer::begin(
    const bool isSingleUse,
    const bool isRenderPassContinue,
    const bool isSimultaneousUse
) -> void {
    const VkCommandBufferUsageFlags commandBufferUsageFlags {
        isSingleUse ? VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : 0u |
        isRenderPassContinue ? VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT : 0u |
        isSimultaneousUse ? VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT : 0u
    };

    const VkCommandBufferBeginInfo commandBufferBeginInfo {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, // sType
        nullptr, // pNext
        commandBufferUsageFlags, // flags
        nullptr // pInheritanceInfo
    };

    VULKAN_CHECK(
        vkBeginCommandBuffer(
            m_commandBuffer,
            &commandBufferBeginInfo
        )
    );

    m_state = State::Recording;
}

auto CommandBuffer::end() -> void {
    VULKAN_CHECK(
        vkEndCommandBuffer(
            m_commandBuffer
        )
    );

    m_state = State::RecordingEnded;
}

auto CommandBuffer::updateSubmitted() -> void {
    m_state = State::Submitted;
}

auto CommandBuffer::reset() -> void {
    m_state = State::Ready;
}

auto CommandBuffer::allocateAndBeginSingleUse(
    const VkCommandPool& commandPool
) -> void {
    allocate(commandPool, true);
    begin(true, false, false);
}

auto CommandBuffer::endSingleUse(
    const VkCommandPool& commandPool,
    const VkQueue& queue
) -> void {
    // End of the command buffer
    end();

    // Submit the queue
    const VkSubmitInfo submitInfo {
        VK_STRUCTURE_TYPE_SUBMIT_INFO, // sType
        nullptr, // pNext
        0u, // waitSemaphoreCount
        nullptr, // pWaitSemaphores
        nullptr, // pWaitDstStageMask
        1u, // commandBufferCount
        &m_commandBuffer, // pCommandBuffers
        0u, // signalSemaphoreCount
        nullptr // pSignalSemaphores
    };

    VULKAN_CHECK(
        vkQueueSubmit(
            queue,
            1u,
            &submitInfo,
            0
        )
    );

    // Wait for finish it
    VULKAN_CHECK(vkQueueWaitIdle(queue));

    // Free the command buffer
    free(commandPool);
}

} // namespace vulkan
} // namespace renderer
} // namespace beige
