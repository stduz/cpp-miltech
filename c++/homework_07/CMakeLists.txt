add_library(
    hw07_lib
    src/analytical_solver.cpp
    src/json_target_provider.cpp
    src/file_config_loader.cpp
    src/mission.cpp
    src/factory.cpp
)
target_include_directories(hw07_lib PUBLIC include)
target_include_directories(hw07_lib PRIVATE src ${CMAKE_SOURCE_DIR})
target_compile_options(hw07_lib PRIVATE -Wall -Wextra -pedantic)

add_executable(mission_app src/main.cpp)
target_link_libraries(mission_app PRIVATE hw07_lib)
target_compile_options(mission_app PRIVATE -Wall -Wextra -pedantic)

if(NOT CMAKE_CROSSCOMPILING)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    add_executable(hw07_tests tests/mission_tests.cpp)
    target_link_libraries(hw07_tests PRIVATE hw07_lib GTest::gtest_main)
    target_include_directories(hw07_tests PRIVATE src)
    target_compile_options(hw07_tests PRIVATE -Wall -Wextra -pedantic)
    include(GoogleTest)
    gtest_discover_tests(hw07_tests DISCOVERY_MODE PRE_TEST)
endif()
