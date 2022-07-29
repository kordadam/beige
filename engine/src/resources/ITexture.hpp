#pragma once

#include <vector>
#include <string>

namespace beige {
namespace resources {

using TextureGeneration = uint32_t;
constexpr TextureGeneration global_invalidTextureGeneration { static_cast<TextureGeneration>(-1) };

class ITexture {
public:
    ITexture(
        const std::string& name,
        const bool autoRelease,
        const int32_t width,
        const int32_t height,
        const int32_t channelCount,
        const void* pixels,
        const bool hasTransparency
    ) :
    m_width{ width },
    m_height{ height },
    m_channelCount{ channelCount },
    m_hasTransparency { hasTransparency },
    m_generation { global_invalidTextureGeneration } {

    };
    virtual ~ITexture() = default;

    virtual auto getGeneration() const -> const TextureGeneration final { return m_generation; }
    virtual auto setGeneration(const TextureGeneration textureGeneration) -> void final { m_generation = textureGeneration; }

protected:
    int32_t m_width;
    int32_t m_height;
    int32_t m_channelCount;
    bool m_hasTransparency;
    TextureGeneration m_generation;
};


} // namespace resources
} // namespace beige
