#pragma once

#include "../VulkanDevice.hpp"
#include "../VulkanPipeline.hpp"
#include "../VulkanBuffer.hpp"
#include "../VulkanSwapchain.hpp"
#include "../../RendererTypes.hpp"
#include "../VulkanTexture.hpp"

#include <vulkan/vulkan.h>

#include <array>
#include <string>
#include <optional>
#include <memory>

namespace beige {
namespace renderer {
namespace vulkan {

class ShaderObject final {
private:
    static constexpr uint32_t m_stageCount{ 2u };
    static constexpr uint32_t m_descriptorCount{ 2u };
    static constexpr uint32_t m_maxObjectCount{ 1024u };
    static constexpr std::string_view m_builtinShaderName{ "Builtin.ObjectShader" };

public:
    struct DescriptorState {
        // One per frame.
        std::array<uint32_t, 3u> generations;
    };

    struct ObjectState {
        // Per frame.
        std::array<VkDescriptorSet, 3u> descriptorSets;

        // Per descriptor.
        std::array<DescriptorState, m_descriptorCount> descriptorStates;
    };

    ShaderObject(
        VkAllocationCallbacks* allocationCallbacks,
        std::shared_ptr<Device> device,
        std::shared_ptr<RenderPass> renderPass,
        std::shared_ptr<Swapchain> swapchain,
        const uint32_t framebufferWidth,
        const uint32_t framebufferHeight,
        std::shared_ptr<Texture> defaultDiffuse
    );
    ~ShaderObject();

    auto setProjection(const glm::mat4x4& projection) -> void;
    auto setView(const glm::mat4x4& view) -> void;

    auto use(const VkCommandBuffer& commandBuffer) -> void;

    auto updateGlobalState(
        const uint32_t imageIndex,
        const VkCommandBuffer& commandBuffer,
        const float deltaTime
    ) -> void;
    auto updateObject(
        const VkCommandBuffer& commandBuffer,
        const uint32_t imageIndex,
        const GeometryRenderData& geometryRenderData,
        const float deltaTime // TODO: Temporary.
    ) -> void;
    auto acquireResources() -> std::optional<ObjectId>;
    auto releaseResources(const ObjectId objectId) -> void;

private:
    struct Stage {
        VkShaderModuleCreateInfo shaderModuleCreateInfo;
        VkShaderModule shaderModule;
        VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfo;
    };

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

    VkDescriptorPool m_objectDescriptorPool;
    VkDescriptorSetLayout m_objectDescriptorSetLayout;
    // Object uniform buffers.
    std::unique_ptr<Buffer> m_objectUniformBuffer;
    // TODO: Manage a free list of some kind here instead.
    uint32_t m_objectUniformBufferIndex;

    // TODO: Make dynamic.
    std::array<ObjectState, m_maxObjectCount> m_objectStates;

    // TODO: Pointers to default textures.
    std::shared_ptr<Texture> m_defaultDiffuse;

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
