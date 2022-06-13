#include "VulkanDevice.hpp"

#include "VulkanDefines.hpp"

#include <exception>
#include <sstream>
#include <array>

namespace beige {
namespace renderer {
namespace vulkan {

VulkanDevice::VulkanDevice(
    VkAllocationCallbacks* allocationCallbacks,
    const VkInstance& instance,
    const VkSurfaceKHR& surface
) :
m_allocationCallbacks { allocationCallbacks },
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
m_physicalDeviceProperties { 0 },
m_physicalDeviceFeatures { 0 },
m_physicalDeviceMemoryProperties { 0 } {
    if (!selectPhysicalDevice(instance, surface)) {
        throw std::exception("Failed to create device!");
    }

    core::Logger::info("Creating logical device...");
    // Do not create additional queues for shared indices

    std::vector<VkDeviceQueueCreateInfo> deviceQueueCreateInfos;

    std::array<float, 2u> graphicsQueuePriorities { 1.0f, 1.0f };
    VkDeviceQueueCreateInfo graphicsDeviceQueueCreateInfo {
        VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, // sType
        nullptr, // pNext
        0u, // flags
        m_graphicsQueueIndex.value(), // queueFamilyIndex
        static_cast<uint32_t>(graphicsQueuePriorities.size()), // queueCount
        graphicsQueuePriorities.data() // pQueuePriorities
    };
    deviceQueueCreateInfos.push_back(graphicsDeviceQueueCreateInfo);

    if (m_graphicsQueueIndex.value() != m_presentQueueIndex.value()) {
        std::array<float, 1u> presentQueuePriorities { 1.0f };
        VkDeviceQueueCreateInfo presentDeviceQueueCreateInfo {
            VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, // sType
            nullptr, // pNext
            0u, // flags
            m_presentQueueIndex.value(), // queueFamilyIndex
            static_cast<uint32_t>(presentQueuePriorities.size()), // queueCount
            presentQueuePriorities.data() // pQueuePriorities
        };
        deviceQueueCreateInfos.push_back(presentDeviceQueueCreateInfo);
    }

    if (m_graphicsQueueIndex.value() != m_transferQueueIndex.value()) {
        std::array<float, 1u> transferQueuePriorities { 1.0f };
        VkDeviceQueueCreateInfo transferDeviceQueueCreateInfo {
            VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, // sType
            nullptr, // pNext
            0u, // flags
            m_transferQueueIndex.value(), // queueFamilyIndex
            static_cast<uint32_t>(transferQueuePriorities.size()), // queueCount
            transferQueuePriorities.data() // pQueuePriorities
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
}

VulkanDevice::~VulkanDevice() {
    core::Logger::info("Destroying logical device...");

    vkDestroyDevice(
        m_logicalDevice,
        m_allocationCallbacks
    );
}

auto VulkanDevice::selectPhysicalDevice(
    const VkInstance& instance,
    const VkSurfaceKHR& surface
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
                surface,
                physicalDeviceProperties,
                physicalDeviceFeatures,
                physicalDeviceRequirements,
                physicalDeviceQueueFamilies,
                m_swapchainSupport
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
            case VK_PHYSICAL_DEVICE_TYPE_OTHER:
            default:
                core::Logger::info("GPU type is uknown!");
                break;
            }

            std::stringstream driverVersionLog;
            driverVersionLog <<
                "GPU driver version: " <<
                VK_VERSION_MAJOR(physicalDeviceProperties.driverVersion) << "." <<
                VK_VERSION_MINOR(physicalDeviceProperties.driverVersion) << "." <<
                VK_VERSION_PATCH(physicalDeviceProperties.driverVersion) << ".";
            core::Logger::info(driverVersionLog.str());

            std::stringstream vulkanVersionLog;
            vulkanVersionLog <<
                "Vulkan API version: " <<
                VK_VERSION_MAJOR(physicalDeviceProperties.apiVersion) << "." <<
                VK_VERSION_MINOR(physicalDeviceProperties.apiVersion) << "." <<
                VK_VERSION_PATCH(physicalDeviceProperties.apiVersion) << ".";
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

auto VulkanDevice::physicalDeviceMeetsRequirements(
    const VkPhysicalDevice& physicalDevice,
    const VkSurfaceKHR& surface,
    const VkPhysicalDeviceProperties& physicalDeviceProperties,
    const VkPhysicalDeviceFeatures& physicalDeviceFeatures,
    const PhysicalDeviceRequirements& physicalDeviceRequirements,
    PhysicalDeviceQueueFamilies& physicalDeviceQueueFamilies,
    SwapchainSupport& swapchainSupport
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
        deviceQuerySwapchainSupport(physicalDevice, surface, swapchainSupport);

        if (swapchainSupport.surfaceFormats.empty() || swapchainSupport.presentModes.empty()) {
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

auto VulkanDevice::deviceQuerySwapchainSupport(
    const VkPhysicalDevice& physicalDevice,
    const VkSurfaceKHR& surface,
    SwapchainSupport& swapchainSupport
) -> void {
    VULKAN_CHECK(
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
            physicalDevice,
            surface,
            &swapchainSupport.surfaceCapabilities
        )
    );

    uint32_t formatCount { 0u };
    VULKAN_CHECK(
        vkGetPhysicalDeviceSurfaceFormatsKHR(
            physicalDevice,
            surface,
            &formatCount,
            nullptr
        )
    );

    if (formatCount > 0u) {
        swapchainSupport.surfaceFormats.resize(formatCount);
        VULKAN_CHECK(
            vkGetPhysicalDeviceSurfaceFormatsKHR(
                physicalDevice,
                surface,
                &formatCount,
                swapchainSupport.surfaceFormats.data()
            )
        );
    }

    uint32_t presentModeCount { 0u };
    VULKAN_CHECK(
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            physicalDevice,
            surface,
            &presentModeCount,
            nullptr
        )
    );

    if (presentModeCount > 0u) {
        swapchainSupport.presentModes.resize(presentModeCount);
        VULKAN_CHECK(
            vkGetPhysicalDeviceSurfacePresentModesKHR(
                physicalDevice,
                surface,
                &presentModeCount,
                swapchainSupport.presentModes.data()
            )
        );
    }
}

} // namespace vulkan
} // namespace renderer
} // namespace beige
