find_package(Threads REQUIRED)

add_library(
    hw10_lib
    src/MissionProcessor.cpp
    src/solvers/AnalyticalSolver.cpp
    src/providers/ThreadSafeTargetProvider.cpp
    src/physics/DronePhysics.cpp
    src/config/FileConfigLoader.cpp
    src/config/ComponentFactory.cpp
)
target_include_directories(hw10_lib PUBLIC include)
target_include_directories(hw10_lib PRIVATE ${CMAKE_SOURCE_DIR})
target_compile_options(hw10_lib PRIVATE -Wall -Wextra -pedantic)
target_link_libraries(hw10_lib PRIVATE Threads::Threads)

add_executable(mission_app_v4 src/main.cpp)
target_link_libraries(mission_app_v4 PRIVATE hw10_lib Threads::Threads)
target_compile_options(mission_app_v4 PRIVATE -Wall -Wextra -pedantic)

if(NOT CMAKE_CROSSCOMPILING)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    add_executable(hw10_tests tests/threading_tests.cpp)
    target_link_libraries(hw10_tests PRIVATE hw10_lib GTest::gtest_main Threads::Threads)
    target_compile_options(hw10_tests PRIVATE -Wall -Wextra -pedantic)
    include(GoogleTest)
    gtest_discover_tests(hw10_tests DISCOVERY_MODE PRE_TEST)
endif()
