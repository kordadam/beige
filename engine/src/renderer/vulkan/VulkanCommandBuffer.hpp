#pragma once

#include "VulkanDevice.hpp"

#include <vulkan/vulkan.h>

#include <memory>

namespace beige {
namespace renderer {
namespace vulkan {

class CommandBuffer final {
public:
    enum class State : uint32_t {
        Ready,
        Recording,
        InRenderPass,
        RecordingEnded,
        Submitted,
        NotAllocated
    };

    CommandBuffer(
        std::shared_ptr<Device> device
    );

    ~CommandBuffer();

    auto getHandle() const -> const VkCommandBuffer&;
    auto setState(const State state) -> void;

    auto allocate(
        const VkCommandPool& commandPool,
        const bool isPrimary
    ) -> void;

    auto free(const VkCommandPool& commandPool) -> void;

    auto begin(
        const bool isSingleUse,
        const bool isRenderPassContinue,
        const bool isSimultaneousUse
    ) -> void;

    auto end() -> void;
    auto updateSubmitted() -> void;
    auto reset() -> void;

    auto allocateAndBeginSingleUse(
        const VkCommandPool& commandPool
    ) -> void;

    auto endSingleUse(
        const VkCommandPool& commandPool,
        const VkQueue& queue
    ) -> void;

private:
    std::shared_ptr<Device> m_device;

    VkCommandBuffer m_handle;
    State m_state;
};

} // namespace vulkan
} // namespace renderer
} // namespace beige
