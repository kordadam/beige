#include "MathRandom.hpp"

namespace beige {
namespace math {

bool Random::m_initialized { false };

auto Random::randomInt() -> int32_t {
    if (!m_initialized) {
        initialize();
    }

    return static_cast<int32_t>(std::rand());
}

auto Random::randomFloat() -> float {
    return static_cast<float>(randomInt()) / static_cast<float>(RAND_MAX);
}

auto Random::range(
    const int32_t min,
    const int32_t max
) -> int32_t {
    return (randomInt() % (max - min + 1)) + min;
}

auto Random::range(
    const float min,
    const float max
) -> float {
    return min + (randomFloat() / (max - min));
}

auto Random::initialize() -> void {
    std::srand(std::time(nullptr));
    m_initialized = true;
}

} // namespace math
} // namespace beige
