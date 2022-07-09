#pragma once

#include "MathVector3.hpp"

namespace beige {
namespace math {

class Vertex3D {
public:
    Vertex3D(const float x, const float y, const float z);
    ~Vertex3D();

    Vector3 m_position;
};

} // namespace math
} // namespace beige

