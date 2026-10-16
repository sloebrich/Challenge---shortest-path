
cmake_minimum_required(VERSION 3.16)

project(MarineRouting LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(PkgConfig REQUIRED)
find_package(nlohmann_json REQUIRED)

add_executable(marine-routing src/main.cpp)

target_link_libraries(marine-routing PRIVATE
    nlohmann_json::nlohmann_json
)

target_compile_options(marine-routing PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)
