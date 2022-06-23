#pragma once

#include "../Defines.hpp"
#include "MathConstans.hpp"

namespace beige {
namespace math {

class Vector3;

class BEIGE_API Vector4 {
public:
    union { float x; float r; float s; };
    union { float y; float g; float t; };
    union { float z; float b; float p; };
    union { float w; float a; float q; };

    Vector4();
    Vector4(const float value);
    Vector4(const float xValue, const float yValue, const float zValue, const float wValue);
    Vector4(const Vector3& a);
    ~Vector4() = default;

    auto operator+(const Vector4& a) -> Vector4;
    auto operator-(const Vector4& a) -> Vector4;
    auto operator*(const Vector4& a) -> Vector4;
    auto operator/(const Vector4& a) -> Vector4;
    auto operator=(const Vector3& a) -> Vector4;
    auto operator==(const Vector4& a) -> bool;

    static auto zero() -> Vector4;
    static auto one() -> Vector4;
    static auto dot(const Vector4& a, const Vector4& b) -> float;

    auto lengthSquared() const -> float;
    auto length() const -> float;
    auto normalize() -> void;
    auto normalized() const -> Vector4;
};

} // namespace math
} // namespace beige
