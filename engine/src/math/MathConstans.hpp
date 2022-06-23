#pragma once

#include <limits>

namespace beige {
namespace math {

constexpr float pi{ 3.14159265358979323846f };
constexpr float twoPi{ 2.0f * pi };
constexpr float halfPi{ 0.5f * pi };
constexpr float quarterPi{ 0.25f * pi };
constexpr float oneOverPi{ 1.0f / pi };
constexpr float oneOverTwoPi{ 1.0f / twoPi };
constexpr float sqrtTwo{ 1.41421356237309504880f };
constexpr float sqrtThree{ 1.73205080756887729352f };
constexpr float sqrtOneOverTwo{ 0.70710678118654752440f };
constexpr float sqrtOneOverThree{ 0.57735026918962576450f };
constexpr float degToRadMultiplier{ pi / 180.0f };
constexpr float radToDegMultiplier{ 180.0f / pi };

// The multiplier to convert seconds to milliseconds.
constexpr float sToMsMultiplier{ 1000.0f };

// The multiplier to convert milliseconds to seconds.
constexpr float msToSMultiplier{ 0.001f };

// A huge number that should be larger than any valid number used.
constexpr float infinity{ std::numeric_limits<float>::infinity() };

// Smallest positive number.
constexpr float epsilon{ std::numeric_limits<float>::epsilon() };

} // namespace math
} // namespace beige
