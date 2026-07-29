include(FetchContent)

file(DOWNLOAD
    https://raw.githubusercontent.com/yhirose/cpp-httplib/v0.15.3/httplib.h
    ${CMAKE_CURRENT_BINARY_DIR}/httplib.h
    TLS_VERIFY ON
)

add_executable(hw15_poster src/main.cpp)
target_include_directories(hw15_poster PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_compile_options(hw15_poster PRIVATE -Wall -Wextra -pedantic)

add_custom_command(TARGET hw15_poster POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/data
        ${CMAKE_CURRENT_BINARY_DIR}/data
)
