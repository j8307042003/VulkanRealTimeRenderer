#pragma once
#include "Renderer.h"
#include "VulkanLib/vkHelper.h"
#include "VulkanLib/VulkanInstance.h"
#include "math/Matrix4.h"
#include "glm/gtc/matrix_transform.hpp"
#define GL_SILENCE_DEPRECATION
#include "GLFW/include/GLFW/glfw3.h"
typedef unsigned int uint;


class VkRealTimeRenderer : public Renderer {
	private:
		VulkanLib::VulkanInstance vulkanInstance;
		GLFWwindow * window;


		VkExtent2D swapChainExtent;
		VkSwapchainKHR swapchain;
		VkQueue presentQueue;
		VkSemaphore imageAvailableSemaphore;
		VkSemaphore renderFinishedSemaphore;
		VkSurfaceFormatKHR surfaceFormat;
		std::vector<VkFramebuffer> swapchainFrameBuffer;
		VkRenderPass renderPass;
		VkPipeline graphicsPipeline;

		std::vector<VkCommandBuffer> commandBuffers;
	public:
		VkRealTimeRenderer();
		~VkRealTimeRenderer();
		void StartRender();

		void UpdateFrame() {}
		void ClearImage() {}
	private:


		void createSwapchain();
		void createRenderPass();
		void createGraphicPipeline();
		void createRenderCommandPool();

};