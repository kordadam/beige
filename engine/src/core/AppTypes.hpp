#pragma once

#include <cstdint>
#include <string>

namespace beige {
namespace core {

struct AppConfig {
    uint32_t xStartPos;
    uint32_t yStartPos;
    uint32_t startWidth;
    uint32_t startHeight;
    std::string name;
};

} // namespace core
} // namespace beige
