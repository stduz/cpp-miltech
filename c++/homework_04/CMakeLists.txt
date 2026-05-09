cmake_minimum_required(VERSION 3.14)
project(ugv_odometry)

set(CMAKE_CXX_STANDARD 17)

add_executable(ugv_odometry src/main.cpp)
target_link_libraries(ugv_odometry m)
