#pragma once

#include <vulkan/vulkan.h>

#include <string>

namespace beige {
namespace renderer {
namespace vulkan {

class Utils final {
public:
    Utils() = delete;
    ~Utils() = delete;

    /**
     * Returns the string representation of result.
     * @param result The result to get the string for.
     * @param getExtended Indicates wheter to also return an extended result.
     * @returns The error code and/or extended error message in string form. Defaults to success for unknown result types.
     */
    static auto resultToString(const VkResult& result, const bool getExtended) -> std::string;

    /**
     * Indicates if the passed result is a success or an error as defined by the Vulkan specification.
     * @param result The result to evaluate.
     * @returns True if success, otherwise false. Defaults to true for unknown result types.
     */
    static auto isResultSuccess(const VkResult& result) -> bool;
};

} // namespace vulkan
} // namespace renderer
} // namespace beige
