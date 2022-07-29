#pragma once

#include <vector>
#include <string>

namespace beige {
namespace resources {

using TextureGeneration = uint32_t;
using ObjectId = uint32_t;

inline constexpr TextureGeneration global_invalidTextureGeneration { static_cast<TextureGeneration>(-1) };
inline constexpr ObjectId global_invalidObjectId { static_cast<ObjectId>(-1) };

class ITexture {
public:
    ITexture(
        const std::string& name,
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
    m_generation { global_invalidTextureGeneration },
    m_id { global_invalidObjectId } {

    };
    virtual ~ITexture() = default;

    virtual auto getGeneration() const -> const TextureGeneration final { return m_generation; }
    virtual auto setGeneration(const TextureGeneration textureGeneration) -> void final { m_generation = textureGeneration; }
    virtual auto getId() const -> const uint32_t final { return m_id; }
    virtual auto setId(const uint32_t id) -> void final { m_id = id; }

protected:
    int32_t m_width;
    int32_t m_height;
    int32_t m_channelCount;
    bool m_hasTransparency;
    TextureGeneration m_generation;
    ObjectId m_id;
};


} // namespace resources
} // namespace beige
