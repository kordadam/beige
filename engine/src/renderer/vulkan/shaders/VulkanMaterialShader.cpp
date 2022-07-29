#include "VulkanMaterialShader.hpp"

#include "../../../core/Logger.hpp"
#include "../VulkanDefines.hpp"
#include "../VulkanTexture.hpp"

#include <map>
#include <fstream>
#include <algorithm>
#include <vector>
#include <memory>

namespace beige {
namespace renderer {
namespace vulkan {

MaterialShader::MaterialShader(
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
m_objectDescriptorPool { VK_NULL_HANDLE },
m_objectDescriptorSetLayout { VK_NULL_HANDLE },
m_objectUniformBuffer { nullptr },
m_objectUniformBufferIndex { 0u },
m_objectStates { },
m_pipeline { nullptr } {
    // Shader module initialization per stage.
    const std::array<std::string, m_stageCount> shaderTypeStrings { "vert", "frag" };
    const std::array<VkShaderStageFlagBits, m_stageCount> shaderTypeStageFlagBits {
        VK_SHADER_STAGE_VERTEX_BIT,
        VK_SHADER_STAGE_FRAGMENT_BIT
    };

    for (uint32_t i { 0u }; i < m_stageCount; i++) {
        if (!createShaderModule(m_stages.at(i), m_builtinMaterialShaderName.data(), shaderTypeStrings.at(i), shaderTypeStageFlagBits.at(i))) {
            const std::string message {
                "Unable to create " + shaderTypeStrings.at(i) + " shader module for " + m_builtinMaterialShaderName.data() + "!"
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

    // Local/object descriptors.
    const uint32_t localSamplerCount { 1u };
    const std::array<VkDescriptorType, m_descriptorCount> descriptorTypes {
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,        // Binding 0 - uniform buffer.
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER // Binding 1 - diffuse sampler layout.
    };

    std::array<VkDescriptorSetLayoutBinding, m_descriptorCount> descriptorSetLayoutBindings { };

    for (uint32_t i { 0u }; i < m_descriptorCount; i++) {
        descriptorSetLayoutBindings.at(i).binding = i;
        descriptorSetLayoutBindings.at(i).descriptorType = descriptorTypes.at(i);
        descriptorSetLayoutBindings.at(i).descriptorCount = 1u;
        descriptorSetLayoutBindings.at(i).stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        descriptorSetLayoutBindings.at(i).pImmutableSamplers = nullptr;
    }

    const VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, // sType
        nullptr,                                             // pNext
        0u,                                                  // flags
        m_descriptorCount,                                   // bindingCount
        descriptorSetLayoutBindings.data()                   // pBindings
    };

    VULKAN_CHECK(
        vkCreateDescriptorSetLayout(
            m_device->getLogicalDevice(),
            &descriptorSetLayoutCreateInfo,
            m_allocationCallbacks,
            &m_objectDescriptorSetLayout
        )
    );

    // Local/object descriptor pool - used for object-specific items like diffuse color.
    // The first section will be used for uniform buffers.
    const VkDescriptorPoolSize uniformBuffersDescriptorPoolSize {
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, // type
        m_maxObjectCount                   // descriptorCount
    };

    // The second section will be used for image samplers.
    const VkDescriptorPoolSize imageSamplersDescriptorPoolSize {
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, // type
        localSamplerCount * m_maxObjectCount       // descriptorCount
    };

    const std::array<VkDescriptorPoolSize, 2u> objectDescriptorPoolSizes {
        uniformBuffersDescriptorPoolSize,
        imageSamplersDescriptorPoolSize
    };

    const VkDescriptorPoolCreateInfo objectDescriptorPoolCreateInfo {
        VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,           // sType
        nullptr,                                                 // pNext
        0u,                                                      // flags
        m_maxObjectCount,                                        // maxSets
        static_cast<uint32_t>(objectDescriptorPoolSizes.size()), // poolSizeCount
        objectDescriptorPoolSizes.data()                         // pPoolSizes
    };

    // Create object descriptor pool.
    VULKAN_CHECK(
        vkCreateDescriptorPool(
            m_device->getLogicalDevice(),
            &objectDescriptorPoolCreateInfo,
            m_allocationCallbacks,
            &m_objectDescriptorPool
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

    const uint32_t attributeCount { 2u };

    const std::array<VkFormat, attributeCount> formats {
        VK_FORMAT_R32G32B32_SFLOAT,
        VK_FORMAT_R32G32_SFLOAT
    };

    const std::array<uint32_t, attributeCount> sizes {
        static_cast<uint32_t>(sizeof(glm::vec3)),
        static_cast<uint32_t>(sizeof(glm::vec2))
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
        m_globalDescriptorSetLayout,
        m_objectDescriptorSetLayout
    };

    std::vector<VkPipelineShaderStageCreateInfo> pipelineShaderStageCreateInfos { m_stageCount };
    for (uint32_t i { 0u }; i < m_stageCount; i++) {
        pipelineShaderStageCreateInfos.at(i).sType = m_stages.at(i).pipelineShaderStageCreateInfo.sType;
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

    const uint32_t deviceLocalBits {
        m_device->supportsDeviceLocalHostVisible()
        ? VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        : 0u
    };

    const uint32_t globalUniformBufferMemoryPropertyFlags {
        deviceLocalBits |
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

    // Create the object uniform buffer.
    const VkBufferUsageFlags objectUniformBufferUsageFlags {
        VK_BUFFER_USAGE_TRANSFER_DST_BIT |
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
    };

    const uint32_t objectUniformBufferMemoryPropertyFlags {
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    };

    m_objectUniformBuffer = std::make_unique<Buffer>(
        m_allocationCallbacks,
        m_device,
        static_cast<uint64_t>(sizeof(ObjectUniformObject)),
        objectUniformBufferUsageFlags,
        objectUniformBufferMemoryPropertyFlags,
        true
    );
}

MaterialShader::~MaterialShader() {
    const VkDevice logicalDevice { m_device->getLogicalDevice() };

    // Destroy object descriptor pool.
    vkDestroyDescriptorPool(
        logicalDevice,
        m_objectDescriptorPool,
        m_allocationCallbacks
    );

    // Destroy object descriptor set layout.
    vkDestroyDescriptorSetLayout(
        logicalDevice,
        m_objectDescriptorSetLayout,
        m_allocationCallbacks
    );

    // Destroy object uniform buffer.
    m_objectUniformBuffer.reset();

    // Destroy global uniform buffer.
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

auto MaterialShader::setProjection(const glm::mat4x4& projection) -> void {
    m_globalUniformObject.projection = projection;
}

auto MaterialShader::setView(const glm::mat4x4& view) -> void {
    m_globalUniformObject.view = view;
}

auto MaterialShader::use(const VkCommandBuffer& commandBuffer) -> void {
    m_pipeline->bind(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
}


auto MaterialShader::updateGlobalState(
    const uint32_t imageIndex,
    const VkCommandBuffer& commandBuffer,
    const float deltaTime
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

auto MaterialShader::updateObject(
    const VkCommandBuffer& commandBuffer,
    const uint32_t imageIndex,
    const GeometryRenderData& geometryRenderData,
    const float deltaTime // TODO: Temporary.
) -> void {
    vkCmdPushConstants(
        commandBuffer,
        m_pipeline->getPipelineLayout(),
        VK_SHADER_STAGE_VERTEX_BIT,
        0u,
        static_cast<uint32_t>(sizeof(glm::mat4x4)),
        static_cast<const void*>(&geometryRenderData.model)
    );

    // Obtain material data.
    ObjectState& objectState { m_objectStates.at(geometryRenderData.objectId) };
    const VkDescriptorSet objectDescriptorSet { objectState.descriptorSets.at(imageIndex) };

    // TODO: If needs update.
    std::array<VkWriteDescriptorSet, m_descriptorCount> writeDescriptorSets { };

    // Descriptor 0 - uniform buffer.
    const uint32_t range { static_cast<uint32_t>(sizeof(ObjectUniformObject)) };
    const uint32_t offset { static_cast<uint32_t>(sizeof(ObjectUniformObject)) * geometryRenderData.objectId }; // Also the index into the array.
    ObjectUniformObject objectUniformObject;

    // TODO: Get diffuse color from a material.
    static float accumulator { 0.0f };
    accumulator += deltaTime;
    const float s { (std::sin(accumulator) + 1.0f) / 2.0f }; // Scale from -1, 1 to 0, 1.
    objectUniformObject.diffuseColor = glm::vec4(s, s, s, 1.0f);

    // Load the data into the buffer.
    m_objectUniformBuffer->loadData(offset, range, 0u, &objectUniformObject);

    uint32_t descriptorIndex { 0u };
    uint32_t descriptorCount { 0u };

    // Only do this if the descriptor has not yet been updated.
    if (objectState.descriptorStates.at(descriptorIndex).generations.at(imageIndex) == resources::global_invalidTextureGeneration) {
        const VkDescriptorBufferInfo descriptorBufferInfo {
            m_objectUniformBuffer->getHandle(), // buffer
            offset,                             // offset
            range                               // range
        };

        const VkWriteDescriptorSet writeDescriptorSet {
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, // sType
            nullptr,                                // pNext
            objectDescriptorSet,                    // dstSet
            descriptorIndex,                        // dstBinding
            0u,                                     // dstArrayElement
            1u,                                     // descriptorCount
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,      // descriptorType
            nullptr,                                // pImageInfo
            &descriptorBufferInfo,                  // pBufferInfo
            nullptr                                 // pTexelBufferView
        };

        writeDescriptorSets.at(descriptorCount) = writeDescriptorSet;
        descriptorCount++;

        // Update the frame generation. In this case it is only needed once since this is a buffer.
        objectState.descriptorStates.at(descriptorIndex).generations.at(imageIndex) = 1u;
    }
    descriptorIndex++;

    // TODO: Samplers.
    const uint32_t samplerCount { 1u };
    std::array<VkDescriptorImageInfo, 1u> descriptorImageInfos { };
    for (uint32_t samplerIndex { 0u }; samplerIndex < samplerCount; samplerIndex++) {
        std::shared_ptr<Texture> texture {
            std::dynamic_pointer_cast<Texture>(geometryRenderData.textures.at(samplerIndex))
        };

        uint32_t& descriptorGeneration { objectState.descriptorStates.at(descriptorIndex).generations.at(imageIndex) };
        uint32_t& descriptorId { objectState.descriptorStates.at(descriptorIndex).ids.at(imageIndex) };

        // If the texture hasn't been loaded yet, use the default.
        // TODO: Determine which use the texture has and pull appropriate default based on that.
        if (texture == nullptr || texture->getGeneration() == resources::global_invalidTextureGeneration) {
            texture = nullptr; // TODO: Refactor with texture system.

            // Reset the descriptor generation if using the default texture.
            descriptorGeneration = resources::global_invalidTextureGeneration;
        }

        // Check if the descriptor needs updating first.
        if (
            texture != nullptr &&
            (descriptorId != texture->getId() || descriptorGeneration != texture->getGeneration() || descriptorGeneration == resources::global_invalidTextureGeneration)
        ) {
            // Assign view to sampler.
            descriptorImageInfos.at(samplerIndex).imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            descriptorImageInfos.at(samplerIndex).imageView = texture->getImageView();
            descriptorImageInfos.at(samplerIndex).sampler = texture->getSampler();

            const VkWriteDescriptorSet writeDescriptorSet {
                VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,    // sType
                nullptr,                                   // pNext
                objectDescriptorSet,                       // dstSet
                descriptorIndex,                           // dstBinding
                0u,                                        // dstArrayElement
                1u,                                        // descriptorCount
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, // descriptorType
                &descriptorImageInfos.at(samplerIndex),    // pImageInfo
                nullptr,                                   // pBufferInfo
                nullptr                                    // pTexelBufferView
            };

            writeDescriptorSets.at(descriptorCount) = writeDescriptorSet;
            descriptorCount++;

            // Sync frame generation if not using a default texture.
            if (texture->getGeneration() != resources::global_invalidTextureGeneration) {
                descriptorGeneration = texture->getGeneration();
                descriptorId = texture->getId();
            }

            descriptorIndex++;
        }
    }


    if (descriptorCount > 0u) {
        vkUpdateDescriptorSets(
            m_device->getLogicalDevice(),
            descriptorCount,
            writeDescriptorSets.data(),
            0u,
            nullptr
        );
    }

    // Bind the descriptor set to be updated, or in case the shader changed.
    vkCmdBindDescriptorSets(
        commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        m_pipeline->getPipelineLayout(),
        1u,
        1u,
        &objectDescriptorSet,
        0u,
        nullptr
    );
}

auto MaterialShader::acquireResources() -> std::optional<resources::ObjectId> {
    // TODO: Free list.
    const resources::ObjectId objectId { m_objectUniformBufferIndex };
    m_objectUniformBufferIndex++;

    ObjectState& objectState { m_objectStates.at(objectId) };
    for (uint32_t i { 0u }; i < objectState.descriptorStates.size(); i++) {
        for (uint32_t j { 0u }; j < 3u; j++) {
            objectState.descriptorStates.at(i).generations.at(j) = resources::global_invalidTextureGeneration;
            objectState.descriptorStates.at(i).ids.at(j) = resources::global_invalidObjectId;
        }
    }

    // Allocate descriptor sets.
    const std::array<VkDescriptorSetLayout, 3u> descriptorSetLayouts {
        m_objectDescriptorSetLayout,
        m_objectDescriptorSetLayout,
        m_objectDescriptorSetLayout
    };

    const VkDescriptorSetAllocateInfo descriptorSetAllocateInfo {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, // sType
        nullptr,                                        // pNext
        m_objectDescriptorPool,                         // descriptorPool
        3u,                                             // descriptorSetCount - one per frame
        descriptorSetLayouts.data()                     // pSetLayouts
    };

    const VkResult result {
        vkAllocateDescriptorSets(
            m_device->getLogicalDevice(),
            &descriptorSetAllocateInfo,
            objectState.descriptorSets.data()
        )
    };

    if (result != VK_SUCCESS) {
        core::Logger::error("Error allocating sets in shader!");
        return std::nullopt;
    }

    return std::optional<resources::ObjectId>(objectId);
}

auto MaterialShader::releaseResources(const resources::ObjectId objectId) -> void {
    ObjectState& objectState { m_objectStates.at(objectId) };

    const uint32_t descriptorSetCount { 3u };

    // Release object descriptor sets.
    const VkResult result {
        vkFreeDescriptorSets(
            m_device->getLogicalDevice(),
            m_objectDescriptorPool,
            descriptorSetCount,
            objectState.descriptorSets.data()
        )
    };

    if (result != VK_SUCCESS) {
        core::Logger::error("Error freeing object shader descriptor sets!");
    }

    for (uint32_t i { 0u }; i < m_maxObjectCount; i++) {
        for (uint32_t j { 0u }; j < 3u; j++) {
            objectState.descriptorStates.at(i).generations.at(j) = resources::global_invalidTextureGeneration;
            objectState.descriptorStates.at(i).ids.at(j) = resources::global_invalidObjectId;
        }
    }

    // TODO: Add the objectId to the free list.
}

auto MaterialShader::createShaderModule(
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
