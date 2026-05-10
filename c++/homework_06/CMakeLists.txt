include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_library(ballistics src/ballistics.cpp)
target_include_directories(ballistics PUBLIC include)

add_executable(ballistics_app src/main.cpp)
target_link_libraries(ballistics_app PRIVATE ballistics)

enable_testing()
add_executable(ballistics_tests tests/ballistics_tests.cpp)
target_link_libraries(ballistics_tests PRIVATE ballistics GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(ballistics_tests)
