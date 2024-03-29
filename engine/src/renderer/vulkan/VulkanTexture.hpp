#pragma once

#include "../../resources/ITexture.hpp"
#include "VulkanImage.hpp"
#include "VulkanDevice.hpp"

namespace beige {
namespace renderer {
namespace vulkan {

class Texture final : public resources::ITexture {
public:
    Texture(
        const std::string& name,
        const int32_t width,
        const int32_t height,
        const int32_t channelCount,
        const void* pixels,
        const bool hasTransparency,
        VkAllocationCallbacks* allocationCallbacks,
        std::shared_ptr<Device> device
    );
    ~Texture();

    auto getImageView() const -> const VkImageView&;
    auto getSampler() const -> const VkSampler&;

private:
    VkAllocationCallbacks* m_allocationCallbacks;
    std::shared_ptr<Device> m_device;

    std::unique_ptr<Image> m_image;
    VkSampler m_sampler;
};

} // namespace vulkan
} // namespace renderer
} // namespace beige
