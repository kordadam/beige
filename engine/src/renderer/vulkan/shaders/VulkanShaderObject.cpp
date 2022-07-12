#include "VulkanShaderObject.hpp"

#include "../../../core/Logger.hpp"
#include "../VulkanDefines.hpp"
#include "../../../math/Math.hpp"

#include <map>
#include <fstream>
#include <algorithm>
#include <vector>

namespace beige {
namespace renderer {
namespace vulkan {

ShaderObject::ShaderObject(
    VkAllocationCallbacks* allocationCallbacks,
    std::shared_ptr<Device> device,
    std::shared_ptr<RenderPass> renderPass,
    const uint32_t framebufferWidth,
    const uint32_t framebufferHeight
) :
m_allocationCallbacks { allocationCallbacks },
m_device { device },
m_renderPass { renderPass },
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

    // TODO: Descriptors

    const VkViewport viewport {
        0.0f,                                   // x
        static_cast<float>(framebufferHeight),  // y
        static_cast<float>(framebufferWidth),   // width
        -static_cast<float>(framebufferHeight), // height
        0.0f,                                   // minDepth
        1.0f                                    // maxDepth
    };

    const VkOffset2D scissorOffset {
        0, // x
        0  // y
    };

    const VkExtent2D scissorExtent {
        framebufferWidth, // width
        framebufferHeight // height
    };

    const VkRect2D scissor {
        scissorOffset, // offset
        scissorExtent  // extent
    };

    uint32_t offset { 0u };

    const uint32_t attributeCount { 1u };

    const std::array<VkFormat, attributeCount> formats {
        VK_FORMAT_R32G32B32_SFLOAT
    };

    const std::array<uint32_t, attributeCount> sizes {
        sizeof(math::Vertex3D)
    };

    std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescriptions { attributeCount };

    for (uint32_t i { 0u }; i < attributeCount; i++) {
        const VkVertexInputAttributeDescription vertexInputAttributeDescription {
            i,             // location
            0u,            // binding
            formats.at(i), // format
            offset         // offset
        };

        vertexInputAttributeDescriptions.at(i) = vertexInputAttributeDescription;
        offset += sizes.at(i);
    }

    // TODO: Descriptor set layouts
    const std::vector<VkDescriptorSetLayout> descriptorSetLayout;

    std::vector<VkPipelineShaderStageCreateInfo> pipelineShaderStageCreateInfos { m_stageCount };
    for (uint32_t i { 0u }; i < m_stageCount; i++) {
        pipelineShaderStageCreateInfos.at(i) = m_stages.at(i).pipelineShaderStageCreateInfo;
    }

    m_pipeline = std::make_shared<Pipeline>(
        m_allocationCallbacks,
        m_device,
        m_renderPass,
        vertexInputAttributeDescriptions,
        descriptorSetLayout,
        pipelineShaderStageCreateInfos,
        viewport,
        scissor,
        false
    );
}

ShaderObject::~ShaderObject() {
    const VkDevice logicalDevice { m_device->getLogicalDevice() };

    m_pipeline.reset();

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

auto ShaderObject::use(const VkCommandBuffer& commandBuffer) -> void {
    m_pipeline->bind(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

auto ShaderObject::createShaderModule(
    Stage& stage,
    const std::string& name,
    const std::string& type,
    const VkShaderStageFlagBits shaderStageFlagBits
) -> bool {
    stage.shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

    const std::string path { "assets/shaders/" + name + "." + type + ".glsl.spv" };
    std::ifstream file { path, std::ios::binary | std::ios::ate };

    if (!file.good()) {
        core::Logger::error("Shader module file error: " + path + "!");
        return false;
    }

    std::streampos end { file.tellg() };
    file.seekg(0, std::ios::beg);

    std::size_t size { std::size_t(end - file.tellg()) };

    if (size == 0u) {
        core::Logger::error("Shader file is empty: " + path + "!");
        return false;
    }

    std::vector<std::byte> buffer { size };

    if (!file.read((char*)buffer.data(), buffer.size())) {
        core::Logger::error("Unable to read shader module: " + path + "!");
        return false;
    }

    stage.shaderModuleCreateInfo.codeSize = size;
    stage.shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(buffer.data());

    file.close();

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
