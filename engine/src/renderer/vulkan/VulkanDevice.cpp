#include "VulkanDevice.hpp"

#include "VulkanDefines.hpp"

#include <exception>
#include <sstream>
#include <array>
#include <algorithm>

namespace beige {
namespace renderer {
namespace vulkan {

Device::Device(
    VkAllocationCallbacks* allocationCallbacks,
    const VkInstance& instance,
    std::shared_ptr<Surface> surface
) :
m_allocationCallbacks { allocationCallbacks },
m_surface { surface },
m_physicalDevice { 0 },
m_logicalDevice { 0 },
m_swapchainSupport { },
m_graphicsQueueIndex { std::nullopt },
m_presentQueueIndex { std::nullopt },
m_computeQueueIndex { std::nullopt },
m_transferQueueIndex { std::nullopt },
m_graphicsQueue { 0 },
m_presentQueue { 0 },
m_transferQueue { 0 },
m_graphicsCommandPool { 0 },
m_physicalDeviceProperties { 0 },
m_physicalDeviceFeatures { 0 },
m_physicalDeviceMemoryProperties { 0 },
m_depthFormat { } {
    if (!selectPhysicalDevice(instance)) {
        throw std::exception("Failed to create device!");
    }

    core::Logger::info("Creating logical device...");
    // Do not create additional queues for shared indices

    std::vector<VkDeviceQueueCreateInfo> deviceQueueCreateInfos;

    std::array<float, 1u> queuePriorities { 1.0f };
    VkDeviceQueueCreateInfo graphicsDeviceQueueCreateInfo {
        VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, // sType
        nullptr, // pNext
        0u, // flags
        m_graphicsQueueIndex.value(), // queueFamilyIndex
        static_cast<uint32_t>(queuePriorities.size()), // queueCount
        queuePriorities.data() // pQueuePriorities
    };
    deviceQueueCreateInfos.push_back(graphicsDeviceQueueCreateInfo);

    if (m_graphicsQueueIndex.value() != m_presentQueueIndex.value()) {
        VkDeviceQueueCreateInfo presentDeviceQueueCreateInfo {
            VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, // sType
            nullptr, // pNext
            0u, // flags
            m_presentQueueIndex.value(), // queueFamilyIndex
            static_cast<uint32_t>(queuePriorities.size()), // queueCount
            queuePriorities.data() // pQueuePriorities
        };
        deviceQueueCreateInfos.push_back(presentDeviceQueueCreateInfo);
    }

    if (m_graphicsQueueIndex.value() != m_transferQueueIndex.value()) {
        VkDeviceQueueCreateInfo transferDeviceQueueCreateInfo {
            VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, // sType
            nullptr, // pNext
            0u, // flags
            m_transferQueueIndex.value(), // queueFamilyIndex
            static_cast<uint32_t>(queuePriorities.size()), // queueCount
            queuePriorities.data() // pQueuePriorities
        };
        deviceQueueCreateInfos.push_back(transferDeviceQueueCreateInfo);
    }

    // Request device features
    // TODO: Should be config driven
    VkPhysicalDeviceFeatures deviceFeatures { VK_FALSE };
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    std::vector<const char*> extensionNames {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    VkDeviceCreateInfo deviceCreateInfo {
        VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, // sType
        nullptr, // pNext
        0u, // flags
        static_cast<uint32_t>(deviceQueueCreateInfos.size()), // queueCreateInfoCount
        deviceQueueCreateInfos.data(), // pQueueCreateInfos
        0u, // enabledLayerCount
        nullptr, // ppEnabledLayerNames
        static_cast<uint32_t>(extensionNames.size()), // enabledExtensionCount
        extensionNames.data(), // ppEnabledExtensionNames
        &deviceFeatures // pEnabledFeatures
    };

    VULKAN_CHECK(
        vkCreateDevice(
            m_physicalDevice,
            &deviceCreateInfo,
            m_allocationCallbacks,
            &m_logicalDevice
        )
    );

    core::Logger::info("Logical device created!");

    vkGetDeviceQueue(
        m_logicalDevice,
        m_graphicsQueueIndex.value(),
        0u,
        &m_graphicsQueue
    );

    vkGetDeviceQueue(
        m_logicalDevice,
        m_presentQueueIndex.value(),
        0u,
        &m_presentQueue
    );

    vkGetDeviceQueue(
        m_logicalDevice,
        m_transferQueueIndex.value(),
        0u,
        &m_transferQueue
    );

    core::Logger::info("Queues obtained!");

    // Create command pool for graphics queue
    const VkCommandPoolCreateInfo commandPoolCreateInfo {
        VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, // sType
        nullptr, // pNext
        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, // flags
        m_graphicsQueueIndex.value() // queueFamilyIndex
    };

    VULKAN_CHECK(
        vkCreateCommandPool(
            m_logicalDevice,
            &commandPoolCreateInfo,
            m_allocationCallbacks,
            &m_graphicsCommandPool
        )
    );

    core::Logger::info("Graphics command pool created!");
}

Device::~Device() {
    core::Logger::info("Destroying command pools...");
    vkDestroyCommandPool(
        m_logicalDevice,
        m_graphicsCommandPool,
        m_allocationCallbacks
    );

    core::Logger::info("Destroying logical device...");

    vkDestroyDevice(
        m_logicalDevice,
        m_allocationCallbacks
    );
}

auto Device::getLogicalDevice() const -> const VkDevice& {
    return m_logicalDevice;
}

auto Device::getPhysicalDevice() const -> const VkPhysicalDevice& {
    return m_physicalDevice;
}

auto Device::getSwapchainSupport() const -> const SwapchainSupport& {
    return m_swapchainSupport;
}

auto Device::getGraphicsQueueIndex() const -> const std::optional<uint32_t>& {
    return m_graphicsQueueIndex;
}
auto Device::getPresentQueueIndex() const -> const std::optional<uint32_t>& {
    return m_presentQueueIndex;
}

auto Device::getDepthFormat() const -> const VkFormat& {
    return m_depthFormat;
}

auto Device::getGraphicsCommandPool() const -> const VkCommandPool& {
    return m_graphicsCommandPool;
}

auto Device::getGraphicsQueue() const -> const VkQueue& {
    return m_graphicsQueue;
}

auto Device::getPresentQueue() const -> const VkQueue& {
    return m_presentQueue;
}

auto Device::querySwapchainSupport(
    const VkPhysicalDevice& physicalDevice
) -> void {
    const VkSurfaceKHR surface { m_surface->getHandle() };

    VULKAN_CHECK(
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
            physicalDevice,
            surface,
            &m_swapchainSupport.surfaceCapabilities
        )
    );

    uint32_t formatCount{ 0u };
    VULKAN_CHECK(
        vkGetPhysicalDeviceSurfaceFormatsKHR(
            physicalDevice,
            surface,
            &formatCount,
            nullptr
        )
    );

    if (formatCount > 0u) {
        m_swapchainSupport.surfaceFormats.resize(formatCount);
        VULKAN_CHECK(
            vkGetPhysicalDeviceSurfaceFormatsKHR(
                physicalDevice,
                surface,
                &formatCount,
                m_swapchainSupport.surfaceFormats.data()
            )
        );
    }

    uint32_t presentModeCount{ 0u };
    VULKAN_CHECK(
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            physicalDevice,
            surface,
            &presentModeCount,
            nullptr
        )
    );

    if (presentModeCount > 0u) {
        m_swapchainSupport.presentModes.resize(presentModeCount);
        VULKAN_CHECK(
            vkGetPhysicalDeviceSurfacePresentModesKHR(
                physicalDevice,
                surface,
                &presentModeCount,
                m_swapchainSupport.presentModes.data()
            )
        );
    }
}

auto Device::detectDepthFormat() -> void {
    const std::array<VkFormat, 3u> candidates {
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT
    };

    const uint32_t flags { VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT };

    for (const VkFormat& candidate : candidates) {
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(
            m_physicalDevice,
            candidate,
            &formatProperties
        );

        if ((formatProperties.linearTilingFeatures & flags) != 0u) {
            m_depthFormat = candidate;
            return;
        } else if ((formatProperties.optimalTilingFeatures & flags) != 0u) {
            m_depthFormat = candidate;
            return;
        }
    }

    m_depthFormat = VK_FORMAT_UNDEFINED;
    core::Logger::fatal("Failed to find a supported format!");
}

auto Device::findMemoryIndex(
    const uint32_t typeFilter,
    const uint32_t propertyFlags
) -> std::optional<uint32_t> {
    VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
    vkGetPhysicalDeviceMemoryProperties(
        m_physicalDevice,
        &physicalDeviceMemoryProperties
    );

    for (uint32_t i { 0u }; i < physicalDeviceMemoryProperties.memoryTypeCount; i++) {
        if (
            (typeFilter & (1u << i)) &&
            ((physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & propertyFlags) == propertyFlags)
        ) {
            return std::optional<uint32_t>(i);
        }
    }

    core::Logger::warn("Unable to find suitable memory type!");
    return std::nullopt;
}

auto Device::selectPhysicalDevice(
    const VkInstance& instance
) -> bool {
    uint32_t physicalDeviceCount { 0u };
    VULKAN_CHECK(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr));

    if (physicalDeviceCount == 0u) {
        core::Logger::fatal("No devices which support Vulkan were found!");
        return false;
    }

    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    VULKAN_CHECK(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data()));

    for (const VkPhysicalDevice& physicalDevice : physicalDevices) {
        VkPhysicalDeviceProperties physicalDeviceProperties;
        vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

        VkPhysicalDeviceFeatures physicalDeviceFeatures;
        vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatures);

        VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &physicalDeviceMemoryProperties);

        // TODO: These requirements should probably be driven by engine
        PhysicalDeviceRequirements physicalDeviceRequirements {
            true, // graphics
            true, // present
            false, // compute
            true, // transfer
            { VK_KHR_SWAPCHAIN_EXTENSION_NAME }, // deviceExtensionNames
            true, // samplerAnisotrophy
            true // discrete
        };

        PhysicalDeviceQueueFamilies physicalDeviceQueueFamilies {
            0u, // graphicsFamilyIndex
            0u, // presentFamilyIndex
            0u, // computeFamilyIndex
            0u // transferFamilyIndex
        };

        const bool result {
            physicalDeviceMeetsRequirements(
                physicalDevice,
                physicalDeviceProperties,
                physicalDeviceFeatures,
                physicalDeviceRequirements,
                physicalDeviceQueueFamilies
            )
        };

        if (result) {
            core::Logger::info("Selected device: " + std::string(physicalDeviceProperties.deviceName));

            switch (physicalDeviceProperties.deviceType) {
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                core::Logger::info("GPU type is integrated!");
                break;
            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                core::Logger::info("GPU type is descrete!");
                break;
            case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                core::Logger::info("GPU type is virtual!");
                break;
            case VK_PHYSICAL_DEVICE_TYPE_CPU:
                core::Logger::info("GPU type is CPU!");
                break;
            case VK_PHYSICAL_DEVICE_TYPE_OTHER: [[fallthrough]];
            default:
                core::Logger::info("GPU type is uknown!");
                break;
            }

            std::stringstream driverVersionLog;
            driverVersionLog <<
                "GPU driver version: " <<
                VK_VERSION_MAJOR(physicalDeviceProperties.driverVersion) << "." <<
                VK_VERSION_MINOR(physicalDeviceProperties.driverVersion) << "." <<
                VK_VERSION_PATCH(physicalDeviceProperties.driverVersion);
            core::Logger::info(driverVersionLog.str());

            std::stringstream vulkanVersionLog;
            vulkanVersionLog <<
                "Vulkan API version: " <<
                VK_VERSION_MAJOR(physicalDeviceProperties.apiVersion) << "." <<
                VK_VERSION_MINOR(physicalDeviceProperties.apiVersion) << "." <<
                VK_VERSION_PATCH(physicalDeviceProperties.apiVersion);
            core::Logger::info(vulkanVersionLog.str());

            for (uint32_t i { 0u }; i < physicalDeviceMemoryProperties.memoryHeapCount; i++) {
                const float memorySizeInGigabytes {
                    static_cast<float>(physicalDeviceMemoryProperties.memoryHeaps[i].size) / 1024.0f / 1024.0f / 1024.0f
                };

                if ((physicalDeviceMemoryProperties.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) != 0u) {
                    core::Logger::info("Local GPU memory: " + std::to_string(memorySizeInGigabytes) + "GB");
                } else {
                    core::Logger::info("Shared system memory: " + std::to_string(memorySizeInGigabytes) + "GB");
                }
            }

            m_physicalDevice = physicalDevice;
            m_graphicsQueueIndex = physicalDeviceQueueFamilies.graphicsFamilyIndex.value();
            m_presentQueueIndex = physicalDeviceQueueFamilies.presentFamilyIndex.value();
            m_computeQueueIndex = physicalDeviceQueueFamilies.computeFamilyIndex.value();
            m_transferQueueIndex = physicalDeviceQueueFamilies.transferFamilyIndex.value();
            m_physicalDeviceProperties = physicalDeviceProperties;
            m_physicalDeviceFeatures = physicalDeviceFeatures;
            m_physicalDeviceMemoryProperties = physicalDeviceMemoryProperties;
            break;
        }
    }

    if (m_physicalDevice == 0) {
        core::Logger::error("No physical devices were found which meet the requirements!");
        return false;
    }

    core::Logger::info("Physical device selected!");
    return true;
}

auto Device::physicalDeviceMeetsRequirements(
    const VkPhysicalDevice& physicalDevice,
    const VkPhysicalDeviceProperties& physicalDeviceProperties,
    const VkPhysicalDeviceFeatures& physicalDeviceFeatures,
    const PhysicalDeviceRequirements& physicalDeviceRequirements,
    PhysicalDeviceQueueFamilies& physicalDeviceQueueFamilies
) -> bool {
    physicalDeviceQueueFamilies.graphicsFamilyIndex = std::nullopt;
    physicalDeviceQueueFamilies.presentFamilyIndex = std::nullopt;
    physicalDeviceQueueFamilies.computeFamilyIndex = std::nullopt;
    physicalDeviceQueueFamilies.transferFamilyIndex = std::nullopt;

    if (
        physicalDeviceRequirements.discrete &&
        physicalDeviceProperties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
    ) {
        core::Logger::info("Device is not a discrete GPU, and one is required, skipping...");
        return false;
    }

    uint32_t queueFamilyPropertyCount { 0u };
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyPropertyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertyCount, queueFamilyProperties.data());

    uint32_t minTransferScore { 255u };

    const VkSurfaceKHR surface { m_surface->getHandle() };

    for (uint32_t i { 0u }; i < queueFamilyProperties.size(); i++) {
        uint32_t currentTransferScore { 0u };

        if ((queueFamilyProperties.at(i).queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0u) {
            physicalDeviceQueueFamilies.graphicsFamilyIndex = i;
            currentTransferScore++;
        }

        if ((queueFamilyProperties.at(i).queueFlags & VK_QUEUE_COMPUTE_BIT) != 0u) {
            physicalDeviceQueueFamilies.computeFamilyIndex = i;
            currentTransferScore++;
        }

        if ((queueFamilyProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT) != 0u) {
            if (currentTransferScore <= minTransferScore) {
                minTransferScore = currentTransferScore;
                physicalDeviceQueueFamilies.transferFamilyIndex = i;
            }
        }

        VkBool32 supportsPreset { VK_FALSE };
        VULKAN_CHECK(
            vkGetPhysicalDeviceSurfaceSupportKHR(
                physicalDevice,
                i,
                surface,
                &supportsPreset
            )
        );

        if (supportsPreset) {
            physicalDeviceQueueFamilies.presentFamilyIndex = i;
        }
    }

    std::stringstream physicalDeviceQueueFamiliesLog;
    physicalDeviceQueueFamiliesLog <<
        "Graphics: " << (physicalDeviceQueueFamilies.graphicsFamilyIndex.has_value() ? "true" : "false") << " | " <<
        "Present: " << (physicalDeviceQueueFamilies.presentFamilyIndex.has_value() ? "true" : "false") << " | " <<
        "Compute: " << (physicalDeviceQueueFamilies.computeFamilyIndex.has_value() ? "true" : "false") << " | " <<
        "Transfer: " << (physicalDeviceQueueFamilies.transferFamilyIndex.has_value() ? "true" : "false") << " | " <<
        "Name: " << physicalDeviceProperties.deviceName;

    core::Logger::info(physicalDeviceQueueFamiliesLog.str());

    if (
        (!physicalDeviceRequirements.graphics || (physicalDeviceRequirements.graphics && physicalDeviceQueueFamilies.graphicsFamilyIndex.has_value())) &&
        (!physicalDeviceRequirements.present || (physicalDeviceRequirements.present && physicalDeviceQueueFamilies.presentFamilyIndex.has_value())) &&
        (!physicalDeviceRequirements.compute || (physicalDeviceRequirements.compute && physicalDeviceQueueFamilies.computeFamilyIndex.has_value())) &&
        (!physicalDeviceRequirements.transfer || (physicalDeviceRequirements.transfer && physicalDeviceQueueFamilies.transferFamilyIndex.has_value()))
    ) {
        core::Logger::info("Device meets queue requirements!");
        core::Logger::trace("Graphics family index: " + std::to_string(physicalDeviceQueueFamilies.graphicsFamilyIndex.value()));
        core::Logger::trace("Present family index: " + std::to_string(physicalDeviceQueueFamilies.presentFamilyIndex.value()));
        core::Logger::trace("Compute family index: " + std::to_string(physicalDeviceQueueFamilies.computeFamilyIndex.value()));
        core::Logger::trace("Transfer family index: " + std::to_string(physicalDeviceQueueFamilies.transferFamilyIndex.value()));
        querySwapchainSupport(physicalDevice);

        if (m_swapchainSupport.surfaceFormats.empty() || m_swapchainSupport.presentModes.empty()) {
            core::Logger::info("Required swapchain support not present, skipping device...");
            return false;
        }

        if (!physicalDeviceRequirements.deviceExtensionNames.empty()) {
            uint32_t propertyCount { 0u };
            std::vector<VkExtensionProperties> extensionProperties;

            VULKAN_CHECK(
                vkEnumerateDeviceExtensionProperties(
                    physicalDevice,
                    nullptr,
                    &propertyCount,
                    nullptr
                )
            );

            if (propertyCount > 0u) {
                extensionProperties.resize(propertyCount);
                VULKAN_CHECK(
                    vkEnumerateDeviceExtensionProperties(
                        physicalDevice,
                        nullptr,
                        &propertyCount,
                        extensionProperties.data()
                    )
                );

                for (const std::string& extensionName : physicalDeviceRequirements.deviceExtensionNames) {
                    bool found { false };
                    for (const VkExtensionProperties& extensionProperty : extensionProperties) {
                        if (extensionName == extensionProperty.extensionName) {
                            found = true;
                            break;
                        }
                    }

                    if (!found) {
                        core::Logger::info("Required extension not found: " + extensionName + ", skipping device...");
                        return false;
                    }
                }
            }
        }

        if (physicalDeviceRequirements.samplerAnisotrophy && !physicalDeviceFeatures.samplerAnisotropy) {
            core::Logger::info("Device does not support sampler anisotrophy, skipping...");
            return false;
        }

        return true;
    }

    return false;
}

} // namespace vulkan
} // namespace renderer
} // namespace beige
