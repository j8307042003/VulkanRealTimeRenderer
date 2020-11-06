#pragma once
#include "Renderer.h"
#include "VulkanLib/vkHelper.h"
#include "VulkanLib/VulkanInstance.h"
#include "math/Matrix4.h"
#include "glm/gtc/matrix_transform.hpp"
#define GL_SILENCE_DEPRECATION
#include "GLFW/include/GLFW/glfw3.h"
typedef unsigned int uint;


using namespace VkEngine {
	struct FrameBuffer {

	};

	struct Window
	{
		GLFWwindow * window;
		VkSurfaceKHR surface;	

		VkSurfaceFormatKHR surfaceFormat;
		VkSwapchainKHR swapchain;
		std::vector<VkImageView> swapchainImageViews;
		std::vector<VkFramebuffer> frameBuffers;
	};

	struct Swapchain {
	};

	struct Rasterizer {

	};

	struct ScreenSetting {
		int x;
		int y;
	};

	struct Img {
		VkFormat format;
		VkImage image;
		VkImageView view;
	}

}


class VkForwardRenderer : public Renderer {
	private:
		const int frameBuffer_Swapchain = 0;
		const int frameBuffer_Color = 1;
		const int frameBuffer_Depth = 2;
		const int frameBuffer_Max = 3;

	private:
		VulkanLib::VulkanInstance vulkanInstance;
		VkEngine::Window windowData;

		VkEngine::ScreenSetting screenSetting;
		VkRenderPass mainRenderPass;

		std::vector<VkAttachmentDescription> attachmentDes;
		std::vector<VkFramebuffer> frameBuffers;

		VkEngine::Img colorImg;
		VkEngine::Img depthImg;

		VkPipeline graphicsPipeline;
		

	public:
		VkForwardRenderer();
		~VkForwardRenderer();

		void render();
	private:

		void initSwapchain();
		void initWindow();
		void initFrameBuffer();
		void initRenderPass();
		void initPipeline();
		void checkAndCreateVulkanInstance();


		void createImg(int width, int height, VkFormat format, VkEngine::Img & img);
};