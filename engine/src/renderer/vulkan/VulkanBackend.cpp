#include "VulkanBackend.hpp"

#include "../../core/Logger.hpp"
#include "VulkanDefines.hpp"
#include "VulkanUtils.hpp"
#include "../../math/MathTypes.hpp"

#include <algorithm>
#include <array>
#include <limits>

namespace beige {
namespace renderer {
namespace vulkan {

Backend::Backend(
    const std::string& appName,
    const uint32_t width,
    const uint32_t height,
    std::shared_ptr<platform::Platform> platform
) :
IBackend { },
m_platform { platform },
m_frameDeltaTime { 0.0f },
m_framebufferWidth { width },
m_framebufferHeight { height },
m_framebufferSizeGeneration { 0u },
m_framebufferSizeLastGeneration { 0u },
m_allocationCallbacks { nullptr }, // TODO: Custom allocator
m_instance { 0 },

#if defined(BEIGE_DEBUG)
m_debugUtilsMessenger { 0 },
#endif // BEIGE_DEBUG

m_surface { nullptr },
m_device { nullptr },
m_swapchain { nullptr },
m_recreatingSwapchain { false },
m_mainRenderPass { nullptr },
m_objectVertexBuffer { nullptr },
m_objectIndexBuffer { nullptr },
m_imageIndex { 0u },
m_graphicsCommandBuffers { },
m_materialShader { nullptr },
m_imageAvailableSemaphores { },
m_queueCompleteSemaphores { },
m_inFlightFences { },
m_imagesInFlight { },
m_geometryVertexOffset { 0u },
m_geometryIndexOffset { 0u } {
    const VkApplicationInfo applicationInfo {
        VK_STRUCTURE_TYPE_APPLICATION_INFO, // sType
        nullptr,                            // pNext
        appName.c_str(),                    // pApplicationName
        VK_MAKE_VERSION(1, 0, 0),           // applicationVersion
        "Beige",                            // pEngineName
        VK_MAKE_VERSION(1, 0, 0),           // engineVersion
        VK_API_VERSION_1_2                  // apiVersion
    };

    VkInstanceCreateInfo instanceCreateInfo {
        VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, // sType
        nullptr,                                // pNext
        0u,                                     // flags
        &applicationInfo,                       // pApplicationInfo
        0u,                                     // enabledLayerCount
        nullptr,                                // ppEnabledLayerNames
        0u,                                     // enabledExtensionCount
        nullptr                                 // ppEnabledExtensionNames
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

    const VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo {
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
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: [[fallthrough]];
            default:
                core::Logger::error(pCallbackData->pMessage);
                break;
            }

            return VK_FALSE;
        }, // pfnUserCallback
        nullptr // pUserData
    };

    const PFN_vkCreateDebugUtilsMessengerEXT createDebugUtilsMessengerCallback {
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

    m_surface = std::make_shared<Surface>(
        m_allocationCallbacks,
        m_instance,
        m_platform
    );

    m_device = std::make_shared<Device>(
        m_allocationCallbacks,
        m_instance,
        m_surface
    );

    m_swapchain = std::make_shared<Swapchain>(
        100u,
        100u,
        m_allocationCallbacks,
        m_surface,
        m_device
    );

    m_mainRenderPass = std::make_shared<RenderPass>(
        m_allocationCallbacks,                   // allocationCallbacks
        m_swapchain,                             // swapchain
        m_device,                                // device
        0.0f,                                    // x
        0.0f,                                    // y
        static_cast<float>(m_framebufferWidth),  // w
        static_cast<float>(m_framebufferHeight), // h
        0.0f,                                    // r
        0.0f,                                    // g
        0.2f,                                    // b
        1.0f,                                    // a
        1.0f,                                    // depth
        0u                                       // stencil
    );

    regenerateFramebuffers();
    createCommandBuffers();

    const VkDevice logicalDevice { m_device->getLogicalDevice() };

    const VkSemaphoreCreateInfo semaphoreCreateInfo {
        VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, // sType
        nullptr,                                 // pNext
        0u                                       // flags
    };

    const uint32_t maxFramesInFlight { m_swapchain->getMaxFramesInFlight() };

    m_imageAvailableSemaphores.resize(maxFramesInFlight);
    std::for_each(
        m_imageAvailableSemaphores.begin(),
        m_imageAvailableSemaphores.end(),
        [&](VkSemaphore& semaphore) -> void {
            vkCreateSemaphore(
                logicalDevice,
                &semaphoreCreateInfo,
                m_allocationCallbacks,
                &semaphore
            );
        }
    );

    m_queueCompleteSemaphores.resize(maxFramesInFlight);
    std::for_each(
        m_queueCompleteSemaphores.begin(),
        m_queueCompleteSemaphores.end(),
        [&](VkSemaphore& semaphore) -> void {
            vkCreateSemaphore(
                logicalDevice,
                &semaphoreCreateInfo,
                m_allocationCallbacks,
                &semaphore
            );
        }
    );

    // Create fence in a signal state, idicating that the first frame has already been rendered
    m_inFlightFences.resize(maxFramesInFlight);
    std::for_each(
        m_inFlightFences.begin(),
        m_inFlightFences.end(),
        [&](std::shared_ptr<Fence>& fence) {
            fence = std::make_shared<Fence>(m_allocationCallbacks, m_device, true);
        }
    );

    // In-flight images should not yet exist at this point, so clear the list
    // These are stored in pointers because the initial state should be 0, and will be 0 when not in use
    // Actual fences are not owned by this list
    m_imagesInFlight.resize(static_cast<uint32_t>(m_swapchain->getImages().size()), nullptr);

    m_materialShader = std::make_shared<MaterialShader>(
        m_allocationCallbacks,
        m_device,
        m_mainRenderPass,
        m_swapchain,
        m_framebufferWidth,
        m_framebufferHeight
    );

    createBuffers();

    const float f { 10.0f };

    // TODO: Temporary test code
    std::array<math::Vertex3D, 4u> verts { };

    verts.at(0u).position = glm::vec3(-0.5f * f, -0.5f * f, 0.0f);
    verts.at(0u).texCoord = glm::vec2(0.0f, 0.0f);

    verts.at(1u).position = glm::vec3(0.5f * f, 0.5f * f, 0.0f);
    verts.at(1u).texCoord = glm::vec2(1.0f, 1.0f);

    verts.at(2u).position = glm::vec3(-0.5f * f, 0.5f * f, 0.0f);
    verts.at(2u).texCoord = glm::vec2(0.0f, 1.0f);

    verts.at(3u).position = glm::vec3(0.5f * f, -0.5f * f, 0.0f);
    verts.at(3u).texCoord = glm::vec2(1.0f, 0.0f);

    std::array<uint32_t, 6u> indices {
        0u, 1u, 2u, 0u, 3u, 1u
    };

    uploadDataRange(
        m_device->getGraphicsCommandPool(),
        VK_NULL_HANDLE,
        m_device->getGraphicsQueue(),
        m_objectVertexBuffer->getHandle(),
        0u,
        static_cast<uint64_t>(sizeof(math::Vertex3D) * verts.size()),
        verts.data()
    );

    uploadDataRange(
        m_device->getGraphicsCommandPool(),
        VK_NULL_HANDLE,
        m_device->getGraphicsQueue(),
        m_objectIndexBuffer->getHandle(),
        0u,
        sizeof(uint32_t) * indices.size(),
        indices.data()
    );

    const std::optional<resources::ObjectId> objectId {
        m_materialShader->acquireResources()
    };

    if (!objectId.has_value()) {
        const std::string message{ "Failed to acquire shader resources!" };
        throw std::exception(message.c_str());
    }

    // TODO: End temporary test code

    core::Logger::info("Vulkan renderer initialized successfully!");
}

Backend::~Backend() {
    const VkDevice logicalDevice { m_device->getLogicalDevice() };
    const VkCommandPool graphicsCommandPool { m_device->getGraphicsCommandPool() };

    vkDeviceWaitIdle(logicalDevice);

    m_objectIndexBuffer.reset();
    m_objectVertexBuffer.reset();

    core::Logger::info("Destroying material shader...");
    m_materialShader.reset();

    core::Logger::info("Destroying images in-flight...");
    m_imagesInFlight.clear();

    core::Logger::info("Destroying in-flight fences...");
    m_inFlightFences.clear();

    core::Logger::info("Destroying queue complete semaphores...");
    std::for_each(
        m_queueCompleteSemaphores.begin(),
        m_queueCompleteSemaphores.end(),
        [&](const VkSemaphore& semaphore) -> void {
            vkDestroySemaphore(
                logicalDevice,
                semaphore,
                m_allocationCallbacks
            );
        }
    );
    m_queueCompleteSemaphores.clear();

    core::Logger::info("Destroying image available semaphores...");
    std::for_each(
        m_imageAvailableSemaphores.begin(),
        m_imageAvailableSemaphores.end(),
        [&](const VkSemaphore& semaphore) -> void {
            vkDestroySemaphore(
                logicalDevice,
                semaphore,
                m_allocationCallbacks
            );
        }
    );
    m_imageAvailableSemaphores.clear();

    core::Logger::info("Destroying graphics command buffers...");
    std::for_each(
        m_graphicsCommandBuffers.begin(),
        m_graphicsCommandBuffers.end(),
        [&graphicsCommandPool](
            const std::shared_ptr<CommandBuffer> graphicsCommandBuffer
        ) -> void {
            graphicsCommandBuffer->free(graphicsCommandPool);
        }
    );
    m_graphicsCommandBuffers.clear();

    core::Logger::info("Destroying framebuffers...");
    m_framebuffers.clear();

    core::Logger::info("Destroying Vulkan render pass...");
    m_mainRenderPass.reset();

    core::Logger::info("Destroying Vulkan swapchain...");
    m_swapchain.reset();

    core::Logger::info("Destroying Vulkan device...");
    m_device.reset();

    core::Logger::info("Destroying Vulkan surface...");
    m_surface.reset();

#ifdef BEIGE_DEBUG
    core::Logger::debug("Destroying Vulkan debugger...");
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

    core::Logger::info("Destroying Vulkan instance...");
    vkDestroyInstance(m_instance, m_allocationCallbacks);
}

auto Backend::onResized(const uint16_t width, const uint16_t height) -> void {
    // Update the framebuffer generation, a counter which indicates when the framebuffer size has been updated
    m_framebufferWidth = width;
    m_framebufferHeight = height;
    m_framebufferSizeGeneration++;

    core::Logger::info(
        "VulkanBackend::onResized - width: " + std::to_string(m_framebufferWidth) +
        ", height: " + std::to_string(m_framebufferHeight) +
        ", generation: " + std::to_string(m_framebufferSizeGeneration)
    );
}

auto Backend::beginFrame(const float deltaTime) -> bool {
    m_frameDeltaTime = deltaTime;
    const VkDevice logicalDevice { m_device->getLogicalDevice() };

    // Check if recreating swapchain and boot out
    if (m_recreatingSwapchain) {
        const VkResult result { vkDeviceWaitIdle(logicalDevice) };
        if (!Utils::isResultSuccess(result)) {
            core::Logger::error("VulkanBackend::beginFrame - vkDeviceWaitIdle failed (1): " + Utils::resultToString(result, true));
            return false;
        }

        core::Logger::info("Recreating swapchain, booting...");
        return false;
    }

    // Check if the framebuffer has been resized, if so, a new swapchain must be created
    if (m_framebufferSizeGeneration != m_framebufferSizeLastGeneration) {
        const VkResult result { vkDeviceWaitIdle(logicalDevice) };
        if (!Utils::isResultSuccess(result)) {
            core::Logger::error("VulkanBackend::beginFrame - vkDeviceWaitIdle failed (2): " + Utils::resultToString(result, true));
            return false;
        }

        // If the swapchain recreation failed (because, for example, the window was minimized), boot out before unsetting the flag
        if (!recreateSwapchain()) {
            return false;
        }

        core::Logger::info("Resized, booting...");
        return false;
    }

    const uint32_t currentFrame { m_swapchain->getCurrentFrame() };

    // Wait for the execution of the current frame to complete.
    // The fence being free will allow this on to move on.
    if (!m_inFlightFences.at(currentFrame)->wait(UINT64_MAX)) {
        core::Logger::warn("In-flight fence wait failure!");
        return false;
    }

    // Acquire the next image from the swapchain. Pass along the semaphore that should signaled when this completes.
    // This same semaphore will later be waited on by the queue submission to ensure this image is available.
    const std::optional<uint32_t> imageIndex {
        m_swapchain->acquireNextImageIndex(
            m_framebufferWidth,
            m_framebufferHeight,
            UINT64_MAX,
            m_imageAvailableSemaphores.at(currentFrame),
            VK_NULL_HANDLE
        )
    };

    if (!imageIndex.has_value()) {
        return false;
    }

    m_imageIndex = imageIndex.value();

    // Begin recording commands.
    std::shared_ptr<CommandBuffer> graphicsCommandBuffer { m_graphicsCommandBuffers.at(m_imageIndex) };
    graphicsCommandBuffer->reset();
    graphicsCommandBuffer->begin(false, false, false);

    // Dynamic state.
    const VkViewport viewport {
        0.0f,                                     // x
        static_cast<float>(m_framebufferHeight),  // y
        static_cast<float>(m_framebufferWidth),   // width
        -static_cast<float>(m_framebufferHeight), // height
        0.0f,                                     // minDepth
        1.0f                                      // maxDepth
    };

    // Scissor.
    const VkOffset2D scissorOffset {
        0, // x
        0  // y
    };

    const VkExtent2D scissorExtent {
        m_framebufferWidth, // width
        m_framebufferHeight // height
    };

    const VkRect2D scissor {
        scissorOffset, // offset
        scissorExtent  // extent
    };

    const VkCommandBuffer graphicsCommandBufferHandle { graphicsCommandBuffer->getHandle() };

    vkCmdSetViewport(graphicsCommandBufferHandle, 0u, 1u, &viewport);
    vkCmdSetScissor(graphicsCommandBufferHandle, 0u, 1u, &scissor);

    m_mainRenderPass->setW(static_cast<float>(m_framebufferWidth));
    m_mainRenderPass->setH(static_cast<float>(m_framebufferHeight));

    m_mainRenderPass->begin(
        graphicsCommandBuffer,
        m_framebuffers.at(m_imageIndex)->getFramebuffer()
    );

    return true;
}

auto Backend::updateGlobalState(
    const glm::mat4x4& projection,
    const glm::mat4x4& view,
    const glm::vec3& viewPosition,
    const glm::vec4& ambientColor,
    const int32_t mode
) -> void {
    const VkCommandBuffer graphicsCommandBufferHandle { m_graphicsCommandBuffers.at(m_imageIndex)->getHandle() };

    m_materialShader->use(graphicsCommandBufferHandle);
    m_materialShader->setProjection(projection);
    m_materialShader->setView(view);

    // TODO: Other uniform object properties.

    m_materialShader->updateGlobalState(m_imageIndex, graphicsCommandBufferHandle, m_frameDeltaTime);
}

auto Backend::endFrame(const float deltaTime) -> bool {
    const std::shared_ptr<CommandBuffer> graphicsCommandBuffer { m_graphicsCommandBuffers.at(m_imageIndex) };

    // End render pass.
    m_mainRenderPass->end(graphicsCommandBuffer);

    graphicsCommandBuffer->end();

    // Make sure the previous frame is not using this image.
    const std::shared_ptr<Fence> fence { m_imagesInFlight.at(m_imageIndex) };

    if (fence != nullptr) {
        fence->wait(UINT64_MAX);
    }

    // Mark the image fence as in-use by this frame.
    const uint32_t currentFrame { m_swapchain->getCurrentFrame() };
    const std::shared_ptr<Fence> currentInFlightFence { m_inFlightFences.at(currentFrame) };
    m_imagesInFlight.at(m_imageIndex) = currentInFlightFence;

    // Reset the fence for use on the next frame.
    currentInFlightFence->reset();

    // Submit the queue and wait for the operation to complete. Being queue submission.
    const VkPipelineStageFlags pipelineStageFlags {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    };

    const VkSemaphore currentQueueCompleteSemaphore { m_queueCompleteSemaphores.at(currentFrame) };

    const VkSubmitInfo submitInfo {
        VK_STRUCTURE_TYPE_SUBMIT_INFO,                // sType
        nullptr,                                      // pNext
        1u,                                           // waitSemaphoreCount
        &m_imageAvailableSemaphores.at(currentFrame), // pWaitSemaphores
        &pipelineStageFlags,                          // pWaitDstStageMask
        1u,                                           // commandBufferCount
        &graphicsCommandBuffer->getHandle(),          // pCommandBuffers
        1u,                                           // signalSemaphoreCount
        &currentQueueCompleteSemaphore                // pSignalSemaphores
    };

    const VkQueue graphicsQueue { m_device->getGraphicsQueue() };
    const VkQueue presentQueue { m_device->getPresentQueue() };

    const VkResult result {
        vkQueueSubmit(
            graphicsQueue,
            1u,
            &submitInfo,
            currentInFlightFence->getFence()
        )
    };

    if (result != VK_SUCCESS) {
        core::Logger::error("vkQueueSubmit failed with result " + std::to_string(static_cast<uint32_t>(result)));
        return false;
    }

    graphicsCommandBuffer->updateSubmitted();
    // End queue submission.

    // Give the image back to the swapchain.
    m_swapchain->present(
        m_framebufferWidth,
        m_framebufferHeight,
        graphicsQueue,
        presentQueue,
        currentQueueCompleteSemaphore,
        m_imageIndex
    );

    return true;
}

auto Backend::updateObject(
    const GeometryRenderData& geometryRenderData
) -> void {
    m_materialShader->updateObject(
        m_graphicsCommandBuffers.at(m_imageIndex)->getHandle(),
        m_imageIndex,
        geometryRenderData,
        m_frameDeltaTime
    );

    // TODO: Temporary test code
    const VkCommandBuffer graphicsCommandBufferHandle { m_graphicsCommandBuffers.at(m_imageIndex)->getHandle() };

    m_materialShader->use(graphicsCommandBufferHandle);

    // Bind vertex buffer and offset.
    const std::array<VkDeviceSize, 1u> offsets{ 0u };
    vkCmdBindVertexBuffers(graphicsCommandBufferHandle, 0u, 1u, &m_objectVertexBuffer->getHandle(), offsets.data());

    // Bind index buffer and offset.
    vkCmdBindIndexBuffer(graphicsCommandBufferHandle, m_objectIndexBuffer->getHandle(), 0u, VK_INDEX_TYPE_UINT32);

    // Issue the draw.
    vkCmdDrawIndexed(graphicsCommandBufferHandle, 6u, 1u, 0u, 0u, 0u);
    // TODO: End temporary test code
}

auto Backend::createTexture(
    const std::string& name,
    const int32_t width,
    const int32_t height,
    const int32_t channelCount,
    const void* pixels,
    const bool hasTransparency
) -> std::shared_ptr<resources::ITexture> {
    return std::make_shared<Texture>(
        name,
        width,
        height,
        channelCount,
        pixels,
        hasTransparency,
        m_allocationCallbacks,
        m_device
    );
}

auto Backend::regenerateFramebuffers() -> void {
    const std::vector<VkImageView> swapchainImageViews { m_swapchain->getImageViews() };
    const std::shared_ptr<Image> swapchainDepthAttachment { m_swapchain->getDepthAttachment() };

    m_framebuffers.clear();

    std::for_each(
        swapchainImageViews.begin(),
        swapchainImageViews.end(),
        [&](const VkImageView& imageView) -> void {
            // TODO: Make this dynamic based on the currently configured attachments
            const std::vector<VkImageView> imageViews {
                imageView,
                swapchainDepthAttachment->getImageView()
            };

            m_framebuffers.push_back(
                std::make_shared<Framebuffer>(
                    m_allocationCallbacks,
                    m_device,
                    m_mainRenderPass,
                    m_framebufferWidth,
                    m_framebufferHeight,
                    imageViews
                )
            );
        }
    );
}

auto Backend::createCommandBuffers() -> void {
    const VkCommandPool graphicsCommandPool { m_device->getGraphicsCommandPool() };

    const uint32_t imagesCount { static_cast<uint32_t>(m_swapchain->getImages().size()) };

    if (m_graphicsCommandBuffers.empty()) {
        m_graphicsCommandBuffers.resize(imagesCount);
    }

    for (uint32_t i { 0u }; i < imagesCount; i++) {
        if (m_graphicsCommandBuffers.at(i) != nullptr && m_graphicsCommandBuffers.at(i)->getHandle() != VK_NULL_HANDLE) {
            m_graphicsCommandBuffers.at(i)->free(graphicsCommandPool);
        }

        m_graphicsCommandBuffers.at(i).reset();
        m_graphicsCommandBuffers.at(i) = std::make_shared<CommandBuffer>(m_device);
        m_graphicsCommandBuffers.at(i)->allocate(graphicsCommandPool, true);
    }

    core::Logger::info("Vulkan graphics command buffers created!");
}

auto Backend::recreateSwapchain() -> bool {
    if (m_recreatingSwapchain) {
        core::Logger::debug("VulkanBackend::recreateSwapchain - called when already recreating, booting...");
        return false;
    }

    if (m_framebufferWidth == 0u || m_framebufferHeight == 0u) {
        core::Logger::debug("VulkanBackend::recreateSwapchain - called when window less than 1 in a dimension, booting...");
        return false;
    }

    // Mark as recreating if the dimensions are valid
    m_recreatingSwapchain = true;

    // Wait for any operations to complete
    const VkDevice logicalDevice { m_device->getLogicalDevice() };
    vkDeviceWaitIdle(logicalDevice);

    // Clear these out just in case
    std::for_each(
        m_imagesInFlight.begin(),
        m_imagesInFlight.end(),
        [](std::shared_ptr<Fence> fence) -> void {
            fence.reset();
            fence = nullptr;
        }
    );

    m_device->querySwapchainSupport(m_device->getPhysicalDevice());
    m_device->detectDepthFormat();
    m_swapchain->recreate(m_framebufferWidth, m_framebufferHeight);

    // Synchronize the framebuffer size with the cached sizes
    m_mainRenderPass->setW(static_cast<float>(m_framebufferWidth));
    m_mainRenderPass->setH(static_cast<float>(m_framebufferHeight));

    // Update framebuffer size generation
    m_framebufferSizeLastGeneration = m_framebufferSizeGeneration;

    // Clean swapchain
    const VkCommandPool graphicsCommandPool { m_device->getGraphicsCommandPool() };
    std::for_each(
        m_graphicsCommandBuffers.begin(),
        m_graphicsCommandBuffers.end(),
        [&graphicsCommandPool](const std::shared_ptr<CommandBuffer> commandBuffer) -> void {
            commandBuffer->free(graphicsCommandPool);
        }
    );

    m_mainRenderPass->setX(0.0f);
    m_mainRenderPass->setY(0.0f);
    m_mainRenderPass->setW(static_cast<float>(m_framebufferWidth));
    m_mainRenderPass->setH(static_cast<float>(m_framebufferHeight));

    regenerateFramebuffers();
    createCommandBuffers();

    // Clear the recreating flag
    m_recreatingSwapchain = false;

    return true;
}

auto Backend::createBuffers() -> void {
    const VkMemoryPropertyFlags memoryPropertyFlags {
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    };

    const VkBufferUsageFlags vertexBufferUsageFlags {
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
        VK_BUFFER_USAGE_TRANSFER_DST_BIT |
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT
    };
    const uint64_t vertexBufferSize { sizeof(glm::vec3) * 1024u * 1024u };
    m_objectVertexBuffer = std::make_unique<Buffer>(
        m_allocationCallbacks,
        m_device,
        vertexBufferSize,
        vertexBufferUsageFlags,
        memoryPropertyFlags,
        true
    );
    m_geometryVertexOffset = 0u;

    const VkBufferUsageFlags indexBufferUsageFlags {
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
        VK_BUFFER_USAGE_TRANSFER_DST_BIT |
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT
    };
    const uint64_t indexBufferSize { sizeof(uint32_t) * 1024u * 1024u };
    m_objectIndexBuffer = std::make_unique<Buffer>(
        m_allocationCallbacks,
        m_device,
        indexBufferSize,
        indexBufferUsageFlags,
        memoryPropertyFlags,
        true
    );
    m_geometryIndexOffset = 0u;
}

auto Backend::uploadDataRange(
    const VkCommandPool& commandPool,
    const VkFence& fence,
    const VkQueue& queue,
    const VkBuffer& buffer,
    const uint64_t offset,
    const uint64_t size,
    void* data
) -> void {
    const VkMemoryPropertyFlags memoryPropertyFlags {
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    };

    const VkBufferUsageFlags bufferUsageFlags {
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT
    };

    Buffer staging {
        m_allocationCallbacks,
        m_device,
        size,
        bufferUsageFlags,
        memoryPropertyFlags,
        true
    };

    staging.loadData(0u, size, 0u, data);
    staging.copyTo(commandPool, fence, queue, staging.getHandle(), 0u, buffer, offset, size);
}

} // namespace vulkan
} // namespace renderer
} // namespace beige
