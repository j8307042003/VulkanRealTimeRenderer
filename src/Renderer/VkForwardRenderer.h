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
#include "json11.hpp"
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
		VkFramebuffer shadowmapFrameBuffer;


		VkEngine::ScreenSetting screenSetting;
		VkRenderPass mainRenderPass;
		VkRenderPass shadowRenderPass;

		std::vector<VkAttachmentDescription> attachmentDes;

		VkEngine::Img colorImg;
		VkEngine::Img depthImg;
		VkEngine::Img shadowmapImg;
		const int kShadowMapRes = 2048;

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

		glm::vec3 directionalLightAngle = glm::vec3(0, 0, 0);
		glm::vec3 directionalLightDir = glm::normalize(glm::vec3(-0.216, -0.973, 0.069));
		std::vector<RenderData> customDatas;

		std::vector<VkCommandBuffer> commandBuffers;


		struct GlobalUniformData {
			glm::mat4 mvp;
			glm::mat4 vp;
			glm::mat4 projectionMatrix;
			glm::mat4 worldToCamMatrix;
			glm::vec4 cameraPos;
			glm::vec4 directionalLightDir;
			glm::vec4 directionalLightColor;
			glm::mat4 shadowVPMatrix;			
			glm::vec4 floatArrs; // 0 time
		};

		GlobalUniformData globalUniformData;
		BufferObject globalUniformBfferObj;		
		BufferObject shadowPassUniformBufferObj;		

		Camera camera;

		json11::Json sceneJsonData;
		std::map<std::string, VkMaterial> materials = {};
		std::map<std::string, std::shared_ptr<Model>> models = {};

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
		void initShadowRenderPass();
		void initPipeline();
		void initShader();
		void initDescriptor();
		void initBuffer();
		void initCustomData();
		void initCommandBuffer();
		void initCamera();
		void initSceneData();		
		void checkAndCreateVulkanInstance();

		void doMovement();
		void doRotate();


		void createImg(int width, int height, VkFormat format, VkImageUsageFlags usage, VkEngine::Img & img);
};