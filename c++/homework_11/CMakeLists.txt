find_package(Threads REQUIRED)

add_library(hw11_nav STATIC
    src/nav/Navigator.cpp
)
target_include_directories(hw11_nav PUBLIC include)
target_compile_options(hw11_nav PRIVATE -Wall -Wextra -pedantic)

if(UNIX)
    find_library(GPIOD_LIB gpiod)
    if(GPIOD_LIB)
        add_executable(hw11_app src/main.cpp)
        target_include_directories(hw11_app PRIVATE include)
        target_link_libraries(hw11_app PRIVATE hw11_nav ${GPIOD_LIB})
        target_compile_options(hw11_app PRIVATE -Wall -Wextra -pedantic)
        set_target_properties(hw11_app PROPERTIES OUTPUT_NAME drone_autopilot)
    else()
        message(WARNING "libgpiod not found — hw11 app will not be built")
    endif()
endif()

if(NOT CMAKE_CROSSCOMPILING)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    add_executable(hw11_tests
        tests/protocol_tests.cpp
        tests/nav_tests.cpp
    )
    target_include_directories(hw11_tests PRIVATE include)
    target_link_libraries(hw11_tests PRIVATE hw11_nav GTest::gtest_main Threads::Threads)
    target_compile_options(hw11_tests PRIVATE -Wall -Wextra -pedantic)
    include(GoogleTest)
    gtest_discover_tests(hw11_tests DISCOVERY_MODE PRE_TEST)
endif()
