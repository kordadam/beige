#pragma once

#include "../Defines.hpp"

#include "MathVector3.hpp"
#include "MathVector4.hpp"

#include <array>

namespace beige {
namespace math {

class Quaternion;

class BEIGE_API Matrix4x4 {
public:
    union {
        std::array<Vector4, 4u> rows;
        std::array<float, 16u> data;
    };

    Matrix4x4();
    Matrix4x4(const Vector4& a, const Vector4& b, const Vector4& c, const Vector4& d);
    Matrix4x4(const std::array<float, 16u>& a);
    Matrix4x4(const Quaternion& a);
    Matrix4x4(const Quaternion& q, const Vector3& center);
    ~Matrix4x4() = default;

    auto operator*(const Matrix4x4& a) -> Matrix4x4;

    static auto identity() -> Matrix4x4;
    static auto ortographic(
        const float left,
        const float right,
        const float bottom,
        const float top,
        const float nearClip,
        const float farClip
    ) -> Matrix4x4;
    static auto perspective(
        const float fovRadians,
        const float aspectRatio,
        const float nearClip,
        const float farClip
    ) -> Matrix4x4;
    static auto lookAt(
        const Vector3& position,
        const Vector3& target,
        const Vector3& up
    ) -> Matrix4x4;
    static auto translation(const Vector3& position) -> Matrix4x4;
    static auto scale(const Vector3& scale) -> Matrix4x4;
    static auto xEuler(const float angleInRadians) -> Matrix4x4;
    static auto yEuler(const float angleInRadians) -> Matrix4x4;
    static auto zEuler(const float angleInRadians) -> Matrix4x4;
    static auto xyzEuler(
        const float xRadians,
        const float yRadians,
        const float zRadians
    ) -> Matrix4x4;

    auto inverse() -> void;
    auto transposed() -> Matrix4x4;
    auto forward() -> Vector3;
    auto backward() -> Vector3;
    auto up() -> Vector3;
    auto down() -> Vector3;
    auto left() -> Vector3;
    auto right() -> Vector3;
};

} // namespace math
} // namespace beige
