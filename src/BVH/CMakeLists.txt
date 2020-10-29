cmake_minimum_required(VERSION 3.10)

project(BVH)

add_library(BVH AABB.h AABB.cpp)
target_link_libraries(BVH math)
target_include_directories(BVH INTERFACE ..)
