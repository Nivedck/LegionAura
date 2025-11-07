cmake_minimum_required(VERSION 3.16)
project(LegionAura VERSION 1.0.1)

add_definitions(-DLEGIONAURA_VERSION=\"${PROJECT_VERSION}\")



set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(lib)
add_subdirectory(cli)

