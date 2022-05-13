cmake_minimum_required (VERSION 3.8)
project(beige)

set(CMAKE_CXX_STANDARD 17)

set(ENGINE_SRC
    engine/Beige.cpp
)

set(TESTBED_SRC
    testbed/main.cpp
)

include_directories(${PROJECT_SOURCE_DIR}/engine)
add_definitions(-DBEIGE_EXPORT)
add_library(${PROJECT_NAME}-engine ${ENGINE_SRC})

add_executable(${PROJECT_NAME}-testbed ${TESTBED_SRC})
target_link_libraries(${PROJECT_NAME}-testbed ${PROJECT_NAME}-engine)
