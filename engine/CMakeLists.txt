cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 17)

set(SRC
    src/core/App.cpp
    src/core/App.hpp
    src/core/AppTypes.hpp
    src/core/Assertions.hpp
    src/core/Clock.cpp
    src/core/Clock.hpp
    src/core/Event.hpp
    src/core/Input.cpp
    src/core/Input.hpp
    src/core/InputTypes.hpp
    src/core/Logger.cpp
    src/core/Logger.hpp
    src/math/Math.hpp
    src/math/MathConstans.hpp
    src/math/MathMatrix4x4.cpp
    src/math/MathMatrix4x4.hpp
    src/math/MathQuaternion.cpp
    src/math/MathQuaternion.hpp
    src/math/MathRandom.cpp
    src/math/MathRandom.hpp
    src/math/MathVector2.cpp
    src/math/MathVector2.hpp
    src/math/MathVector3.cpp
    src/math/MathVector3.hpp
    src/math/MathVector4.cpp
    src/math/MathVector4.hpp
    src/platform/Platform.hpp
    src/platform/PlatformTypes.hpp
    src/platform/PlatformWin32.cpp
    src/renderer/vulkan/shaders/VulkanShaderObject.cpp
    src/renderer/vulkan/shaders/VulkanShaderObject.hpp
    src/renderer/vulkan/VulkanBackend.cpp
    src/renderer/vulkan/VulkanBackend.hpp
    src/renderer/vulkan/VulkanBuffer.cpp
    src/renderer/vulkan/VulkanBuffer.hpp
    src/renderer/vulkan/VulkanCommandBuffer.cpp
    src/renderer/vulkan/VulkanCommandBuffer.hpp
    src/renderer/vulkan/VulkanDefines.hpp
    src/renderer/vulkan/VulkanDevice.cpp
    src/renderer/vulkan/VulkanDevice.hpp
    src/renderer/vulkan/VulkanFence.cpp
    src/renderer/vulkan/VulkanFence.hpp
    src/renderer/vulkan/VulkanFramebuffer.cpp
    src/renderer/vulkan/VulkanFramebuffer.hpp
    src/renderer/vulkan/VulkanImage.cpp
    src/renderer/vulkan/VulkanImage.hpp
    src/renderer/vulkan/VulkanPipeline.cpp
    src/renderer/vulkan/VulkanPipeline.hpp
    src/renderer/vulkan/VulkanRenderPass.cpp
    src/renderer/vulkan/VulkanRenderPass.hpp
    src/renderer/vulkan/VulkanSurface.cpp
    src/renderer/vulkan/VulkanSurface.hpp
    src/renderer/vulkan/VulkanSwapchain.cpp
    src/renderer/vulkan/VulkanSwapchain.hpp
    src/renderer/vulkan/VulkanUtils.cpp
    src/renderer/vulkan/VulkanUtils.hpp
    src/renderer/IRendererBackend.hpp
    src/renderer/RendererFrontend.cpp
    src/renderer/RendererFrontend.hpp
    src/renderer/RendererTypes.hpp
    src/Defines.hpp
    src/Entry.hpp
    src/IGame.hpp
)

if(WIN32)
    find_path(VULKAN_INCLUDE_DIR
        NAMES
            vulkan/vulkan.hpp
        PATHS
            $ENV{VULKAN_SDK}/Include
    )

    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        find_library(VULKAN_LIBRARY
            NAMES
                vulkan-1
            PATHS
                $ENV{VULKAN_SDK}/Lib
                $ENV{VULKAN_SDK}/Bin
        )
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
        find_library(VULKAN_LIBRARY
            NAMES
                vulkan-1
            PATHS
                $ENV{VULKAN_SDK}/Lib32
                $ENV{VULKAN_SDK}/Bin32
        )
    endif()
endif()

set(VULKAN_LIBRARIES ${VULKAN_LIBRARY})
set(VULKAN_INCLUDE_DIRS ${VULKAN_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Vulkan
    DEFAULT_MSG
    VULKAN_LIBRARY VULKAN_INCLUDE_DIR
)

mark_as_advanced(VULKAN_INCLUDE_DIR VULKAN_LIBRARY)

if(Vulkan_FOUND AND NOT TARGET Vulkan::Vulkan)
    add_library(Vulkan::Vulkan UNKNOWN IMPORTED)
    set_target_properties(Vulkan::Vulkan PROPERTIES
        IMPORTED_LOCATION ${VULKAN_LIBRARIES}
        INTERFACE_INCLUDE_DIRECTORIES ${VULKAN_INCLUDE_DIRS}
    )
endif()

include_directories(
    ${VULKAN_INCLUDE_DIRS}
)
add_definitions(-DBEIGE_EXPORT -DBEIGE_DEBUG)
add_library(engine SHARED ${SRC})
target_link_libraries(engine ${VULKAN_LIBRARIES})
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/build)
