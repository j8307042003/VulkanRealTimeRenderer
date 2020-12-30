cmake_minimum_required(VERSION 3.10)

project(Renderer)
add_library(Renderer 
			Camera.h Camera.cpp 
			RenderView.h RenderView.cpp 
			Renderer.h Renderer.cpp 
			VulkanRenderer.h VulkanRenderer.cpp 
			VkRealTimeRenderer.h VkRealTimeRenderer.cpp 
			VkForwardRenderer.h VkForwardRenderer.cpp 
			Shader.h Shader.cpp
			Model.h Model.cpp
			VkMaterial.h VkMaterial.cpp
			#stb_image.h 
			json11.hpp json11.cpp
			cgltf.h
			vkSys.h vkSys.cpp
			mikktspace.h mikktspace.c
			RenderData.h RenderData.cpp
			)
target_link_libraries(Renderer math)
target_link_libraries(Renderer spirv-reflect)
target_link_libraries(Renderer VulkanLib)
target_link_libraries(Renderer glm)
target_link_libraries(Renderer assimp)
target_link_libraries(Renderer glfw ${GLFW_LIBRARIES})
target_include_directories(Renderer INTERFACE ..)

