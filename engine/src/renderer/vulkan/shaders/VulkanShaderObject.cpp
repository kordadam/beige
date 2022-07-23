#include "VulkanShaderObject.hpp"

#include "../../../core/Logger.hpp"
#include "../VulkanDefines.hpp"

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
    std::shared_ptr<Swapchain> swapchain,
    const uint32_t framebufferWidth,
    const uint32_t framebufferHeight
) :
m_allocationCallbacks { allocationCallbacks },
m_device { device },
m_renderPass { renderPass },
m_swapchain { swapchain },
m_stages { },
m_globalDescriptorPool { VK_NULL_HANDLE },
m_globalDescriptorSetLayout { VK_NULL_HANDLE },
m_globalDescriptorSets { },
m_globalUniformObject { },
m_globalUniformBuffer { nullptr },
m_pipeline { nullptr } {
    // Shader module initialization per stage.
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

    // Global descriptors.
    const VkDescriptorSetLayoutBinding globalUniformObjectDescriptorSetLayoutBinding {
        0u,                                // binding
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, // descriptorType
        1u,                                // descriptorCount
        VK_SHADER_STAGE_VERTEX_BIT,        // stageFlags
        nullptr                            // pImmutableSamplers
    };

    const VkDescriptorSetLayoutCreateInfo globalDescriptorSetLayoutCreateInfo {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, // sType
        nullptr,                                             // pNext
        0u,                                                  // flags
        1u,                                                  // bindingCount
        &globalUniformObjectDescriptorSetLayoutBinding       // pBindings
    };

    VULKAN_CHECK(
        vkCreateDescriptorSetLayout(
            m_device->getLogicalDevice(),
            &globalDescriptorSetLayoutCreateInfo,
            m_allocationCallbacks,
            &m_globalDescriptorSetLayout
        )
    );

    // Global descriptor pool - used for global items such as view or projection matrix.
    const uint32_t imageCount {
        static_cast<uint32_t>(m_swapchain->getImages().size())
    };

    const VkDescriptorPoolSize globalDescriptorPoolSize {
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, // type
        imageCount                         // descriptorCount
    };

    const VkDescriptorPoolCreateInfo globalDescriptorPoolCreateInfo {
        VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO, // sType
        nullptr,                                       // pNext
        0u,                                            // flags
        imageCount,                                    // maxSets
        1u,                                            // poolSizeCount
        &globalDescriptorPoolSize                      // pPoolSizes
    };

    VULKAN_CHECK(
        vkCreateDescriptorPool(
            m_device->getLogicalDevice(),
            &globalDescriptorPoolCreateInfo,
            m_allocationCallbacks,
            &m_globalDescriptorPool
        )
    );

    // Pipeline creation.
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
        sizeof(glm::vec3)
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

    // Descriptor set layouts.
    const std::vector<VkDescriptorSetLayout> descriptorSetLayouts {
        m_globalDescriptorSetLayout
    };

    std::vector<VkPipelineShaderStageCreateInfo> pipelineShaderStageCreateInfos { m_stageCount };
    for (uint32_t i { 0u }; i < m_stageCount; i++) {
        pipelineShaderStageCreateInfos.at(i) = m_stages.at(i).pipelineShaderStageCreateInfo;
    }

    m_pipeline = std::make_unique<Pipeline>(
        m_allocationCallbacks,
        m_device,
        m_renderPass,
        vertexInputAttributeDescriptions,
        descriptorSetLayouts,
        pipelineShaderStageCreateInfos,
        viewport,
        scissor,
        false
    );

    // Create uniform buffer.
    const VkBufferUsageFlags globalUniformBufferUsageFlags {
        VK_BUFFER_USAGE_TRANSFER_DST_BIT |
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
    };

    const uint32_t globalUniformBufferMemoryPropertyFlags {
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    };

    m_globalUniformBuffer = std::make_unique<Buffer>(
        m_allocationCallbacks,
        m_device,
        static_cast<uint64_t>(sizeof(GlobalUniformObject)),
        globalUniformBufferUsageFlags,
        globalUniformBufferMemoryPropertyFlags,
        true
    );

    // Allocate global descriptor sets.
    const std::array<VkDescriptorSetLayout, 3u> globalDescriptorSetLayouts {
        m_globalDescriptorSetLayout,
        m_globalDescriptorSetLayout,
        m_globalDescriptorSetLayout
    };

    const VkDescriptorSetAllocateInfo globalDescriptorSetAllocateInfo {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, // sType
        nullptr,                                        // pNext
        m_globalDescriptorPool,                         // descriptorPool
        3u,                                             // descriptorSetCount
        globalDescriptorSetLayouts.data()               // pSetLayouts
    };

    VULKAN_CHECK(
        vkAllocateDescriptorSets(
            m_device->getLogicalDevice(),
            &globalDescriptorSetAllocateInfo,
            m_globalDescriptorSets.data()
        )
    );
}

ShaderObject::~ShaderObject() {
    const VkDevice logicalDevice { m_device->getLogicalDevice() };

    // Destroy uniform buffer.
    m_globalUniformBuffer.reset();

    // Destroy pipeline.
    m_pipeline.reset();

    // Destroy global descriptor pool.
    vkDestroyDescriptorPool(
        logicalDevice,
        m_globalDescriptorPool,
        m_allocationCallbacks
    );

    // Destroy global descriptor set layout.
    vkDestroyDescriptorSetLayout(
        logicalDevice,
        m_globalDescriptorSetLayout,
        m_allocationCallbacks
    );

    // Destroy shader modules.
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

auto ShaderObject::setProjection(const glm::mat4x4& projection) -> void {
    m_globalUniformObject.projection = projection;
}

auto ShaderObject::setView(const glm::mat4x4& view) -> void {
    m_globalUniformObject.view = view;
}

auto ShaderObject::use(const VkCommandBuffer& commandBuffer) -> void {
    m_pipeline->bind(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
}


auto ShaderObject::updateGlobalState(
    const uint32_t imageIndex,
    const VkCommandBuffer& commandBuffer
) -> void {
    const VkDescriptorSet globalDescriptorSet { m_globalDescriptorSets.at(imageIndex) };

    // Bind the global descriptor set to be updated.
    vkCmdBindDescriptorSets(
        commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        m_pipeline->getPipelineLayout(),
        0u,
        1u,
        &globalDescriptorSet,
        0u,
        nullptr
    );

    // Configure the descriptors for the given index.
    const uint32_t range { static_cast<uint32_t>(sizeof(GlobalUniformObject)) };
    const uint32_t offset { 0u };

    // Copy data to buffer.
    m_globalUniformBuffer->loadData(offset, range, 0u, &m_globalUniformObject);

    const VkDescriptorBufferInfo descriptorBufferInfo {
        m_globalUniformBuffer->getHandle(), // buffer
        offset,                             // offset
        range                               // range
    };

    // Update descriptor sets.
    const VkWriteDescriptorSet writeDescriptorSet {
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, // sType
        nullptr,                                // pNext
        m_globalDescriptorSets.at(imageIndex),  // dstSet
        0u,                                     // dstBinding
        0u,                                     // dstArrayElement
        1u,                                     // descriptorCount
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,      // descriptorType
        nullptr,                                // pImageInfo
        &descriptorBufferInfo,                  // pBufferInfo
        nullptr                                 // pTexelBufferView
    };

    vkUpdateDescriptorSets(
        m_device->getLogicalDevice(),
        1u,
        &writeDescriptorSet,
        0u,
        nullptr
    );
}

auto ShaderObject::updateObject(
    const VkCommandBuffer& commandBuffer,
    const glm::mat4x4& model
) -> void {
    vkCmdPushConstants(
        commandBuffer,
        m_pipeline->getPipelineLayout(),
        VK_SHADER_STAGE_VERTEX_BIT,
        0u,
        static_cast<uint32_t>(sizeof(glm::mat4x4)),
        static_cast<const void*>(&model)
    );
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
