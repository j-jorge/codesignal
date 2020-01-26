cmake_minimum_required(VERSION 3.13)

set(CMAKE_CXX_STANDARD 17)

find_package(benchmark)

set(executable_name "matrix-elements-sum-${CMAKE_CXX_COMPILER_ID}")

add_executable(
  ${executable_name}
  02-matrix-elements-sum.cpp
)

target_link_libraries(
  ${executable_name}
  benchmark::benchmark
  benchmark::benchmark_main
  )
