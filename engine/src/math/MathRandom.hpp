#pragma once

#include "../Defines.hpp"

// TODO: Use <random>
#include <ctime>
#include <cstdlib>
#include <cstdint>

namespace beige {
namespace math {

class BEIGE_API Random {
public:
    Random() = delete;
    ~Random() = delete;

    static auto randomInt() -> int32_t;
    static auto randomFloat() -> float;
    static auto range(
        const int32_t min,
        const int32_t max
    ) -> int32_t;
    static auto range(
        const float min,
        const float max
    ) -> float;

private:
    static bool m_initialized;

    static auto initialize() -> void;
};

} // namespace math
} // namespace beige
