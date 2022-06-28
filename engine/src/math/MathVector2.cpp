#include "MathVector2.hpp"

#include "MathConstans.hpp"

#include <cmath>

namespace beige {
namespace math {

Vector2::Vector2() :
x { 0.0f },
y { 0.0f } {

}

Vector2::Vector2(const float value) :
x { value },
y { value } {

}

Vector2::Vector2(const float xValue, const float yValue) :
x { xValue },
y { yValue } {

}

auto Vector2::operator+(const Vector2& a) -> Vector2 {
    return Vector2(x + a.x, y + a.y);
}

auto Vector2::operator-(const Vector2& a) -> Vector2 {
    return Vector2(x - a.x, y - a.y);
}

auto Vector2::operator*(const Vector2& a) -> Vector2 {
    return Vector2(x * a.x, y * a.y);
}

auto Vector2::operator/(const Vector2& a) -> Vector2 {
    return Vector2(x / a.x, y / a.y);
}

auto Vector2::operator==(const Vector2& a) -> bool {
    if (std::abs(x - a.x) > epsilon || std::abs(y - a.y) > epsilon) {
        return false;
    }

    return true;
}

auto Vector2::zero() -> Vector2 {
    return Vector2();
}

auto Vector2::one() -> Vector2 {
    return Vector2(1.0f);
}

auto Vector2::up() -> Vector2 {
    return Vector2(0.0f, 1.0f);
}

auto Vector2::down() -> Vector2 {
    return Vector2(0.0f, -1.0f);
}

auto Vector2::left() -> Vector2 {
    return Vector2(-1.0f, 0.0f);
}

auto Vector2::right() -> Vector2 {
    return Vector2(1.0f, 0.0f);
}

auto Vector2::distance(const Vector2& a, const Vector2& b) -> float {
    const Vector2 c {
        a.x - b.x,
        a.y - b.y
    };

    return c.length();
}

auto Vector2::lengthSquared() const -> float {
    return x * x + y * y;
}

auto Vector2::length() const -> float {
    return std::sqrt(x * x + y * y);
}

auto Vector2::normalize() -> void {
    const float v { length() };
    x /= v;
    y /= v;
}

auto Vector2::normalized() const -> Vector2 {
    Vector2 a { *this };
    a.normalize();
    return a;
}

} // namespace math
} // namespace beige
