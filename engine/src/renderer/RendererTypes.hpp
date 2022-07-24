#pragma once

#include "../resources/ITexture.hpp"

#include <glm/glm.hpp>

#include <array>
#include <memory>


namespace beige {
namespace renderer {

using ObjectId = uint32_t;

inline constexpr ObjectId invalidObjectId { static_cast<ObjectId>(-1) };

struct Packet {
    float deltaTime;
};

struct GlobalUniformObject {
    glm::mat4x4 projection; // 64 bytes
    glm::mat4x4 view;       // 64 bytes
    glm::mat4x4 reserved_0; // 64 bytes, reserved for future use
    glm::mat4x4 reserved_1; // 64 bytes, reserved for future use
};

struct ObjectUniformObject {
    glm::vec4 diffuseColor; // 16 bytes
    glm::vec4 reserved_0;   // 16 bytes, reserved for future use
    glm::vec4 reserved_1;   // 16 bytes, reserved for future use
    glm::vec4 reserved_2;   // 16 bytes, reserved for future use
};

struct GeometryRenderData {
    ObjectId objectId;
    glm::mat4x4 model;
    std::array<std::shared_ptr<resources::ITexture>, 16u> textures;
};

} // namespace renderer
} // namespace beige
