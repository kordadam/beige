#include "VulkanFramebuffer.hpp"

#include "VulkanDefines.hpp"

namespace beige {
namespace renderer {
namespace vulkan {

Framebuffer::Framebuffer(
    VkAllocationCallbacks* allocationCallbacks,
    std::shared_ptr<Device> device,
    const std::shared_ptr<RenderPass> renderPass,
    const uint32_t width,
    const uint32_t height,
    const std::vector<VkImageView>& imageViews
) :
m_allocationCallbacks { allocationCallbacks },
m_device { device },
m_renderPass { renderPass },
m_framebuffer { VK_NULL_HANDLE },
m_imageViews { imageViews }
{
    const VkFramebufferCreateInfo framebufferCreateInfo {
        VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,  // sType
        nullptr,                                    // pNext
        0u,                                         // flags
        m_renderPass->getRenderPass(),              // renderPass
        static_cast<uint32_t>(m_imageViews.size()), // attachmentCount
        m_imageViews.data(),                        // pAttachments
        width,                                      // width
        height,                                     // height
        1u                                          // layers
    };

    VULKAN_CHECK(
        vkCreateFramebuffer(
            m_device->getLogicalDevice(),
            &framebufferCreateInfo,
            m_allocationCallbacks,
            &m_framebuffer
        )
    );
}

Framebuffer::~Framebuffer() {
    vkDestroyFramebuffer(
        m_device->getLogicalDevice(),
        m_framebuffer,
        m_allocationCallbacks
    );
}

auto Framebuffer::getFramebuffer() const -> const VkFramebuffer& {
    return m_framebuffer;
}

} // namespace vulkan
} // namespace renderer
} // namespace beige
