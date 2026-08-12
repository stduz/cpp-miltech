cmake_minimum_required(VERSION 3.16)
project(homework_17 CXX)
set(CMAKE_CXX_STANDARD 20)

if(NOT UNIX)
    message(STATUS "homework_17: skipping (Linux only)")
    return()
endif()

include(FetchContent)
FetchContent_Declare(
    mavlink_c_library
    GIT_REPOSITORY https://github.com/mavlink/c_library_v2.git
    GIT_TAG        main
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(mavlink_c_library)

add_executable(hw17_drone src/main.cpp)

target_include_directories(hw17_drone SYSTEM PRIVATE
    ${mavlink_c_library_SOURCE_DIR}
)
target_include_directories(hw17_drone PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../homework_11/include
)
target_link_libraries(hw17_drone PRIVATE hw11_nav)
target_compile_options(hw17_drone PRIVATE -Wall -Wextra -pedantic)
