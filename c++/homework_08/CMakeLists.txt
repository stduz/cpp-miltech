add_library(
    hw08_lib
    src/MissionProcessor.cpp
    src/solvers/AnalyticalSolver.cpp
    src/providers/JsonTargetProvider.cpp
    src/config/FileConfigLoader.cpp
    src/config/ComponentFactory.cpp
)
target_include_directories(hw08_lib PUBLIC include)
target_include_directories(hw08_lib PRIVATE ${CMAKE_SOURCE_DIR})
target_compile_options(hw08_lib PRIVATE -Wall -Wextra -pedantic)

add_executable(mission_app_v2 src/main.cpp)
target_link_libraries(mission_app_v2 PRIVATE hw08_lib)
target_compile_options(mission_app_v2 PRIVATE -Wall -Wextra -pedantic)

if(NOT CMAKE_CROSSCOMPILING)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    add_executable(hw08_tests tests/mission_tests.cpp)
    target_link_libraries(hw08_tests PRIVATE hw08_lib GTest::gtest_main)
    target_compile_options(hw08_tests PRIVATE -Wall -Wextra -pedantic)
    include(GoogleTest)
    gtest_discover_tests(hw08_tests DISCOVERY_MODE PRE_TEST)
endif()
