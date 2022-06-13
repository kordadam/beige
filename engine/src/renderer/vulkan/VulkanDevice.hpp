#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <string>
#include <optional>

namespace beige {
namespace renderer {
namespace vulkan {

class VulkanDevice final {
public:
    VulkanDevice(
        VkAllocationCallbacks* allocationCallbacks,
        const VkInstance& instance,
        const VkSurfaceKHR& surface
    );
    ~VulkanDevice();

private:
    struct SwapchainSupport {
        VkSurfaceCapabilitiesKHR surfaceCapabilities;
        std::vector<VkSurfaceFormatKHR> surfaceFormats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    struct PhysicalDeviceRequirements {
        bool graphics;
        bool present;
        bool compute;
        bool transfer;
        std::vector<std::string> deviceExtensionNames;
        bool samplerAnisotrophy;
        bool discrete;
    };

    struct PhysicalDeviceQueueFamilies {
        std::optional<uint32_t> graphicsFamilyIndex;
        std::optional<uint32_t> presentFamilyIndex;
        std::optional<uint32_t> computeFamilyIndex;
        std::optional<uint32_t> transferFamilyIndex;
    };

    VkAllocationCallbacks* m_allocationCallbacks;
    VkPhysicalDevice m_physicalDevice;
    VkDevice m_logicalDevice;
    SwapchainSupport m_swapchainSupport;
    std::optional<uint32_t> m_graphicsQueueIndex;
    std::optional<uint32_t> m_presentQueueIndex;
    std::optional<uint32_t> m_computeQueueIndex;
    std::optional<uint32_t> m_transferQueueIndex;
    VkQueue m_graphicsQueue;
    VkQueue m_presentQueue;
    VkQueue m_transferQueue;
    VkPhysicalDeviceProperties m_physicalDeviceProperties;
    VkPhysicalDeviceFeatures m_physicalDeviceFeatures;
    VkPhysicalDeviceMemoryProperties m_physicalDeviceMemoryProperties;

    auto selectPhysicalDevice(
        const VkInstance& instance,
        const VkSurfaceKHR& surface
    ) -> bool;

    auto physicalDeviceMeetsRequirements(
        const VkPhysicalDevice& physicalDevice,
        const VkSurfaceKHR& surface,
        const VkPhysicalDeviceProperties& physicalDeviceProperties,
        const VkPhysicalDeviceFeatures& physicalDeviceFeatures,
        const PhysicalDeviceRequirements& physicalDeviceRequirements,
        PhysicalDeviceQueueFamilies& physicalDeviceQueueFamilies,
        SwapchainSupport& swapchainSupport
    ) -> bool;

    auto deviceQuerySwapchainSupport(
        const VkPhysicalDevice& physicalDevice,
        const VkSurfaceKHR& surface,
        SwapchainSupport& swapchainSupport
    ) -> void;
};

} // namespace vulkan
} // namespace renderer
} // namespace beige
