#pragma once

#include "../../core/Assertions.hpp"

#include <string>

namespace notidok {
namespace renderer {
namespace vulkan {

#define VULKAN_CHECK(condition) \
{ \
    ASSERT(condition == VK_SUCCESS); \
}

} // namespace vulkan
} // namespace renderer
} // namespace notidok
