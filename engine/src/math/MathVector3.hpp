#pragma once

#include "../Defines.hpp"
#include "MathConstans.hpp"

namespace beige {
namespace math {

class Vector4;

class BEIGE_API Vector3 {
public:
    union { float x; float r; float s; float u; };
    union { float y; float g; float t; float v; };
    union { float z; float b; float p; float w; };

    Vector3();
    Vector3(const float value);
    Vector3(const float xValue, const float yValue, const float zValue);
    Vector3(const Vector4& a);
    ~Vector3() = default;

    auto operator+(const Vector3& a) -> Vector3;
    auto operator-(const Vector3& a) -> Vector3;
    auto operator*(const Vector3& a) -> Vector3;
    auto operator*(const float scalar) -> Vector3;
    auto operator/(const Vector3& a) -> Vector3;
    auto operator=(const Vector4& a) -> Vector3;
    auto operator==(const Vector3& a) -> bool;

    static auto zero() -> Vector3;
    static auto one() -> Vector3;
    static auto up() -> Vector3;
    static auto down() -> Vector3;
    static auto left() -> Vector3;
    static auto right() -> Vector3;
    static auto forward() -> Vector3;
    static auto back() -> Vector3;
    static auto distance(const Vector3& a, const Vector3& b) -> float;
    static auto dot(const Vector3& a, const Vector3& b) -> float;
    static auto cross(const Vector3& a, const Vector3& b) -> Vector3;

    auto lengthSquared() const -> float;
    auto length() const -> float;
    auto normalize() -> void;
    auto normalized() const -> Vector3;
};

} // namespace math
} // namespace beige
