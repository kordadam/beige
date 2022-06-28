#include "MathVector4.hpp"

#include "MathVector3.hpp"

#include <cmath>

namespace beige {
namespace math {

Vector4::Vector4() :
x { 0.0f },
y { 0.0f },
z { 0.0f },
w { 0.0f } {

}

Vector4::Vector4(const float value) :
x { value },
y { value },
z { value },
w { value } {

}

Vector4::Vector4(const float x, const float y, const float z, const float w) :
x { x },
y { y },
z { z },
w { w } {

}

Vector4::Vector4(const Vector3& a) :
x { a.x },
y { a.y },
z { a.z },
w { 0.0f } {

}

auto Vector4::operator+(const Vector4& a) -> Vector4 {
    return Vector4(x + a.x, y + a.y, z + a.z, w + a.w);
}

auto Vector4::operator-(const Vector4& a) -> Vector4 {
    return Vector4(x - a.x, y - a.y, z - a.z, w - a.w);
}

auto Vector4::operator*(const Vector4& a) -> Vector4 {
    return Vector4(x * a.x, y * a.y, z * a.z, w * a.w);
}

auto Vector4::operator/(const Vector4& a) -> Vector4 {
    return Vector4(x / a.x, y / a.y, z / a.z, w / a.w);
}

auto Vector4::operator=(const Vector3& a) -> Vector4 {
    return Vector3(a);
}

auto Vector4::operator==(const Vector4& a) -> bool {
    if (
        std::abs(x - a.x) > epsilon ||
        std::abs(y - a.y) > epsilon ||
        std::abs(z - a.z) > epsilon ||
        std::abs(w - a.w) > epsilon
    ) {
        return false;
    }

    return true;
}

auto Vector4::zero() -> Vector4 {
    return Vector4(0.0f);
}

auto Vector4::one() -> Vector4 {
    return Vector4(1.0f);
}

auto Vector4::dot(const Vector4& a, const Vector4& b) -> float {
    const float p {
        a.x * b.x +
        a.y * b.y +
        a.z * b.z +
        a.w * b.w    
    };

    return p;
}

auto Vector4::lengthSquared() const -> float {
    return x * x + y * y + z * z + w * w;
}

auto Vector4::length() const -> float {
    return std::sqrt(x * x + y * y + z * z + w * w);
}

auto Vector4::normalize() -> void {
    const float v { length() };
    x /= v;
    y /= v;
    z /= v;
    w /= v;
}

auto Vector4::normalized() const -> Vector4 {
    Vector4 a { *this };
    a.normalize();
    return a;
}

} // namespace math
} // namespace beige
