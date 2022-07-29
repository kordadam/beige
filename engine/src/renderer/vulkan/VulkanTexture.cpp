#include "VulkanTexture.hpp"

#include "VulkanBuffer.hpp"
#include "VulkanCommandBuffer.hpp"
#include "VulkanUtils.hpp"
#include "../../core/Logger.hpp"

namespace beige {
namespace renderer {
namespace vulkan {

Texture::Texture(
    const std::string& name,
    const int32_t width,
    const int32_t height,
    const int32_t channelCount,
    const void* pixels,
    const bool hasTransparency,
    VkAllocationCallbacks* allocationCallbacks,
    std::shared_ptr<Device> device
) :
ITexture {
    name,
    width,
    height,
    channelCount,
    pixels,
    hasTransparency
},
m_image { nullptr },
m_sampler { VK_NULL_HANDLE },
m_allocationCallbacks { allocationCallbacks },
m_device { device } {
    const VkDeviceSize imageSize { static_cast<VkDeviceSize>(m_width * m_height * m_channelCount) };

    // NOTE: Assumes 8 bits per channel.
    const VkFormat imageFormat { VK_FORMAT_R8G8B8A8_UNORM };

    // Create a staging buffer and load data into it.
    const VkBufferUsageFlags bufferUsageFlags {
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT
    };

    const VkMemoryPropertyFlags memoryPropertyFlags {
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    };

    Buffer staging {
        m_allocationCallbacks,
        m_device,
        imageSize,
        bufferUsageFlags,
        memoryPropertyFlags,
        true
    };

    staging.loadData(0u, imageSize, 0u, pixels);

    // NOTE: Lots of assumptions here, different texture types will require different options here.
    const VkImageUsageFlags imageUsageFlags {
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
        VK_IMAGE_USAGE_TRANSFER_DST_BIT |
        VK_IMAGE_USAGE_SAMPLED_BIT |
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
    };

    m_image = std::make_unique<Image>(
        m_allocationCallbacks,
        m_device,
        VK_IMAGE_TYPE_2D,
        m_width,
        m_height,
        imageFormat,
        VK_IMAGE_TILING_OPTIMAL,
        imageUsageFlags,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        true,
        VK_IMAGE_ASPECT_COLOR_BIT
    );

    const VkCommandPool commandPool { m_device->getGraphicsCommandPool() };
    const VkQueue queue { m_device->getGraphicsQueue() };

    CommandBuffer temporaryCommandBuffer { m_device };
    temporaryCommandBuffer.allocateAndBeginSingleUse(commandPool);

    // Transition the layout from whatever it is currently to optimal for receiving data.
    m_image->transitionLayout(
        temporaryCommandBuffer.getHandle(),
        imageFormat,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    );

    // Copy the data from the buffer.
    m_image->copyFromBuffer(staging.getHandle(), temporaryCommandBuffer.getHandle());

    // Transition from optimal for data reciept to shader-read-only optimal layout.
    m_image->transitionLayout(
        temporaryCommandBuffer.getHandle(),
        imageFormat,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    );

    temporaryCommandBuffer.endSingleUse(commandPool, queue);

    // Create a sampler for the texture.
    // TODO: Filters should be configurable.
    const VkSamplerCreateInfo samplerCreateInfo {
        VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO, // sType
        nullptr,                               // pNext
        0u,                                    // flags
        VK_FILTER_LINEAR,                      // magFilter
        VK_FILTER_LINEAR,                      // minFilter
        VK_SAMPLER_MIPMAP_MODE_LINEAR,         // mipmapMode
        VK_SAMPLER_ADDRESS_MODE_REPEAT,        // addressModeU
        VK_SAMPLER_ADDRESS_MODE_REPEAT,        // addressModeV
        VK_SAMPLER_ADDRESS_MODE_REPEAT,        // addressModeW
        0.0f,                                  // mipLodBias
        VK_TRUE,                               // anisotropyEnable
        16.0f,                                 // maxAnisotropy
        VK_FALSE,                              // compareEnable
        VK_COMPARE_OP_ALWAYS,                  // compareOp
        0.0f,                                  // minLod
        0.0f,                                  // maxLod
        VK_BORDER_COLOR_INT_OPAQUE_BLACK,      // borderColor
        VK_FALSE                               // unnormalizedCoordinates
    };

    VkResult result {
        vkCreateSampler(
            m_device->getLogicalDevice(),
            &samplerCreateInfo,
            m_allocationCallbacks,
            &m_sampler
        )
    };

    if (!Utils::isResultSuccess(result)) {
        const std::string message { "Error creating texture sampler: " + Utils::resultToString(result, true) + "!" };
        throw std::exception(message.c_str());
    }

    m_generation++;
}

Texture::~Texture() {
    vkDeviceWaitIdle(m_device->getLogicalDevice());

    m_image.reset();

    vkDestroySampler(
        m_device->getLogicalDevice(),
        m_sampler,
        m_allocationCallbacks
    );
}

auto Texture::getImageView() const -> const VkImageView& {
    return m_image->getImageView();
}

auto Texture::getSampler() const -> const VkSampler& {
    return m_sampler;
}

} // namespace vulkan
} // namespace renderer
} // namespace beige
