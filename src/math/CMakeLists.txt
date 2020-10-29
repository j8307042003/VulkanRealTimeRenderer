cmake_minimum_required(VERSION 3.10)


project(math)
add_library(math Vec3.cpp Vec3.h Ray.h Ray.cpp Quaternion.h Quaternion.cpp Matrix4.h Transform.h Transform.cpp)
target_link_libraries(math glm)
target_include_directories(math INTERFACE ..)
