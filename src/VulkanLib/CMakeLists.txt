cmake_minimum_required(VERSION 3.10)


project(VulkanLib)
find_package(Vulkan REQUIRED)
add_library(VulkanLib vkHelper.h vkHelper.cpp VulkanInstance.h VulkanInstance.cpp stb_image.h )
target_include_directories(VulkanLib INTERFACE ..)
target_link_libraries(VulkanLib Vulkan::Vulkan)
target_link_libraries(VulkanLib Renderer)

