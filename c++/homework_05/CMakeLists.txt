cmake_minimum_required(VERSION 3.20)
project(homework_05 CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Бібліотека з логікою читання і парсингу телеметрії
add_library(telemetry src/telemetry.cpp)
target_include_directories(telemetry PUBLIC include)
target_compile_options(telemetry PRIVATE -Wall -Wextra -pedantic)

# Виконуваний файл, який читає шлях до логу з argv[1]
add_executable(telemetry_check src/main.cpp)
target_link_libraries(telemetry_check PRIVATE telemetry)
target_compile_options(telemetry_check PRIVATE -Wall -Wextra -pedantic)
