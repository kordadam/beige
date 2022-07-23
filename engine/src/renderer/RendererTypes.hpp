#pragma once

#include <glm/glm.hpp>

namespace beige {
namespace renderer {

struct Packet {
    float deltaTime;
};

struct GlobalUniformObject {
    glm::mat4x4 projection; // 64 bytes
    glm::mat4x4 view;       // 64 bytes
    glm::mat4x4 reserved_0; // 64 bytes, reserved for future use
    glm::mat4x4 reserved_1; // 64 bytes, reserved for future use
};

} // namespace renderer
} // namespace beige
