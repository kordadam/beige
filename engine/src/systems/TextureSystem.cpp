#include "TextureSystem.hpp"

#include "../core/Logger.hpp"

// TODO: Resource loader.
#define STB_IMAGE_IMPLEMENTATION
#include "../external/stb/stb_image.h"

namespace beige {
namespace systems {

Texture::Texture(std::shared_ptr<renderer::Frontend> rendererFrontend) :
m_rendererFrontend { rendererFrontend },
m_defaultTexture { nullptr },
m_textures { },
m_textureIds { 0u } {
    m_defaultTexture = createDefaultTexture();
}

Texture::~Texture() {

}

auto Texture::acquire(const std::string& name, const bool autoRelease) -> std::shared_ptr<resources::ITexture> {
    if (name == m_defaultName) {
        core::Logger::warn("beige::systems::Texture::acquire() called for default texture, use getDefaultTexture() for it!");
        return m_defaultTexture;
    }

    std::unordered_map<std::string, Entry>::iterator entry { m_textures.find(name) };

    if (entry != m_textures.end()) {
        return entry->second.texture;
    } else {
        std::shared_ptr<resources::ITexture> newTexture { nullptr };

        // Create new texture.
        if (!loadTexture(name, newTexture)) {
            core::Logger::fatal("Failed to load texture: " + name + "!");
            return nullptr;
        }

        // Update the entry.
        const Entry newEntry {
            newTexture,
            autoRelease
        };

        m_textures.emplace(name, newEntry);
        m_textureIds++;
        newTexture->setId(m_textureIds);
        return newTexture;
    }
}

auto Texture::release(const std::string& name) -> void {
    if (name == m_defaultName) {
        core::Logger::warn("Tried to release default texture!");
        return;
    }

    std::unordered_map<std::string, Entry>::iterator entry { m_textures.find(name) };

    if (entry != m_textures.end()) {
        const long referenceCount { entry->second.texture.use_count() };
        if (referenceCount == 1 && entry->second.autoRelease) {
            m_textures.erase(name);
            core::Logger::trace("Released texture " + name + ", texture unloaded because reference count is 0 and auto release was enabled!");
        } else {
            core::Logger::trace(
                "Released texture " + name + ", now has a reference count of " + std::to_string(referenceCount) + " (auto release: " + std::to_string(entry->second.autoRelease) + ")!"
            );
        }
    } else {
        core::Logger::warn("Tried to release non-existent texture: " + name + "!");
        return;
    }
}

auto Texture::getDefaultTexture() -> std::shared_ptr<resources::ITexture> {
    return m_defaultTexture;
}

auto Texture::loadTexture(
    const std::string& textureName,
    std::shared_ptr<resources::ITexture>& texture
) -> bool {
    // TODO: Should be able to be located anywhere.
    const int32_t requiredChannelCount { 4 };

    stbi_set_flip_vertically_on_load(true);

    // TODO: Try different extensions.
    const std::string filePath { "assets/textures/" + textureName + ".png" };

    int32_t width { 0 };
    int32_t height { 0 };
    int32_t channelCount { 0 };

    stbi_uc* data {
        stbi_load(
            filePath.c_str(),
            &width,
            &height,
            &channelCount,
            requiredChannelCount
        )
    };

    if (data != nullptr) {

        const resources::TextureGeneration currentGeneration {
            texture != nullptr ? texture->getGeneration() : resources::global_invalidTextureGeneration
        };

        const uint64_t totalSize {
            static_cast<uint64_t>(width * height * requiredChannelCount)
        };

        // Check for transparency.
        bool hasTransparency { false };
        for (uint64_t i{ 0u }; i < totalSize; i += requiredChannelCount) {
            stbi_uc a{ data[i + 3u] };
            if (a < 0xFFu) {
                hasTransparency = true;
                break;
            }
        }

        if (stbi_failure_reason() != nullptr) {
            const std::string message{ "Loading texture failed to load file " + filePath + ": " + stbi_failure_reason() + "!" };
            core::Logger::warn(message);
        }

        // Acquire internal texture resources and upload to GPU.
        std::shared_ptr<resources::ITexture> temporaryTexture {
            m_rendererFrontend->createTexture(
                textureName,
                width,
                height,
                requiredChannelCount,
                data,
                hasTransparency
            )
        };

        // Destroy the old texture.
        texture.reset();

        // Assign the temporary texture to the pointer.
        texture = temporaryTexture;

        if (currentGeneration == resources::global_invalidTextureGeneration) {
            texture->setGeneration(0u);
        }
        else {
            texture->setGeneration(currentGeneration + 1u);
        }

        // Clean up data.
        stbi_image_free(data);

        return true;
    }
    else {
        if (stbi_failure_reason() != nullptr) {
            const std::string message { "Loading texture failed to load file " + filePath + ": " + stbi_failure_reason() + "!" };
            core::Logger::warn(message);
        }

        return false;
    }
}

auto Texture::createDefaultTexture() -> std::shared_ptr<resources::ITexture> {
    // NOTE: Create default texture, a 256x256 blue/white checkerboard pattern.
    // This is done in code to eliminate asset dependecies.
    core::Logger::trace("Creating default texture...");
    const uint32_t textureDimension { 256u };
    const uint32_t channels { 4u };
    const uint32_t pixelCount { textureDimension * textureDimension };
    std::vector<std::byte> pixels { pixelCount * channels, std::byte(0xFF) };

    // Each pixel.
    for (uint32_t row { 0u }; row < textureDimension; row++) {
        for (uint32_t col { 0u }; col < textureDimension; col++) {
            const uint32_t index { row * textureDimension + col };
            const uint32_t indexChannel { index * channels };
            if (row % 2u) {
                if (col % 2u) {
                    pixels.at(indexChannel + 0u) = std::byte(0x00);
                    pixels.at(indexChannel + 1u) = std::byte(0x00);
                }
            }
            else {
                if (!(col % 2u)) {
                    pixels.at(indexChannel + 0u) = std::byte(0x00);
                    pixels.at(indexChannel + 1u) = std::byte(0x00);
                }
            }
        }
    }

    return m_rendererFrontend->createTexture(
        m_defaultName.data(),
        static_cast<int32_t>(textureDimension),
        static_cast<int32_t>(textureDimension),
        static_cast<int32_t>(channels),
        pixels.data(),
        false
    );
}

} // namespace systems
} // namespace beige
