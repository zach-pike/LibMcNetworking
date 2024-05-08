cmake_minimum_required(VERSION 3.22.1)

project(McNetworkingLib)

file(GLOB_RECURSE LIB_SRC src/lib/*.cpp)

add_library(mcnetwork SHARED ${LIB_SRC})

add_executable(server src/testing/servertest.cpp)
add_executable(client src/testing/clienttest.cpp)

include_directories(src/lib)

target_link_libraries(server mcnetwork)
target_link_libraries(client mcnetwork)