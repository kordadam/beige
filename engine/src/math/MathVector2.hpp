#pragma once

#include "../Defines.hpp"

namespace beige {
namespace math {

class BEIGE_API Vector2 {
public:
    union { float x; float r; float s; float u; };
    union { float y; float g; float t; float v; };

    Vector2();
    Vector2(const float value);
    Vector2(const float xValue, const float yValue);
    ~Vector2() = default;

    auto operator+(const Vector2& a) -> Vector2;
    auto operator-(const Vector2& a) -> Vector2;
    auto operator*(const Vector2& a) -> Vector2;
    auto operator/(const Vector2& a) -> Vector2;
    auto operator==(const Vector2& a) -> bool;

    static auto zero() -> Vector2;
    static auto one() -> Vector2;
    static auto up() -> Vector2;
    static auto down() -> Vector2;
    static auto left() -> Vector2;
    static auto right() -> Vector2;
    static auto distance(const Vector2& a, const Vector2& b) -> float;

    auto lengthSquared() const -> float;
    auto length() const -> float;
    auto normalize() -> void;
    auto normalized() const -> Vector2;
};

} // namespace math
} // namespace beige
