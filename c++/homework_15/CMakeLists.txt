include(FetchContent)

FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG        v0.15.3
)
FetchContent_MakeAvailable(httplib)

add_executable(hw15_poster src/main.cpp)
target_link_libraries(hw15_poster PRIVATE httplib::httplib)
target_compile_options(hw15_poster PRIVATE -Wall -Wextra -pedantic)

add_custom_command(TARGET hw15_poster POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/data
        ${CMAKE_CURRENT_BINARY_DIR}/data
)
