cmake_minimum_required(VERSION 3.16)
project(cutting-stock CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Gurobi (auto-detect or set -DGUROBI_HOME=...)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
find_package(Gurobi REQUIRED)

add_executable(compact compact/src/main.cpp)
target_link_libraries(compact Gurobi::Gurobi)

add_executable(colgen column-generation/src/main.cpp)
target_link_libraries(colgen Gurobi::Gurobi)
