cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 17)

set(SRC
    src/main.cpp
)

include_directories(
    ${PROJECT_SOURCE_DIR}/engine/src
)
add_executable(testbed ${SRC})
target_link_libraries(testbed engine)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
