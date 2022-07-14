#include "VulkanPipeline.hpp"

#include "VulkanDefines.hpp"
#include "VulkanUtils.hpp"
#include "../../math/Math.hpp"

#include <array>

namespace beige {
namespace renderer {
namespace vulkan {

Pipeline::Pipeline(
    VkAllocationCallbacks* allocationCallbacks,
    std::shared_ptr<Device> device,
    std::shared_ptr<RenderPass> renderPass,
    const std::vector<VkVertexInputAttributeDescription>& vertexInputAttributeDescriptions,
    const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts,
    const std::vector<VkPipelineShaderStageCreateInfo>& pipelineShaderStageCreateInfos,
    const VkViewport& viewport,
    const VkRect2D& scissor,
    const bool isWireframe
) :
m_allocationCallbacks { allocationCallbacks },
m_device { device },
m_renderPass { renderPass },
m_pipelineLayout { VK_NULL_HANDLE },
m_handle { VK_NULL_HANDLE } {
    const VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo {
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO, // sType
        nullptr,                                               // pNext
        0u,                                                    // flags
        1u,                                                    // viewportCount
        &viewport,                                             // pViewports
        1u,                                                    // scissorCount
        &scissor                                               // pScissors
    };

    const VkPolygonMode polygonMode {
        isWireframe ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL
    };

    const VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo {
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO, // sType
        nullptr,                                                    // pNext
        0u,                                                         // flags
        VK_FALSE,                                                   // depthClampEnable
        VK_FALSE,                                                   // rasterizerDiscardEnable
        polygonMode,                                                // polygonMode
        VK_CULL_MODE_BACK_BIT,                                      // cullMode
        VK_FRONT_FACE_COUNTER_CLOCKWISE,                            // frontFace
        VK_FALSE,                                                   // depthBiasEnable
        0.0f,                                                       // depthBiasConstantFactor
        0.0f,                                                       // depthBiasClamp
        0.0f,                                                       // depthBiasSlopeFactor
        1.0f                                                        // lineWidth
    };

    const VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo {
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO, // sType
        nullptr,                                                  // pNext
        0u,                                                       // flags
        VK_SAMPLE_COUNT_1_BIT,                                    // rasterizationSamples
        VK_FALSE,                                                 // sampleShadingEnable
        1.0f,                                                     // minSampleShading
        nullptr,                                                  // pSampleMask
        VK_FALSE,                                                 // alphaToCoverageEnable
        VK_FALSE                                                  // alphaToOneEnable
    };

    const VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo {
        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO, // sType
        nullptr,                                                    // pNext
        0u,                                                         // flags
        VK_TRUE,                                                    // depthTestEnable
        VK_TRUE,                                                    // depthWriteEnable
        VK_COMPARE_OP_LESS,                                         // depthCompareOp
        VK_FALSE,                                                   // depthBoundsTestEnable
        VK_FALSE,                                                   // stencilTestEnable
        { },                                                        // front
        { },                                                        // back
        0.0f,                                                       // minDepthBounds
        0.0f                                                        // maxDepthBounds
    };

    const VkColorComponentFlags colorComponentFlags {
        VK_COLOR_COMPONENT_R_BIT |
        VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT |
        VK_COLOR_COMPONENT_A_BIT
    };

    const VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState {
        VK_TRUE,                             // blendEnable
        VK_BLEND_FACTOR_SRC_ALPHA,           // srcColorBlendFactor
        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, // dstColorBlendFactor
        VK_BLEND_OP_ADD,                     // colorBlendOp
        VK_BLEND_FACTOR_SRC_ALPHA,           // srcAlphaBlendFactor
        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, // dstAlphaBlendFactor
        VK_BLEND_OP_ADD,                     // alphaBlendOp
        colorComponentFlags                  // colorWriteMask
    };

    const VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo {
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO, // sType
        nullptr,                                                  // pNext
        0u,                                                       // flags
        VK_FALSE,                                                 // logicOpEnable
        VK_LOGIC_OP_COPY,                                         // logicOp
        1u,                                                       // attachmentCount
        &pipelineColorBlendAttachmentState,                       // pAttachments
        // blendConstants[4]
    };

    const uint32_t dynamicStateCount { 3u };
    const std::array<VkDynamicState, dynamicStateCount> dynamicStates {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
        VK_DYNAMIC_STATE_LINE_WIDTH
    };

    const VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo {
        VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO, // sType
        nullptr,                                              // pNext
        0u,                                                   // flags
        dynamicStateCount,                                    // dynamicStateCount
        dynamicStates.data()                                  // pDynamicStates
    };

    const VkVertexInputBindingDescription vertexInputBindingDescription {
        0u,                         // binding
        sizeof(math::Vertex3D),     // stride
        VK_VERTEX_INPUT_RATE_VERTEX // inputRate
    };

    const VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo {
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,      // sType
        nullptr,                                                        // pNext
        0u,                                                             // flags
        1u,                                                             // vertexBindingDescriptionCount
        &vertexInputBindingDescription,                                 // pVertexBindingDescriptions
        static_cast<uint32_t>(vertexInputAttributeDescriptions.size()), // vertexAttributeDescriptionCount
        vertexInputAttributeDescriptions.data()                         // pVertexAttributeDescriptions
    };

    const VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo {
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, // sType
        nullptr,                                                     // pNext
        0u,                                                          // flags
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,                         // topology
        VK_FALSE                                                     // primitiveRestartEnable
    };

    const VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo {
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,      // sType
        nullptr,                                            // pNext
        0u,                                                 // flags
        static_cast<uint32_t>(descriptorSetLayouts.size()), // setLayoutCount
        descriptorSetLayouts.data(),                        // pSetLayouts
        0u,                                                 // pushConstantRangeCount
        nullptr                                             // pPushConstantRanges
    };

    VULKAN_CHECK(
        vkCreatePipelineLayout(
            m_device->getLogicalDevice(),
            &pipelineLayoutCreateInfo,
            m_allocationCallbacks,
            &m_pipelineLayout
        )
    );

    const VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo {
        VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,              // sType
        nullptr,                                                      // pNext
        0u,                                                           // flags
        static_cast<uint32_t>(pipelineShaderStageCreateInfos.size()), // stageCount
        pipelineShaderStageCreateInfos.data(),                        // pStages
        &pipelineVertexInputStateCreateInfo,                          // pVertexInputState
        &pipelineInputAssemblyStateCreateInfo,                        // pInputAssemblyState
        nullptr,                                                      // pTessellationState
        &pipelineViewportStateCreateInfo,                             // pViewportState
        &pipelineRasterizationStateCreateInfo,                        // pRasterizationState
        &pipelineMultisampleStateCreateInfo,                          // pMultisampleState
        &pipelineDepthStencilStateCreateInfo,                         // pDepthStencilState
        &pipelineColorBlendStateCreateInfo,                           // pColorBlendState
        &pipelineDynamicStateCreateInfo,                              // pDynamicState
        m_pipelineLayout,                                             // layout
        m_renderPass->getRenderPass(),                                // renderPass
        0u,                                                           // subpass
        VK_NULL_HANDLE,                                               // basePipelineHandle
        -1                                                            // basePipelineIndex
    };

    const VkResult result {
        vkCreateGraphicsPipelines(
            m_device->getLogicalDevice(),
            VK_NULL_HANDLE,
            1u,
            &graphicsPipelineCreateInfo,
            m_allocationCallbacks,
            &m_handle
        )
    };

    if (Utils::isResultSuccess(result)) {
        core::Logger::info("Graphics pipeline created!");
    } else {
        const std::string message { "vkCreateGraphicsPipelines failed with " + Utils::resultToString(result, true) + "!" };
        throw std::exception(message.c_str());
    }
}

Pipeline::~Pipeline() {
    const VkDevice logicalDevice { m_device->getLogicalDevice() };

    vkDestroyPipeline(
        logicalDevice,
        m_handle,
        m_allocationCallbacks
    );

    vkDestroyPipelineLayout(
        logicalDevice,
        m_pipelineLayout,
        m_allocationCallbacks
    );
}

auto Pipeline::getPipelineLayout() const -> const VkPipelineLayout& {
    return m_pipelineLayout;
}

auto Pipeline::bind(
    const VkCommandBuffer& commandBuffer,
    const VkPipelineBindPoint& pipelineBindPoint
) -> void {
    vkCmdBindPipeline(commandBuffer, pipelineBindPoint, m_handle);
}

} // namespace vulkan
} // namespace renderer
} // namespace beige
