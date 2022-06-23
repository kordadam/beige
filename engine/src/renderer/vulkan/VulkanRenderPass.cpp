#include "VulkanRenderPass.hpp"

#include "VulkanDefines.hpp"

#include <array>

namespace beige {
namespace renderer {
namespace vulkan {

RenderPass::RenderPass(
    VkAllocationCallbacks* allocationCallbacks,
    std::shared_ptr<Swapchain> swapchain,
    std::shared_ptr<Device> device,
    const float x,
    const float y,
    const float w,
    const float h,
    const float r,
    const float g,
    const float b,
    const float a,
    const float depth,
    const uint32_t stencil
) :
m_allocationCallbacks { allocationCallbacks },
m_swapchain { swapchain },
m_device { device },
m_renderPass { 0 },
m_x { x },
m_y { y },
m_w { w },
m_h { h },
m_r { r },
m_g { g },
m_b { b },
m_a { a },
m_depth { depth },
m_stencil { stencil },
m_state { State::Ready } {
    const VkSurfaceFormatKHR surfaceFormat { m_swapchain->getSurfaceFormat() };
    const VkFormat depthFormat { m_device->getDepthFormat() };
    const VkDevice logicalDevice { m_device->getLogicalDevice() };

    const VkAttachmentDescription colorAttachmentDescription {
        0u,                               // flags
        surfaceFormat.format,             // format // TODO: Configurable
        VK_SAMPLE_COUNT_1_BIT,            // samples
        VK_ATTACHMENT_LOAD_OP_CLEAR,      // loadOp
        VK_ATTACHMENT_STORE_OP_STORE,     // storeOp
        VK_ATTACHMENT_LOAD_OP_DONT_CARE,  // stencilLoadOp
        VK_ATTACHMENT_STORE_OP_DONT_CARE, // stencilStoreOp
        VK_IMAGE_LAYOUT_UNDEFINED,        // initialLayout - do not expect any particular layout before render pass
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR   // finalLayout - transitioned to after the render pass
    };

    const VkAttachmentDescription depthAttachmentDescription {
        0u,                                              // flags
        depthFormat,                                     // format
        VK_SAMPLE_COUNT_1_BIT,                           // samples
        VK_ATTACHMENT_LOAD_OP_CLEAR,                     // loadOp
        VK_ATTACHMENT_STORE_OP_DONT_CARE,                // storeOp
        VK_ATTACHMENT_LOAD_OP_DONT_CARE,                 // stencilLoadOp
        VK_ATTACHMENT_STORE_OP_DONT_CARE,                // stencilStoreOp
        VK_IMAGE_LAYOUT_UNDEFINED,                       // initialLayout
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL // finalLayout
    };

    const std::array<VkAttachmentDescription, 2u> attachmentDescriptions {
        colorAttachmentDescription,
        depthAttachmentDescription
    };

    const VkAttachmentReference colorAttachmentReference {
        0u,                                      // attachment - attachment description array index
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL // layout
    };

    const VkAttachmentReference depthAttachmentReference {
        1u,                                              // attachment
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL // layout
    };

    // TODO: Other attachment types (input, resolve, preserve)

    const VkSubpassDescription subpassDescription {
        0u,                              // flags
        VK_PIPELINE_BIND_POINT_GRAPHICS, // pipelineBindPoint
        0u,                              // inputAttachmentCount - input from a shader
        nullptr,                         // pInputAttachments - input from a shader
        1u,                              // colorAttachmentCount
        &colorAttachmentReference,       // pColorAttachments
        nullptr,                         // pResolveAttachments - attachments used for multisampling color attachments
        &depthAttachmentReference,       // pDepthStencilAttachment
        0u,                              // preserveAttachmentCount - attachments not used in this subpass, but must be preserved for the next
        nullptr                          // pPreserveAttachments - attachments not used in this subpass, but must be preserved for the next
    };

    const VkSubpassDependency subpassDependency {
        VK_SUBPASS_EXTERNAL,                                                        // srcSubpass
        0u,                                                                         // dstSubpass
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,                              // srcStageMask
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,                              // dstStageMask
        0u,                                                                         // srcAccessMask
        VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, // dstAccessMask
        0u                                                                          // dependencyFlags
    };

    const VkRenderPassCreateInfo renderPassCreateInfo {
        VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,            // sType
        nullptr,                                              // pNext
        0u,                                                   // flags
        static_cast<uint32_t>(attachmentDescriptions.size()), // attachmentCount
        attachmentDescriptions.data(),                        // pAttachments
        1u,                                                   // subpassCount
        &subpassDescription,                                  // pSubpasses
        1u,                                                   // dependencyCount
        &subpassDependency                                    // pDependencies
    };

    VULKAN_CHECK(
        vkCreateRenderPass(
            logicalDevice,
            &renderPassCreateInfo,
            m_allocationCallbacks,
            &m_renderPass
        )
    );
}

RenderPass::~RenderPass() {
    const VkDevice logicalDevice { m_device->getLogicalDevice() };

    vkDestroyRenderPass(
        logicalDevice,
        m_renderPass,
        m_allocationCallbacks
    );
}

auto RenderPass::getRenderPass() const -> const VkRenderPass& {
    return m_renderPass;
}

auto RenderPass::setX(const float x) -> void {
    m_x = x;
}

auto RenderPass::setY(const float y) -> void {
    m_y = y;
}

auto RenderPass::setW(const float w) -> void {
    m_w = w;
}

auto RenderPass::setH(const float h) -> void {
    m_h = h;
}

auto RenderPass::begin(
    std::shared_ptr<CommandBuffer> commandBuffer,
    const VkFramebuffer& framebuffer
) -> void {
    const VkOffset2D renderAreaOffset {
        static_cast<int32_t>(m_x),
        static_cast<int32_t>(m_y)
    };

    const VkExtent2D renderAreaExtent {
        static_cast<uint32_t>(m_w),
        static_cast<uint32_t>(m_h)
    };

    const VkRect2D renderArea {
        renderAreaOffset,
        renderAreaExtent
    };

    std::array<VkClearValue, 2u> clearValues;
    clearValues.at(0).color.float32[0] = m_r;
    clearValues.at(0).color.float32[1] = m_g;
    clearValues.at(0).color.float32[2] = m_b;
    clearValues.at(0).color.float32[3] = m_a;
    clearValues.at(1).depthStencil.depth = m_depth;
    clearValues.at(1).depthStencil.stencil = m_stencil;

    const VkRenderPassBeginInfo renderPassBeginInfo {
        VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,  // sType
        nullptr,                                   // pNext
        m_renderPass,                              // renderPass
        framebuffer,                               // framebuffer
        renderArea,                                // renderArea
        static_cast<uint32_t>(clearValues.size()), // clearValueCount
        clearValues.data()                         // pClearValues
    };

    vkCmdBeginRenderPass(
        commandBuffer->getCommandBuffer(),
        &renderPassBeginInfo,
        VK_SUBPASS_CONTENTS_INLINE
    );

    commandBuffer->setState(CommandBuffer::State::InRenderPass);
}

auto RenderPass::end(std::shared_ptr<CommandBuffer> commandBuffer) -> void {
    vkCmdEndRenderPass(commandBuffer->getCommandBuffer());
    commandBuffer->setState(CommandBuffer::State::Recording);
}

} // namespace vulkan
} // namespace renderer
} // namespace beige
