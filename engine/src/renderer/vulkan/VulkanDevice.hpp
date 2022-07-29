#pragma once

#include "VulkanSurface.hpp"

#include <vulkan/vulkan.h>

#include <vector>
#include <string>
#include <optional>

namespace beige {
namespace renderer {
namespace vulkan {

class Device final {
public:
    struct SwapchainSupport {
        VkSurfaceCapabilitiesKHR surfaceCapabilities;
        std::vector<VkSurfaceFormatKHR> surfaceFormats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    Device(
        VkAllocationCallbacks* allocationCallbacks,
        const VkInstance& instance,
        std::shared_ptr<Surface> surface
    );

    ~Device();

    auto getLogicalDevice() const -> const VkDevice&;
    auto getPhysicalDevice() const -> const VkPhysicalDevice&;
    auto getSwapchainSupport() const -> const SwapchainSupport&;
    auto getGraphicsQueueIndex() const -> const std::optional<uint32_t>&;
    auto getPresentQueueIndex() const -> const std::optional<uint32_t>&;
    auto getDepthFormat() const -> const VkFormat&;
    auto getGraphicsCommandPool() const -> const VkCommandPool&;
    auto getGraphicsQueue() const -> const VkQueue&;
    auto getPresentQueue() const -> const VkQueue&;

    auto supportsDeviceLocalHostVisible() const -> bool;

    auto querySwapchainSupport(
        const VkPhysicalDevice& physicalDevice
    ) -> void;

    auto detectDepthFormat() -> void;

    auto findMemoryIndex(
        const uint32_t typeFilter,
        const uint32_t propertyFlags
    ) -> std::optional<uint32_t>;

private:
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
    std::shared_ptr<Surface> m_surface;

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

    VkCommandPool m_graphicsCommandPool;

    VkPhysicalDeviceProperties m_physicalDeviceProperties;
    VkPhysicalDeviceFeatures m_physicalDeviceFeatures;
    VkPhysicalDeviceMemoryProperties m_physicalDeviceMemoryProperties;

    VkFormat m_depthFormat;

    bool m_supportsDeviceLocalHostVisible;

    auto selectPhysicalDevice(
        const VkInstance& instance
    ) -> bool;

    auto physicalDeviceMeetsRequirements(
        const VkPhysicalDevice& physicalDevice,
        const VkPhysicalDeviceProperties& physicalDeviceProperties,
        const VkPhysicalDeviceFeatures& physicalDeviceFeatures,
        const PhysicalDeviceRequirements& physicalDeviceRequirements,
        PhysicalDeviceQueueFamilies& physicalDeviceQueueFamilies
    ) -> bool;
};

} // namespace vulkan
} // namespace renderer
} // namespace beige
