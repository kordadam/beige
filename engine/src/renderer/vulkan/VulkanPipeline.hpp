#pragma once

#include "VulkanRenderPass.hpp"
#include "VulkanCommandBuffer.hpp"

namespace beige {
namespace renderer {
namespace vulkan {

class Pipeline final {
public:
    Pipeline(
        VkAllocationCallbacks* allocationCallbacks,
        std::shared_ptr<Device> device,
        std::shared_ptr<RenderPass> renderPass,
        const std::vector<VkVertexInputAttributeDescription>& vertexInputAttributeDescriptions,
        const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts,
        const std::vector<VkPipelineShaderStageCreateInfo>& pipelineShaderStageCreateInfos,
        const VkViewport& viewport,
        const VkRect2D& scissor,
        const bool isWireframe
    );

    ~Pipeline();

    auto getPipelineLayout() const -> const VkPipelineLayout&;

    auto bind(
        const VkCommandBuffer& commandBuffer,
        const VkPipelineBindPoint& pipelineBindPoint
    ) -> void;

private:
    VkAllocationCallbacks* m_allocationCallbacks;
    std::shared_ptr<Device> m_device;
    std::shared_ptr<RenderPass> m_renderPass;

    VkPipelineLayout m_pipelineLayout;
    VkPipeline m_handle;
};

} // namespace vulkan
} // namespace renderer
} // namespace beige
