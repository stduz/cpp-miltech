add_library(
    hw09_lib
    src/MissionProcessor.cpp
    src/solvers/AnalyticalSolver.cpp
    src/solvers/TableSolver.cpp
    src/providers/JsonTargetProvider.cpp
    src/config/FileConfigLoader.cpp
    src/config/ComponentFactory.cpp
    src/drone/DroneStates.cpp
    src/drone/DroneController.cpp
)
target_include_directories(hw09_lib PUBLIC include)
target_include_directories(hw09_lib PRIVATE ${CMAKE_SOURCE_DIR})
target_compile_options(hw09_lib PRIVATE -Wall -Wextra -pedantic)

add_executable(mission_app_v3 src/main.cpp)
target_link_libraries(mission_app_v3 PRIVATE hw09_lib)
target_compile_options(mission_app_v3 PRIVATE -Wall -Wextra -pedantic)

if(NOT CMAKE_CROSSCOMPILING)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    add_executable(hw09_tests
        tests/mission_tests.cpp
        tests/drone_state_tests.cpp
        tests/table_solver_tests.cpp
    )
    target_link_libraries(hw09_tests PRIVATE hw09_lib GTest::gtest_main)
    target_compile_options(hw09_tests PRIVATE -Wall -Wextra -pedantic)
    include(GoogleTest)
    gtest_discover_tests(hw09_tests DISCOVERY_MODE PRE_TEST)
endif()
