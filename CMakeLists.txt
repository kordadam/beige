cmake_minimum_required (VERSION 3.8)
project(beige)

set(CMAKE_CXX_STANDARD 17)

set(ENGINE_SRC
    engine/Beige.cpp
)

set(TESTBED_SRC
    testbed/main.cpp
)

if(WIN32)
    find_path(VULKAN_INCLUDE_DIR
        NAMES vulkan/vulkan.hpp
        PATHS
            "$ENV{VULKAN_SDK}/Include"
    )

    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        find_library(VULKAN_LIBRARY
            NAMES vulkan-1
            PATHS
                "$ENV{VULKAN_SDK}/Lib"
                "$ENV{VULKAN_SDK}/Bin"
        )
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
        find_library(VULKAN_LIBRARY
            NAMES vulkan-1
            PATHS
                "$ENV{VULKAN_SDK}/Lib32"
                "$ENV{VULKAN_SDK}/Bin32"
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
        IMPORTED_LOCATION "${VULKAN_LIBRARIES}"
        INTERFACE_INCLUDE_DIRECTORIES "${VULKAN_INCLUDE_DIRS}"
    )
endif()

include_directories(
    ${PROJECT_SOURCE_DIR}/engine
    ${VULKAN_INCLUDE_DIRS}
)
add_definitions(-DBEIGE_EXPORT)
add_library(${PROJECT_NAME}-engine ${ENGINE_SRC})
target_link_libraries(${PROJECT_NAME}-engine ${VULKAN_LIBRARIES})

add_executable(${PROJECT_NAME}-testbed ${TESTBED_SRC})
target_link_libraries(${PROJECT_NAME}-testbed ${PROJECT_NAME}-engine)