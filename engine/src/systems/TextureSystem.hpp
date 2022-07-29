#pragma once

#include "../resources/ITexture.hpp"
#include "../renderer/RendererFrontend.hpp"

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

namespace beige {
namespace systems {

class Texture final {
public:
    static constexpr std::string_view m_defaultName { "default" };

    Texture(std::shared_ptr<renderer::Frontend> rendererFrontend);
    ~Texture();

    auto acquire(const std::string& name, const bool autoRelease) -> std::shared_ptr<resources::ITexture>;
    auto release(const std::string& name) -> void;
    auto getDefaultTexture() -> std::shared_ptr<resources::ITexture>;

private:
    struct Entry {
        std::shared_ptr<resources::ITexture> texture;
        bool autoRelease;
    };

    std::shared_ptr<renderer::Frontend> m_rendererFrontend;

    std::shared_ptr<resources::ITexture> m_defaultTexture;

    // Hashtable for texture lookup.
    std::unordered_map<std::string, Entry> m_textures;
    uint64_t m_textureIds;

    auto loadTexture(
        const std::string& textureName,
        std::shared_ptr<resources::ITexture>& texture
    ) -> bool;
    auto createDefaultTexture() -> std::shared_ptr<resources::ITexture>;
};

} // namespace systems
} // namespace beige
