#include "MathMatrix4x4.hpp"

#include "MathConstans.hpp"
#include "MathQuaternion.hpp"

#include <cmath>

namespace beige {
namespace math {

Matrix4x4::Matrix4x4() :
data { 0.0f } {

}

Matrix4x4::Matrix4x4(const Vector4& a, const Vector4& b, const Vector4& c, const Vector4& d) :
rows { a, b, c, d } {

}

Matrix4x4::Matrix4x4(const std::array<float, 16u>& a) :
data { a } {

}

Matrix4x4::Matrix4x4(const Quaternion& a) {
    Quaternion n { a };
    n.normalize();

    *this = identity();
    data.at(0u) = 1.0f - 2.0f * n.y * n.y - 2.0f * n.z * n.z;
    data.at(1u) = 2.0f * n.x * n.y - 2.0f * n.z * n.w;
    data.at(2u) = 2.0f * n.x * n.z + 2.0f * n.y * n.w;

    data.at(4u) = 2.0f * n.x * n.y + 2.0f * n.z * n.w;
    data.at(5u) = 1.0f - 2.0f * n.x * n.x - 2.0f * n.z * n.w;
    data.at(6u) = 2.0f * n.y * n.z - 2.0f * n.x * n.w;

    data.at(8u) = 2.0f * n.x * n.z - 2.0f * n.y * n.w;
    data.at(9u) = 2.0f * n.y * n.z + 2.0f * n.x * n.w;
    data.at(10u) = 1.0f - 2.0f * n.x * n.x - 2.0f * n.y * n.y;
}

Matrix4x4::Matrix4x4(const Quaternion& q, const Vector3& center) {
    data.at(0u) = (q.x * q.x) - (q.y * q.y) - (q.z * q.z) + (q.w * q.w);
    data.at(1u) = 2.0f * ((q.x * q.y) + (q.z * q.w));
    data.at(2u) = 2.0f * ((q.x * q.z) - (q.y * q.w));
    data.at(3u) = center.x - center.x * data.at(0u) - center.y * data.at(1u) - center.z * data.at(2u);

    data.at(4u) = 2.0f * ((q.x * q.y) - (q.z * q.w));
    data.at(5u) = -(q.x * q.x) + (q.y * q.y) - (q.z * q.z) + (q.w * q.w);
    data.at(6u) = 2.0f * ((q.y * q.z) + (q.x * q.w));
    data.at(7u) = center.y - center.x * data.at(4u) - center.y * data.at(5u) - center.z * data.at(6u);

    data.at(8u) = 2.0f * (q.x * q.z) + (q.y * q.w);
    data.at(9u) = 2.0f * (q.y * q.z) - (q.x * q.w);
    data.at(10u) = -(q.x * q.x) - (q.y * q.y) + (q.z * q.z) + (q.w * q.w);
    data.at(11u) = center.z - center.x * data.at(8u) - center.y * data.at(9u) - center.z * data.at(10u);

    data.at(12u) = 0.0f;
    data.at(13u) = 0.0f;
    data.at(14u) = 0.0f;
    data.at(15u) = 0.0f;
}

auto Matrix4x4::operator*(const Matrix4x4& a) -> Matrix4x4 {
    const std::array<float, 16u> b {
        data.at(0u) * a.data.at(0u) + data.at(1u) * a.data.at(4u) + data.at(2u) * a.data.at(8u) + data.at(3u) * a.data.at(12u),
        data.at(0u) * a.data.at(1u) + data.at(1u) * a.data.at(5u) + data.at(2u) * a.data.at(9u) + data.at(3u) * a.data.at(13u),
        data.at(0u) * a.data.at(2u) + data.at(1u) * a.data.at(6u) + data.at(2u) * a.data.at(10u) + data.at(3u) * a.data.at(14u),
        data.at(0u) * a.data.at(2u) + data.at(1u) * a.data.at(6u) + data.at(2u) * a.data.at(10u) + data.at(3u) * a.data.at(14u),

        data.at(4u) * a.data.at(0u) + data.at(5u) * a.data.at(4u) + data.at(6u) * a.data.at(8u) + data.at(7u) * a.data.at(12u),
        data.at(4u) * a.data.at(1u) + data.at(5u) * a.data.at(5u) + data.at(6u) * a.data.at(9u) + data.at(7u) * a.data.at(13u),
        data.at(4u) * a.data.at(2u) + data.at(5u) * a.data.at(6u) + data.at(6u) * a.data.at(10u) + data.at(7u) * a.data.at(14u),
        data.at(4u) * a.data.at(2u) + data.at(5u) * a.data.at(6u) + data.at(6u) * a.data.at(10u) + data.at(7u) * a.data.at(14u),

        data.at(8u) * a.data.at(0u) + data.at(9u) * a.data.at(4u) + data.at(10u) * a.data.at(8u) + data.at(11u) * a.data.at(12u),
        data.at(8u) * a.data.at(1u) + data.at(9u) * a.data.at(5u) + data.at(10u) * a.data.at(9u) + data.at(11u) * a.data.at(13u),
        data.at(8u) * a.data.at(2u) + data.at(9u) * a.data.at(6u) + data.at(10u) * a.data.at(10u) + data.at(11u) * a.data.at(14u),
        data.at(8u) * a.data.at(2u) + data.at(9u) * a.data.at(6u) + data.at(10u) * a.data.at(10u) + data.at(11u) * a.data.at(14u),

        data.at(12u) * a.data.at(0u) + data.at(13u) * a.data.at(4u) + data.at(14u) * a.data.at(8u) + data.at(15u) * a.data.at(12u),
        data.at(12u) * a.data.at(1u) + data.at(13u) * a.data.at(5u) + data.at(14u) * a.data.at(9u) + data.at(15u) * a.data.at(13u),
        data.at(12u) * a.data.at(2u) + data.at(13u) * a.data.at(6u) + data.at(14u) * a.data.at(10u) + data.at(15u) * a.data.at(14u),
        data.at(12u) * a.data.at(2u) + data.at(13u) * a.data.at(6u) + data.at(14u) * a.data.at(10u) + data.at(15u) * a.data.at(14u)
    };

    return Matrix4x4(b);
}

auto Matrix4x4::identity() -> Matrix4x4 {
    return Matrix4x4(
        Vector4(1.0f, 0.0f, 0.0f, 0.0f),
        Vector4(0.0f, 1.0f, 0.0f, 0.0f),
        Vector4(0.0f, 0.0f, 1.0f, 0.0f),
        Vector4(0.0f, 0.0f, 0.0f, 1.0f)
    );
}

auto Matrix4x4::ortographic(
    const float left,
    const float right,
    const float bottom,
    const float top,
    const float nearClip,
    const float farClip
) -> Matrix4x4 {
    const float lr { 1.0f / (left - right) };
    const float bt { 1.0f / (bottom - top) };
    const float nf { 1.0f / (nearClip - farClip) };
    return Matrix4x4(
        Vector4(-2.0f * lr, 0.0f, 0.0f, 0.0f),
        Vector4(0.0f, -2.0f * bt, 0.0f, 0.0f),
        Vector4(0.0f, 0.0f, 2.0f * nf, 0.0f),
        Vector4((left + right) * lr, (top + bottom) * bt, (farClip + nearClip) * nf, 1.0f)
    );
}

auto Matrix4x4::perspective(
    const float fovRadians,
    const float aspectRatio,
    const float nearClip,
    const float farClip
) -> Matrix4x4 {
    const float halfTanFov { std::tan(fovRadians * 0.5f) };
    return Matrix4x4(
        Vector4(1.0f / (aspectRatio * halfTanFov), 0.0f, 0.0f, 0.0f),
        Vector4(0.0f, 1.0f / halfTanFov, 0.0f, 0.0f),
        Vector4(0.0f, 0.0f, -((farClip + nearClip) / (farClip - nearClip)), -1.0f),
        Vector4(0.0f, 0.0f, -((2.0f * farClip * nearClip) / (farClip - nearClip)), 0.0f)
    );
}

auto Matrix4x4::lookAt(
    const Vector3& position,
    const Vector3& target,
    const Vector3& up
) -> Matrix4x4 {
    Vector3 zAxis {
        target.x - position.x,
        target.y - position.y,
        target.z - position.z
    };
    zAxis.normalize();

    Vector3 xAxis {
        Vector3::cross(zAxis, up)
    };
    xAxis.normalize();

    Vector3 yAxis {
        Vector3::cross(xAxis, zAxis)
    };
    yAxis.normalize();

    const std::array<float, 16u> a {
        xAxis.x, yAxis.x, -zAxis.x, 0.0f,
        xAxis.y, yAxis.y, -zAxis.y, 0.0f,
        xAxis.z, yAxis.z, -zAxis.z, 0.0f,
        -Vector3::dot(xAxis, position), -Vector3::dot(yAxis, position), Vector3::dot(zAxis, position), 1.0f
    };

    return Matrix4x4(a);
}

auto Matrix4x4::translation(const Vector3& position) -> Matrix4x4 {
    Matrix4x4 a { identity() };
    a.data.at(12u) = position.x;
    a.data.at(13u) = position.y;
    a.data.at(14u) = position.z;
    return a;
}

auto Matrix4x4::scale(const Vector3& scale) -> Matrix4x4 {
    Matrix4x4 a { identity() };
    a.data.at(0u) = scale.x;
    a.data.at(5u) = scale.y;
    a.data.at(10u) = scale.z;
    return a;
}

// 0   1   2   3
// 4   5   6   7
// 8   9   10  11
// 12  13  14  15

auto Matrix4x4::xEuler(const float angleInRadians) -> Matrix4x4 {
    Matrix4x4 a { identity() };
    const float c { std::cos(angleInRadians) };
    const float s { std::sin(angleInRadians) };
    a.data.at(5u) = c;
    a.data.at(6u) = s;
    a.data.at(9u) = -s;
    a.data.at(10u) = c;
    return a;
}

auto Matrix4x4::yEuler(const float angleInRadians) -> Matrix4x4 {
    Matrix4x4 a { identity() };
    const float c { std::cos(angleInRadians) };
    const float s { std::sin(angleInRadians) };
    a.data.at(0u) = c;
    a.data.at(2u) = -s;
    a.data.at(8u) = s;
    a.data.at(10u) = c;
    return a;
}

auto Matrix4x4::zEuler(const float angleInRadians) -> Matrix4x4 {
    Matrix4x4 a { identity() };
    const float c { std::cos(angleInRadians) };
    const float s { std::sin(angleInRadians) };
    a.data.at(0u) = c;
    a.data.at(1u) = s;
    a.data.at(4u) = -s;
    a.data.at(5u) = c;
    return a;
}

auto Matrix4x4::xyzEuler(
    const float xRadians,
    const float yRadians,
    const float zRadians
) -> Matrix4x4 {
    return xEuler(xRadians) * xEuler(yRadians) * xEuler(zRadians);
}

auto Matrix4x4::inverse() -> void {
    const float t0 { data.at(10u) * data.at(15u) };
    const float t1 { data.at(14u) * data.at(11u) };
    const float t2 { data.at(6u) * data.at(15u) };
    const float t3 { data.at(14u) * data.at(7u) };
    const float t4 { data.at(6u) * data.at(11u) };
    const float t5 { data.at(10u) * data.at(7u) };
    const float t6 { data.at(2u) * data.at(15u) };
    const float t7 { data.at(14u) * data.at(3u) };
    const float t8 { data.at(2u) * data.at(11u) };
    const float t9 { data.at(10u) * data.at(3u) };
    const float t10 { data.at(2u) * data.at(7u) };
    const float t11 { data.at(6u) * data.at(3u) };
    const float t12 { data.at(8u) * data.at(13u) };
    const float t13 { data.at(12u) * data.at(9u) };
    const float t14 { data.at(4u) * data.at(13u) };
    const float t15 { data.at(12u) * data.at(5u) };
    const float t16 { data.at(4u) * data.at(9u) };
    const float t17 { data.at(8u) * data.at(5u) };
    const float t18 { data.at(0u) * data.at(13u) };
    const float t19 { data.at(12u) * data.at(1u) };
    const float t20 { data.at(0u) * data.at(9u) };
    const float t21 { data.at(8u) * data.at(1u) };
    const float t22 { data.at(0u) * data.at(5u) };
    const float t23 { data.at(4u) * data.at(1u) };

    std::array<float, 16u> inverted { 0.0f };

    inverted.at(0u) = (t0 * data.at(5u) + t3 * data.at(9u) + t4 * data.at(13u)) - (t1 * data.at(5u) + t2 * data.at(9u) + t5 * data.at(13u));
    inverted.at(1u) = (t1 * data.at(1u) + t6 * data.at(9u) + t9 * data.at(13u)) - (t0 * data.at(1u) + t7 * data.at(9u) + t8 * data.at(13u));
    inverted.at(2u) = (t2 * data.at(1u) + t7 * data.at(5u) + t10 * data.at(13u)) - (t3 * data.at(1u) + t6 * data.at(5u) + t11 * data.at(13u));
    inverted.at(3u) = (t5 * data.at(1u) + t8 * data.at(5u) + t11 * data.at(9u)) - (t4 * data.at(1u) + t9 * data.at(5u) + t10 * data.at(9u));

    const float d {
        1.0f / (
            data.at(0u) * inverted.at(0u) +
            data.at(4u) * inverted.at(1u) +
            data.at(8u) * inverted.at(2u) +
            data.at(12u) * inverted.at(3u)
        )
    };

    inverted.at(0u) = d * inverted.at(0u);
    inverted.at(1u) = d * inverted.at(1u);
    inverted.at(2u) = d * inverted.at(2u);
    inverted.at(3u) = d * inverted.at(3u);
    inverted.at(4u) = d * ((t1 * data.at(4u) + t2 * data.at(8u) + t5 * data.at(12u)) - (t0 * data.at(4u) + t3 * data.at(8u) + t4 * data.at(12u)));
    inverted.at(5u) = d * ((t0 * data.at(0u) + t7 * data.at(8u) + t8 * data.at(12u)) - (t1 * data.at(0u) + t6 * data.at(8u) + t9 * data.at(12u)));
    inverted.at(6u) = d * ((t3 * data.at(0u) + t6 * data.at(4u) + t11 * data.at(12u)) - (t2 * data.at(0u) + t7 * data.at(4u) + t10 * data.at(12u)));
    inverted.at(7u) = d * ((t4 * data.at(0u) + t9 * data.at(4u) + t10 * data.at(8u)) - (t5 * data.at(0u) + t8 * data.at(4u) + t11 * data.at(8u)));
    inverted.at(8u) = d * ((t12 * data.at(7u) + t15 * data.at(11u) + t16 * data.at(15u)) - (t13 * data.at(7u) + t14 * data.at(11u) + t17 * data.at(15u)));
    inverted.at(9u) = d * ((t13 * data.at(3u) + t18 * data.at(11u) + t21 * data.at(15u)) - (t12 * data.at(3u) + t19 * data.at(11u) + t20 * data.at(15u)));
    inverted.at(10u) = d * ((t14 * data.at(3u) + t19 * data.at(7u) + t22 * data.at(15u)) - (t15 * data.at(3u) + t18 * data.at(7u) + t23 * data.at(15u)));
    inverted.at(11u) = d * ((t17 * data.at(3u) + t20 * data.at(7u) + t23 * data.at(11u)) - (t16 * data.at(3u) + t21 * data.at(7u) + t22 * data.at(11u)));
    inverted.at(12u) = d * ((t14 * data.at(10u) + t17 * data.at(14u) + t13 * data.at(6u)) - (t16 * data.at(14u) + t12 * data.at(6u) + t15 * data.at(10u)));
    inverted.at(13u) = d * ((t20 * data.at(14u) + t12 * data.at(2u) + t19 * data.at(10u)) - (t18 * data.at(10u) + t21 * data.at(14u) + t13 * data.at(2u)));
    inverted.at(14u) = d * ((t18 * data.at(6u) + t23 * data.at(14u) + t15 * data.at(2u)) - (t22 * data.at(14u) + t14 * data.at(2u) + t19 * data.at(6u)));
    inverted.at(15u) = d * ((t22 * data.at(10u) + t16 * data.at(2u) + t21 * data.at(6u)) - (t20 * data.at(6u) + t23 * data.at(10u) + t17 * data.at(2u)));

    data = inverted;
}

auto Matrix4x4::transposed() -> Matrix4x4 {
    Matrix4x4 a { identity() };
    a.data.at(0u) = data.at(0u);
    a.data.at(1u) = data.at(4u);
    a.data.at(2u) = data.at(8u);
    a.data.at(3u) = data.at(12u);
    a.data.at(4u) = data.at(1u);
    a.data.at(5u) = data.at(5u);
    a.data.at(6u) = data.at(9u);
    a.data.at(7u) = data.at(13u);
    a.data.at(8u) = data.at(2u);
    a.data.at(9u) = data.at(6u);
    a.data.at(10u) = data.at(10u);
    a.data.at(11u) = data.at(14u);
    a.data.at(12u) = data.at(3u);
    a.data.at(13u) = data.at(7u);
    a.data.at(14u) = data.at(11u);
    a.data.at(15u) = data.at(15u);
    return a;
}

auto Matrix4x4::forward() -> Vector3 {
    Vector3 forward {
        -data.at(2u),
        -data.at(6u),
        -data.at(10u)
    };
    forward.normalize();
    return forward;
}
auto Matrix4x4::backward() -> Vector3 {
    Vector3 backward {
        data.at(2u),
        data.at(6u),
        data.at(10u)
    };
    backward.normalize();
    return backward;
}

auto Matrix4x4::up() -> Vector3 {
    Vector3 up {
        data.at(1u),
        data.at(5u),
        data.at(9u)
    };
    up.normalize();
    return up;
}

auto Matrix4x4::down() -> Vector3 {
    Vector3 down {
        -data.at(1u),
        -data.at(5u),
        -data.at(9u)
    };
    down.normalize();
    return down;
}

auto Matrix4x4::left() -> Vector3 {
    Vector3 left {
        -data.at(0u),
        -data.at(4u),
        -data.at(8u)
    };
    left.normalize();
    return left;
}

auto Matrix4x4::right() -> Vector3 {
    Vector3 right {
        data.at(0u),
        data.at(4u),
        data.at(8u)
    };
    right.normalize();
    return right;
}

} // namespace math
} // namespace beige
