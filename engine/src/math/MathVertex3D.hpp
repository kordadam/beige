#pragma once

#include "MathVector3.hpp"

namespace beige {
namespace math {

class Vertex3D {
public:
    Vertex3D() = default;
    ~Vertex3D() = default;

    Vector3 m_position;
};

} // namespace math
} // namespace beige

