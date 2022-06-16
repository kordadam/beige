#include "VulkanBackend.hpp"

#include "../../core/Logger.hpp"
#include "VulkanDefines.hpp"

namespace beige {
namespace renderer {
namespace vulkan {

VulkanBackend::VulkanBackend(
    const std::string& appName,
    std::shared_ptr<platform::Platform> platform
) :
IRendererBackend { appName, platform },
m_framebufferWidth { 0u },
m_framebufferHeight { 0u },
m_allocationCallbacks { nullptr },
m_instance { 0 },
m_surface { 0 },

#if defined(BEIGE_DEBUG)
m_debugUtilsMessenger { 0 },
#endif // BEIGE_DEBUG

m_device { nullptr },
m_swapchain { nullptr } {
    // TODO: Custom allocator
    m_allocationCallbacks = nullptr;

    VkApplicationInfo applicationInfo {
        VK_STRUCTURE_TYPE_APPLICATION_INFO, // sType
        nullptr, // pNext
        appName.c_str(), // pApplicationName
        VK_MAKE_VERSION(1, 0, 0), // applicationVersion
        "Beige", // pEngineName
        VK_MAKE_VERSION(1, 0, 0), // engineVersion
        VK_API_VERSION_1_2 // apiVersion
    };

    VkInstanceCreateInfo instanceCreateInfo {
        VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, // sType
        nullptr, // pNext
        0u, // flags
        &applicationInfo, // pApplicationInfo
        0u, // enabledLayerCount
        nullptr, // ppEnabledLayerNames
        0u, // enabledExtensionCount
        nullptr // ppEnabledExtensionNames
    };

    std::vector<const char*> requiredValidationLayerNames;
#if defined(BEIGE_DEBUG)
    core::Logger::info("Validation layers enabled, enumerating...");

    requiredValidationLayerNames.push_back("VK_LAYER_KHRONOS_validation");

    uint32_t propertyCount { 0u };
    VULKAN_CHECK(vkEnumerateInstanceLayerProperties(&propertyCount, nullptr));
    std::vector<VkLayerProperties> layerProperties(propertyCount);
    VULKAN_CHECK(vkEnumerateInstanceLayerProperties(&propertyCount, layerProperties.data()));

    for (const char* requiredValidationLayerName : requiredValidationLayerNames) {
        core::Logger::info("Searching for layer: " + std::string(requiredValidationLayerName) + "...");
        bool found { false };
        
        for (const VkLayerProperties& layerProperty : layerProperties) {
            if (std::string(requiredValidationLayerName) == std::string(layerProperty.layerName)) {
                found = true;
                core::Logger::info("Found!");
                break;
            }
        }

        if (!found) {
            const std::string message { "Required validation layer is missing: " + std::string(requiredValidationLayerName) + "!" };
            throw std::exception(message.c_str());
        }
    }

    core::Logger::info("All required validation layers are present!");
#endif // BEIGE_DEBUG

    instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(requiredValidationLayerNames.size());
    instanceCreateInfo.ppEnabledLayerNames = requiredValidationLayerNames.data();

    std::vector<const char*> requiredExtensions { m_platform->getVulkanRequiredExtensionNames() };
    requiredExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

#ifdef BEIGE_DEBUG
    requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif // BEIGE_DEBUG

    core::Logger::info("Required extensions:");
    for (const char* requiredExtension : requiredExtensions) {
        core::Logger::info(requiredExtension);
    }

    instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
    instanceCreateInfo.ppEnabledExtensionNames = requiredExtensions.data();

    VULKAN_CHECK(vkCreateInstance(&instanceCreateInfo, m_allocationCallbacks, &m_instance));
    core::Logger::info("Vulkan instance created!");

#ifdef BEIGE_DEBUG
    core::Logger::debug("Creating Vulkan debugger...");

    const VkDebugUtilsMessageSeverityFlagsEXT debugUtilsMessageSeverityFlags {
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
    };

    const VkDebugUtilsMessageTypeFlagsEXT debugUtilsMessageTypeFlags {
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
    };

    VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo {
        VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT, // sType
        nullptr, // pNext
        0u, // flags
        debugUtilsMessageSeverityFlags, // messageSeverity
        debugUtilsMessageTypeFlags, // messageType
        [](
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageTypes,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData
        ) -> VkBool32 {
            switch (messageSeverity) {
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
                core::Logger::trace(pCallbackData->pMessage);
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
                core::Logger::info(pCallbackData->pMessage);
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
                core::Logger::warn(pCallbackData->pMessage);
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            default:
                core::Logger::error(pCallbackData->pMessage);
                break;
            }

            return VK_FALSE;
        }, // pfnUserCallback
        nullptr // pUserData
    };

    PFN_vkCreateDebugUtilsMessengerEXT createDebugUtilsMessengerCallback {
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance, "vkCreateDebugUtilsMessengerEXT")
    };

    ASSERT_MESSAGE(
        createDebugUtilsMessengerCallback != nullptr,
        "Failed to create debug messenger!"
    );

    VULKAN_CHECK(
        createDebugUtilsMessengerCallback(
            m_instance,
            &debugUtilsMessengerCreateInfo,
            m_allocationCallbacks,
            &m_debugUtilsMessenger
        )
    );

    core::Logger::debug("Vulkan debugger created!");
#endif // BEIGE_DEBUG

    core::Logger::info("Creating Vulkan surface...");

    std::optional<VkSurfaceKHR> surface {
        m_platform->createVulkanSurface(m_instance, m_allocationCallbacks)
    };

    if (surface.has_value()) {
        m_surface = surface.value();
    } else {
        throw std::exception("Failed to create platform surface!");
    }

    core::Logger::info("Vulkan surface created!");

    m_device = std::make_shared<VulkanDevice>(
        m_allocationCallbacks,
        m_instance,
        m_surface
    );

    m_swapchain = std::make_shared<VulkanSwapchain>(
        100u,
        100u,
        m_surface,
        m_allocationCallbacks,
        m_device
    );

    core::Logger::info("Vulkan renderer initialized successfully!");
}

VulkanBackend::~VulkanBackend() {
    m_swapchain.reset();
    m_device.reset();

    core::Logger::info("Vulkan destroying surface...");
    vkDestroySurfaceKHR(m_instance, m_surface, m_allocationCallbacks);

#ifdef BEIGE_DEBUG
    core::Logger::debug("Vulkan destroying debugger...");
    if (m_debugUtilsMessenger != 0) {
        PFN_vkDestroyDebugUtilsMessengerEXT destroyDebugUtilsMessengerCallback {
            (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT")
        };
        destroyDebugUtilsMessengerCallback(
            m_instance,
            m_debugUtilsMessenger,
            m_allocationCallbacks
        );
    }
#endif // BEIGE_DEBUG

    core::Logger::info("Vulkan destroying instance...");
    vkDestroyInstance(m_instance, m_allocationCallbacks);
}

auto VulkanBackend::onResized(const uint16_t width, const uint16_t height) -> void {

}

auto VulkanBackend::beginFrame(const float deltaTime) -> bool {
    return true;
}

auto VulkanBackend::endFrame(const float deltaTime) -> bool {
    return true;
}

auto VulkanBackend::drawFrame(const Packet& packet) -> bool {
    return true;
}

} // namespace vulkan
} // namespace renderer
} // namespace beige
