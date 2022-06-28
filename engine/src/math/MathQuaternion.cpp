#include "MathQuaternion.hpp"

#include "MathConstans.hpp"
#include "MathVector3.hpp"

#include <cmath>

namespace beige {
namespace math {

Quaternion::Quaternion() :
x { 0.0f },
y { 0.0f },
z { 0.0f },
w { 0.0f } {

}

Quaternion::Quaternion(
    const float x,
    const float y,
    const float z,
    const float w
) :
x ( x ),
y { y },
z { z },
w { w } {

}

Quaternion::Quaternion(
    const Vector3& axis,
    const float angle,
    const bool createNormalized
) {
    const float halfAngle { 0.5f * angle };
    const float s { std::sin(halfAngle) };
    const float c { std::cos(halfAngle) };

    x = s * axis.x;
    y = s * axis.y;
    z = s * axis.z;
    w = c;

    if (createNormalized) {
        normalize();
    }
}

auto Quaternion::operator*(const Quaternion& q) -> Quaternion {
    return Quaternion(
        x * q.w +
        y * q.z -
        z * q.y +
        w * q.x,

        -x * q.z +
        y * q.w +
        z * q.x +
        w * q.y,

        x * q.y -
        y * q.x +
        z * q.w +
        w * q.z,

        -x * q.x -
        y * q.y -
        z * q.z +
        w * q.w
    );
}

auto Quaternion::identity() -> Quaternion {
    return Quaternion(0.0f, 0.0f, 0.0f, 1.0f);
}

auto Quaternion::dot(const Quaternion& a, const Quaternion& b) -> float {
    const float p {
        a.x * b.x +
        a.y * b.y +
        a.z * b.z +
        a.w * b.w
    };

    return p;
}

auto Quaternion::slerp(
    const Quaternion& a,
    const Quaternion& b,
    const float percentage
) -> Quaternion {
    Quaternion na { a };
    na.normalize();
    Quaternion nb { b };
    nb.normalize();

    float d { dot(na, nb) };

    if (d < 0.0f) {
        nb.x = -nb.x;
        nb.y = -nb.x;
        nb.z = -nb.x;
        nb.w = -nb.x;

        d = -d;
    }

    const float dotTreshold { 0.9995f };
    if (d > dotTreshold) {
        return Quaternion(
            na.x + ((nb.x - na.x) * percentage),
            na.y + ((nb.y - na.y) * percentage),
            na.z + ((nb.z - na.z) * percentage),
            na.w + ((nb.w - na.w) * percentage)
        );
    }

    const float theta0 { std::cos(d) };
    const float theta { theta0 * percentage };
    const float sinTheta { std::sin(theta) };
    const float sinTheta0 { std::sin(theta0) };

    const float s0 { std::cos(theta) - d * sinTheta / sinTheta0 };
    const float s1 { sinTheta / sinTheta0 };

    return Quaternion(
        (na.x * s0) + (nb.x * s1),
        (na.y * s0) + (nb.y * s1),
        (na.z * s0) + (nb.z * s1),
        (na.w * s0) + (nb.w * s1)
    );
}

auto Quaternion::degToRad(const float degrees) -> float {
    return degrees * degToRadMultiplier;
}

auto Quaternion::radToDeg(const float radians) -> float {
    return radians * radToDegMultiplier;
}

auto Quaternion::normal() const -> float {
    return std::sqrt(
        x * x +
        y * y +
        z * z +
        w * w
    );
}

auto Quaternion::normalize() -> void {
    const float n { normal() };

    x /= n;
    y /= n;
    z /= n;
    w /= n;
}

auto Quaternion::conjugate() const -> Quaternion {
    return Quaternion(-x, -y, -z, -w);
}

auto Quaternion::inverse() const -> Quaternion {
    Quaternion q { *this };
    q = q.conjugate();
    q.normalize();

    return q;
}

} // namespace math
} // namespace beige
