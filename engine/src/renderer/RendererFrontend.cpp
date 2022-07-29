#include "RendererFrontend.hpp"

#include "vulkan/VulkanBackend.hpp"
#include "../core/Logger.hpp"

#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

// TODO: Temporary
#include "../core/Input.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "../external/stb/stb_image.h"
// TODO: End temporary

#include <iostream>

namespace beige {
namespace renderer {

Frontend::Frontend(
    const std::string& appName,
    const uint32_t width,
    const uint32_t height,
    std::shared_ptr<platform::Platform> platform
) :
m_backend {
    std::make_unique<vulkan::Backend>(
        appName,
        width,
        height,
        platform
    )
},
m_frameCount { 0u },
m_nearClip { 0.01f },
m_farClip { 1000.0f },
m_projection {
    glm::perspective(
        glm::radians(45.0f),
        static_cast<float>(width) / static_cast<float>(height),
        m_nearClip,
        m_farClip
    )
},
m_view { glm::mat4x4(1.0f) },
m_defaultTexture { nullptr },
m_testDiffuse { nullptr } {
    m_defaultTexture = m_backend->createDefaultTexture();
    m_testDiffuse = m_backend->createDefaultTexture();

    // Manually set the texture generation to invalind since this is a default texture.
    m_defaultTexture->setGeneration(resources::global_invalidTextureGeneration);
}

Frontend::~Frontend() {
    m_testDiffuse.reset();
    m_defaultTexture.reset();
}

auto Frontend::onResized(const uint16_t width, const uint16_t height) -> void {
    m_projection = glm::perspective(
        glm::radians(45.0f),
        static_cast<float>(width) / static_cast<float>(height),
        m_nearClip,
        m_farClip
    );

    m_backend->onResized(width, height);
}

auto Frontend::drawFrame(const Packet& packet) -> bool {
    // If the begin frame returned successfully, mid-frame operations may continue.
    if (beginFrame(packet.deltaTime)) {
        m_backend->updateGlobalState(
            m_projection,
            m_view,
            glm::vec3(0.0f),
            glm::vec4(1.0f),
            0
        );

        static float angle { 0.01f };
        angle += 0.001f;

        const GeometryRenderData geometryRenderData {
            0u, // TODO: Actual objectId;
            glm::rotate(glm::mat4x4(1.0f), angle, glm::vec3(0.0f, 0.0f, 1.0f)),
            { m_testDiffuse }
        };

        m_backend->updateObject(geometryRenderData);

        // End the frame. if this fails, it is likely unrecoverable.
        const bool result { endFrame(packet.deltaTime) };

        if (!result) {
            core::Logger::error("RendererFrontend::drawFrame - failed to ending frame, application shutting down...");
            return false;
        }
    }

    return true;
}

auto Frontend::setView(const glm::mat4x4& view) -> void {
    m_view = view;
}

auto Frontend::beginFrame(const float deltaTime) -> bool {
    return m_backend->beginFrame(deltaTime);
}

auto Frontend::endFrame(const float deltaTime) -> bool {
    const bool result { m_backend->endFrame(deltaTime) };
    m_frameCount++;
    return result;
}

auto Frontend::loadTexture(
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
            texture->getGeneration()
        };

        texture->setGeneration(resources::global_invalidTextureGeneration);

        const uint64_t totalSize {
            static_cast<uint64_t>(width * height * requiredChannelCount)
        };

        // Check for transparency.
        bool hasTransparency { false };
        for (uint64_t i { 0u }; i < totalSize; i += requiredChannelCount) {
            stbi_uc a { data[i + 3u] };
            if (a < 0xFFu) {
                hasTransparency = true;
                break;
            }
        }

        if (stbi_failure_reason() != nullptr) {
            const std::string message { "Loading texture failed to load file " + filePath + ": " + stbi_failure_reason() + "!" };
            core::Logger::warn(message);
        }

        // Acquire internal texture resources and upload to GPU.
        std::shared_ptr<resources::ITexture> temporaryTexture {
            m_backend->createTexture(
                textureName,
                true,
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
        } else {
            texture->setGeneration(currentGeneration + 1u);
        }

        // Clean up data.
        stbi_image_free(data);

        return true;
    } else {
        if (stbi_failure_reason() != nullptr) {
            const std::string message { "Loading texture failed to load file " + filePath + ": " + stbi_failure_reason() + "!" };
            core::Logger::warn(message);
        }

        return false;
    }
}

} // namespace renderer
} // namespace beige
