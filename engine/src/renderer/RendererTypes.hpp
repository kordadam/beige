#pragma once

#include "../math/Math.hpp"

namespace beige {
namespace renderer {

struct Packet {
    float deltaTime;
};

struct GlobalUniformObject {
    math::Matrix4x4 projection; // 64 bytes
    math::Matrix4x4 view;       // 64 bytes
    math::Matrix4x4 reserved_0; // 64 bytes, reserved for future use
    math::Matrix4x4 reserved_1; // 64 bytes, reserved for future use
};

} // namespace renderer
} // namespace beige
