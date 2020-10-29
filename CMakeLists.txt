cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

SET(ASSIMP_BUILD_TESTS CACHE BOOL OFF)

if (APPLE)
	#define Molten sdk path here!
	Set(MacVulkanSDK /Users/pine/lib/vulkansdk-macos-1.1.130.0)

	Set(VULKAN_SDK ${MacVulkanSDK}/macOS)
	Set(VK_LAYER_PATH ${MacVulkanSDK}/macOS/etc/vulkan/explicit_layer.d)
	Set(VK_ICD_FILENAMES ${MacVulkanSDK}/macOS/etc/vulkan/icd.d/MoltenVK_icd.json)
	Set(Vulkan_LIBRARY ${MacVulkanSDK}/MoltenVK/macOS/dynamic/libMoltenVK.dylib)
	Set(Vulkan_INCLUDE_DIR ${MacVulkanSDK}/MoltenVK/include)
	Set(DYLD_LIBRARY_PATH ${MacVulkanSDK}/macOS/lib)
	Set(VK_SDK_PATH ${MacVulkanSDK}/MoltenVK/)
	Set(PATH ${MacVulkanSDK}/macOS/bin:$PATH)
endif()





add_subdirectory(src/math src/math/build)
add_subdirectory(src/glm src/glm/build)

if (APPLE)
	add_subdirectory(src/GLFW src/GLFW/build)
endif()

if (WIN32)
	add_subdirectory(src/GLFW ../src/GLFW)
endif()

add_subdirectory(src/BVH src/BVH/build)
add_subdirectory(src/VulkanLib src/VulkanLib/build)
add_subdirectory(src/Renderer src/Renderer/build)


include_directories(src/glad)
include_directories(src/GLFW/include)


###assimp
add_subdirectory(src/assimp src/assimp/build)
LINK_DIRECTORIES (${Assimp_BINARY_DIR} ${AssetImporter_BINARY_DIR}/lib)
include_directories(src/assimp/include)


project(App)

add_executable(App src/main.cpp)
target_link_libraries(App Renderer)
target_link_libraries(App math)
target_link_libraries(App BVH)
target_link_libraries(App VulkanLib)
target_link_libraries(App glm)
target_link_libraries(App glfw ${GLFW_LIBRARIES})



if (APPLE)
  target_link_libraries(App "-framework OpenGL")
endif()


if(WIN32)
  find_package(OpenGL REQUIRED)
  target_link_libraries(App ${OPENGL_LIBRARIES})
endif()