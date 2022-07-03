#pragma once

#include "../VulkanDevice.hpp"

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
        std::shared_ptr<Device> device
    );
    ~ShaderObject();

    auto use() -> void;

private:
    struct Stage {
        VkShaderModuleCreateInfo shaderModuleCreateInfo;
        VkShaderModule shaderModule;
        VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfo;
    };

    struct Pipeline {
        VkPipeline pipeline;
        VkPipelineLayout pipelineLayout;
    };

    static constexpr uint32_t m_stageCount { 2u };
    static constexpr std::string_view m_builtinShaderName { "Builtin.ObjectShader" };

    VkAllocationCallbacks* m_allocationCallbacks;
    std::shared_ptr<Device> m_device;

    std::array<Stage, m_stageCount> m_stages;
    Pipeline m_pipeline;

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
