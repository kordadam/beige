#pragma once

#include "../Defines.hpp"

namespace beige {
namespace math {

class Vector3;

class BEIGE_API Quaternion {
public:
    float x;
    float y;
    float z;
    float w;

    Quaternion();
    Quaternion(
        const float x,
        const float y,
        const float z,
        const float w
    );
    Quaternion(
        const Vector3& axis,
        const float angle,
        const bool createNormalized
    );
    ~Quaternion() = default;

    auto operator*(const Quaternion& q) -> Quaternion;

    static auto identity() -> Quaternion;
    static auto dot(const Quaternion& a, const Quaternion& b) -> float;
    static auto slerp(
        const Quaternion& a,
        const Quaternion& b,
        const float percentage
    ) -> Quaternion;
    static auto degToRad(const float degrees) -> float;
    static auto radToDeg(const float radians) -> float;

    auto normal() const -> float;
    auto normalize() -> void;
    auto conjugate() const -> Quaternion;
    auto inverse() const -> Quaternion;
};

} // namespace math
} // namespace beige
