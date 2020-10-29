cmake_minimum_required(VERSION 3.10)

project(Renderer)
add_library(Renderer Camera.h Camera.cpp RenderView.h RenderView.cpp Renderer.h Renderer.cpp VulkanRenderer.h VulkanRenderer.cpp VkRealTimeRenderer.h VkRealTimeRenderer.cpp)
target_link_libraries(Renderer math)
target_link_libraries(Renderer VulkanLib)
target_link_libraries(Renderer glm)
target_link_libraries(Renderer assimp)
target_link_libraries(Renderer glfw ${GLFW_LIBRARIES})
target_include_directories(Renderer INTERFACE ..)

