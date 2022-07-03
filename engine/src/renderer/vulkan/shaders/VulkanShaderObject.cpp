#include "VulkanShaderObject.hpp"

#include "../../../core/Logger.hpp"
#include "../VulkanDefines.hpp"

#include <map>
#include <fstream>
#include <filesystem>
#include <algorithm>

namespace beige {
namespace renderer {
namespace vulkan {

ShaderObject::ShaderObject(
    VkAllocationCallbacks* allocationCallbacks,
    std::shared_ptr<Device> device
) :
m_allocationCallbacks { allocationCallbacks },
m_device { device },
m_stages { },
m_pipeline { } {
    const std::array<std::string, m_stageCount> shaderTypeStrings { "vert", "frag" };
    const std::array<VkShaderStageFlagBits, m_stageCount> shaderTypeStageFlagBits {
        VK_SHADER_STAGE_VERTEX_BIT,
        VK_SHADER_STAGE_FRAGMENT_BIT
    };

    for (uint32_t i { 0u }; i < m_stageCount; i++) {
        if (!createShaderModule(m_stages.at(i), m_builtinShaderName.data(), shaderTypeStrings.at(i), shaderTypeStageFlagBits.at(i))) {
            const std::string message {
                "Unable to create " + shaderTypeStrings.at(i) + " shader module for " + m_builtinShaderName.data() + "!"
            };

            throw std::exception(message.c_str());
        }
    }
}

ShaderObject::~ShaderObject() {
    const VkDevice logicalDevice { m_device->getLogicalDevice() };

    std::for_each(
        m_stages.begin(),
        m_stages.end(),
        [&](const Stage& stage) -> void {
            vkDestroyShaderModule(
                logicalDevice,
                stage.shaderModule,
                m_allocationCallbacks
            );
        }
    );
}

auto ShaderObject::use() -> void {

}

auto ShaderObject::createShaderModule(
    Stage& stage,
    const std::string& name,
    const std::string& type,
    const VkShaderStageFlagBits shaderStageFlagBits
) -> bool {
    stage.shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

    std::filesystem::path path { "assets/shaders/" + name + "." + type + ".glsl.spv" };
    std::ifstream file { path, std::ios::binary | std::ios::ate };

    if (!file.good()) {
        core::Logger::error("Shader module file error: " + path.string() + "!");
        return false;
    }

    std::streampos end { file.tellg() };
    file.seekg(0, std::ios::beg);

    std::size_t size { std::size_t(end - file.tellg()) };

    if (size == 0u) {
        core::Logger::error("Shader file is empty: " + path.string() + "!");
        return false;
    }

    std::vector<std::byte> buffer { size };

    if (!file.read((char*)buffer.data(), buffer.size())) {
        core::Logger::error("Unable to read shader module: " + path.string() + "!");
        return false;
    }

    stage.shaderModuleCreateInfo.codeSize = size;
    stage.shaderModuleCreateInfo.pCode = (uint32_t*)(buffer.data());

    VULKAN_CHECK(
        vkCreateShaderModule(
            m_device->getLogicalDevice(),
            &stage.shaderModuleCreateInfo,
            m_allocationCallbacks,
            &stage.shaderModule
        )
    );

    stage.pipelineShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stage.pipelineShaderStageCreateInfo.stage = shaderStageFlagBits;
    stage.pipelineShaderStageCreateInfo.module = stage.shaderModule;
    stage.pipelineShaderStageCreateInfo.pName = "main";

    return true;
}

} // namespace vulkan
} // namespace renderer
} // namespace beige
