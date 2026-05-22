add_library(ballistics src/ballistics.cpp)
target_include_directories(ballistics PUBLIC include)
target_compile_options(ballistics PRIVATE -Wall -Wextra -pedantic)

add_executable(ballistics_app src/main.cpp)
target_link_libraries(ballistics_app PRIVATE ballistics)
target_compile_options(ballistics_app PRIVATE -Wall -Wextra -pedantic)

if(NOT CMAKE_CROSSCOMPILING)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    add_executable(ballistics_tests tests/ballistics_tests.cpp)
    target_link_libraries(ballistics_tests PRIVATE ballistics GTest::gtest_main)
    target_compile_options(ballistics_tests PRIVATE -Wall -Wextra -pedantic)
    include(GoogleTest)
    gtest_discover_tests(ballistics_tests DISCOVERY_MODE PRE_TEST)
endif()
