#pragma once

#include <vector>
#include <string>

namespace beige {
namespace resources {

class ITexture {
public:
    using Generation = uint32_t;

    ITexture(
        const std::string& name,
        const bool autoRelease,
        const int32_t width,
        const int32_t height,
        const int32_t channelCount,
        const std::vector<std::byte>& pixels,
        const bool hasTransparency
    ) :
    m_width{ width },
    m_height{ height },
    m_channelCount{ channelCount },
    m_hasTransparency { hasTransparency },
    m_generation { m_invalidGeneration } {

    };
    virtual ~ITexture() = default;

    virtual auto getGeneration() const -> const Generation final { return m_generation; }

protected:
    static constexpr Generation m_invalidGeneration { static_cast<Generation>(-1) };

    int32_t m_width;
    int32_t m_height;
    int32_t m_channelCount;
    bool m_hasTransparency;
    Generation m_generation;
};


} // namespace resources
} // namespace beige
