cmake_minimum_required(VERSION 3.14)

project(Riemann LANGUAGES CXX)

find_package(OpenMP REQUIRED)

add_executable(RiemannSiegel "src/RiemannSiegel.cpp")
target_link_libraries(RiemannSiegel PRIVATE OpenMP::OpenMP_CXX)
