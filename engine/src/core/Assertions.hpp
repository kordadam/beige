#pragma once

#include "Logger.hpp"
#include "../Defines.hpp"

namespace beige {
namespace core {

#define BEIGE_ASSERTIONS_ENABLED

#ifdef BEIGE_ASSERTIONS_ENABLED

#define ASSERT(condition) { \
    if (condition) { \
    } else { \
        beige::core::Logger::reportAssertionFailure(#condition, "", __FILE__, __LINE__); \
        debugBreak(); \
    } \
}

#define ASSERT_MESSAGE(condition, message) { \
    if (condition) { \
    } else { \
        beige::core::Logger::reportAssertionFailure(#condition, message, __FILE__, __LINE__); \
        debugBreak(); \
    } \
}

#ifdef BEIGE_DEBUG
#define ASSERT_DEBUG(condition) { \
    if (condition) { \
    } else { \
        beige::core::Logger::reportAssertionFailure(#condition, "", __FILE__, __LINE__); \
        debugBreak(); \
    } \
}
#else

#define ASSERT_DEBUG(condition)

#endif // BEIGE_DEBUG

#else

#define ASSERT(condition)
#define ASSERT_MESSAGE(condition, message)
#define ASSERT_DEBUG(condition)

#endif // BEIGE_ASSERTIONS_ENABLED

} // namespace core
} // namespace beige
