#pragma once

#include "../VulkanDevice.hpp"
#include "../VulkanPipeline.hpp"
#include "../VulkanBuffer.hpp"
#include "../VulkanSwapchain.hpp"
#include "../../RendererTypes.hpp"

#include <vulkan/vulkan.h>

#include <array>
#include <string>

namespace beige {
namespace renderer {
namespace vulkan {

class ShaderObject final {
public:
    ShaderObject(
        VkAllocationCallbacks* allocationCallbacks,
        std::shared_ptr<Device> device,
        std::shared_ptr<RenderPass> renderPass,
        std::shared_ptr<Swapchain> swapchain,
        const uint32_t framebufferWidth,
        const uint32_t framebufferHeight
    );
    ~ShaderObject();

    auto setProjection(const glm::mat4x4& projection) -> void;
    auto setView(const glm::mat4x4& view) -> void;

    auto use(const VkCommandBuffer& commandBuffer) -> void;

    auto updateGlobalState(
        const uint32_t imageIndex,
        const VkCommandBuffer& commandBuffer
    ) -> void;
    auto updateObject(
        const VkCommandBuffer& commandBuffer,
        const glm::mat4x4& model
    ) -> void;

private:
    struct Stage {
        VkShaderModuleCreateInfo shaderModuleCreateInfo;
        VkShaderModule shaderModule;
        VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfo;
    };

    static constexpr uint32_t m_stageCount { 2u };
    static constexpr std::string_view m_builtinShaderName { "Builtin.ObjectShader" };

    VkAllocationCallbacks* m_allocationCallbacks;
    std::shared_ptr<Device> m_device;
    std::shared_ptr<RenderPass> m_renderPass;
    std::shared_ptr<Swapchain> m_swapchain;

    std::array<Stage, m_stageCount> m_stages;

    VkDescriptorPool m_globalDescriptorPool;
    VkDescriptorSetLayout m_globalDescriptorSetLayout;

    // One descriptor set per frame - max 3 for tripple-buffering.
    std::array<VkDescriptorSet, 3u> m_globalDescriptorSets;

    // Global uniform object.
    GlobalUniformObject m_globalUniformObject;

    // Global uniform buffer.
    std::unique_ptr<Buffer> m_globalUniformBuffer;

    std::unique_ptr<Pipeline> m_pipeline;

    auto createShaderModule(
        Stage& stage,
        const std::string& name,
        const std::string& type,
        const VkShaderStageFlagBits shaderStageFlagBits
    ) -> bool;
};

} // namespace vulkan
} // namespace renderer
} // namespace beige
