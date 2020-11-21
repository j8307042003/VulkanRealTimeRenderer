#pragma once
#include "Renderer.h"
#include "Shader.h"
#include "RenderData.h"
#include "Model.h"
#include "VkMaterial.h"
#include "vkSys.h"
#include "VulkanLib/vkHelper.h"
#include "VulkanLib/VulkanInstance.h"
#include "Camera.h"
#include "math/Matrix4.h"
#include "glm/gtc/matrix_transform.hpp"
#define GL_SILENCE_DEPRECATION
#include "GLFW/include/GLFW/glfw3.h"
typedef unsigned int uint;


namespace VkEngine {
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
		VkExtent2D swapChainExtent;
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
	};

}


class VkForwardRenderer : public Renderer {
	private:
		const int frameBuffer_Swapchain = 0;
		const int frameBuffer_Color = 1;
		const int frameBuffer_Depth = 2;
		static const int frameBuffer_Max = 3;

		struct forwardUbo {
			float color[4];
		};

	private:
		VulkanLib::VulkanInstance vulkanInstance;
		VkEngine::Window windowData;

		VkEngine::ScreenSetting screenSetting;
		VkRenderPass mainRenderPass;

		std::vector<VkAttachmentDescription> attachmentDes;

		VkEngine::Img colorImg;
		VkEngine::Img depthImg;

		VkQueue presentQueue;


		Shader forwardShader;
		Shader swapchainShader;
		
		VkPipeline graphicsPipeline;
		VkPipeline swapchainPipeline;


		BufferObject forwardUboBuffers;
		BufferObject forwardSwapchainUboBuffers;

		VkPipelineLayout forwardPipelineLayout;
		VkDescriptorSet attachmentWriteDescriptorSet;

		VkPipelineLayout swapchainPipelineLayout;
		std::vector<VkDescriptorSet> swapchainDescriptorSets;

		VkDescriptorPool descriptorPool;
		VkCommandPool commandPool;

		//SceneObj
		RenderData quadObj;


		std::vector<RenderData> customDatas;

		std::vector<VkCommandBuffer> commandBuffers;


		struct GlobalUniformData {
			glm::mat4 mvp;
			glm::vec4 cameraPos;
			glm::vec4 directionalLightDir;
			glm::vec4 directionalLightColor;
		};

		GlobalUniformData globalUniformData;
		BufferObject globalUniformBfferObj;		
		Camera camera;



		GLfloat deltaTime = 0.0f;
		GLfloat lastFrame = 0.0f;
		
		float rot_x = 0;
		float rot_y = 0;

	public:
		VkForwardRenderer();
		~VkForwardRenderer();


		void StartRender();
		void UpdateFrame() {}
		void ClearImage() {}
	private:

		void initSwapchain();
		void initWindow();
		void initAttachment();
		void initFrameBuffer();
		void initRenderPass();
		void initPipeline();
		void initShader();
		void initDescriptor();
		void initBuffer();
		void initCustomData();
		void initCommandBuffer();
		void initCamera();
		void checkAndCreateVulkanInstance();

		void doMovement();
		void doRotate();


		void createImg(int width, int height, VkFormat format, VkImageUsageFlags usage, VkEngine::Img & img);
};