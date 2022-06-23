#include "MathVector3.hpp"

#include "MathVector4.hpp"

#include <cmath>

namespace beige {
namespace math {

Vector3::Vector3() :
x { 0.0f },
y { 0.0f },
z { 0.0f } {

}

Vector3::Vector3(const float value) :
x { value },
y { value },
z { value } {

}

Vector3::Vector3(const float xValue, const float yValue, const float zValue) :
x { xValue },
y { yValue },
z { zValue } {

}

Vector3::Vector3(const Vector4& a) :
x { a.x },
y { a.y },
z { a.z } {

}

auto Vector3::operator+(const Vector3& a) -> Vector3 {
    return Vector3(x + a.x, y + a.y, z + a.z);
}

auto Vector3::operator-(const Vector3& a) -> Vector3 {
    return Vector3(x - a.x, y - a.y, z - a.z);
}

auto Vector3::operator*(const Vector3& a) -> Vector3 {
    return Vector3(x * a.x, y * a.y, z * a.z);
}

auto Vector3::operator*(const float scalar) -> Vector3 {
    return Vector3(x * scalar, y * scalar, z * scalar);
}

auto Vector3::operator/(const Vector3& a) -> Vector3 {
    return Vector3(x / a.x, y / a.y, z / a.z);
}

auto Vector3::operator=(const Vector4& a) -> Vector3 {
    return Vector3(a);
}

auto Vector3::operator==(const Vector3& a) -> bool {
    if (
        std::abs(x - a.x) > epsilon ||
        std::abs(y - a.y) > epsilon ||
        std::abs(z - a.z) > epsilon
    ) {
        return false;
    }

    return true;
}

auto Vector3::zero() -> Vector3 {
    return Vector3();
}

auto Vector3::one() -> Vector3 {
    return Vector3(1.0f);
}

auto Vector3::up() -> Vector3 {
    return Vector3(0.0f, 1.0f, 0.0f);
}

auto Vector3::down() -> Vector3 {
    return Vector3(0.0f, -1.0f, 0.0f);
}

auto Vector3::left() -> Vector3 {
    return Vector3(-1.0f, 0.0f, 0.0f);
}

auto Vector3::right() -> Vector3 {
    return Vector3(1.0f, 0.0f, 0.0f);
}

auto Vector3::forward() -> Vector3 {
    return Vector3(0.0f, 0.0f, -1.0f);
}

auto Vector3::back() -> Vector3 {
    return Vector3(0.0f, 0.0f, 1.0f);
}

auto Vector3::distance(const Vector3& a, const Vector3& b) -> float {
    const Vector3 c {
        a.x - b.x,
        a.y - b.y,
        a.z - b.z
    };

    return c.length();
}

auto Vector3::dot(const Vector3& a, const Vector3& b) -> float {
    const float p {
        a.x * b.x +
        a.y * b.y +
        a.z * b.z
    };

    return p;
}

auto Vector3::cross(const Vector3& a, const Vector3& b) -> Vector3 {
    const Vector3 c(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );

    return c;
}

auto Vector3::lengthSquared() const -> float {
    return x * x + y * y + z * z;
}

auto Vector3::length() const -> float {
    return std::sqrt(x * x + y * y + z * z);
}

auto Vector3::normalize() -> void {
    const float v { length() };
    x /= v;
    y /= v;
    z /= v;
}

auto Vector3::normalized() const -> Vector3 {
    Vector3 a { *this };
    a.normalize();
    return a;
}

} // namespace math
} // namespace beige
