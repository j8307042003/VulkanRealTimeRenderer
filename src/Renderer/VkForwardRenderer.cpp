#include "VkForwardRenderer.h"
#include<stdlib.h>
#include<stdio.h>
#include<iostream>
#include "VulkanLib/vkHelper.h"
#include "VulkanLib/VulkanInstance.h"
#include<vector>
#include<set>
#define GL_SILENCE_DEPRECATION
#include "GLFW/include/GLFW/glfw3.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include "glm/gtc/quaternion.hpp"
#include <glm/gtx/string_cast.hpp>
#include <array>

#include "json11.hpp"
#include <fstream>      // std::ifstream
#include <memory>

bool checkDeviceExtensionSupport(VkPhysicalDevice device, std::vector<const char *> * deviceExtensions) {
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions->begin(), deviceExtensions->end());

	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

double mouse_pos_x = 0, mouse_pos_y = 0;
bool keys[1024];
bool mouse_keys[64] = {};

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods){
    //if(action == GLFW_PRESS) {
    //    double xpos, ypos;
    //    glfwGetCursorPos(window, &xpos, &ypos);
    //    std::cout << "-----------------" << std::endl;
    //    std::cout << std::endl << "x " << xpos << ". y " << ypos << std::endl;
    //    renderer->ClearImage();
    //    std::cout << "-----------------" << std::endl;
    //}
	
    if (action == GLFW_PRESS){
        glfwGetCursorPos(window, &mouse_pos_x, &mouse_pos_y);
        mouse_keys[button] = true;
    }
    else if (action == GLFW_RELEASE)
        mouse_keys[button] = false;
	
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (action == GLFW_PRESS)
		keys[key] = true;
	else if (action == GLFW_RELEASE)
		keys[key] = false;
}


// Moves/alters the camera positions based on user input
void VkForwardRenderer::doMovement()
{
	
	float x = 0, y = 0, z = 0;

	if (keys[GLFW_KEY_W]) {
		z += 1;
	}
	if (keys[GLFW_KEY_S]) {
		z += -1;
	}
	if (keys[GLFW_KEY_A]) {
		x += 1;
	}
	if (keys[GLFW_KEY_D]) {
		x += -1;
	}
	if (keys[GLFW_KEY_E]) {
		y += 1;
	}
	if (keys[GLFW_KEY_Q]) {
		y += -1;
	}

	if (x != 0 || y != 0 || z != 0) {
        Vec3 dir = camera.transform.TransformDir({x,y,z});
		camera.transform.position += dir * deltaTime * 4.0f;
        std::cout << "position : " << camera.transform.position.tostring() << std::endl;

	}

	float l_x = 0, l_y = 0, l_z = 0;
	int ratio = 1;
	if (keys[GLFW_KEY_1]) {
		l_x += ratio;
	}

	if (keys[GLFW_KEY_2]) {
		l_x -= ratio;
	}

	if (keys[GLFW_KEY_3]) {
		l_y += ratio;	
	}

	if (keys[GLFW_KEY_4]) {
		l_y -= ratio;		
	}

	if (keys[GLFW_KEY_5]) {
		l_z += ratio;			
	}

	if (keys[GLFW_KEY_6]) {
		l_z -= ratio;			
	}			

	directionalLightAngle.x += l_x;
	directionalLightAngle.y += l_y;
	directionalLightAngle.z += l_z;

	if (directionalLightAngle.x > 360) directionalLightAngle.x -= 360;
	if (directionalLightAngle.x <   0) directionalLightAngle.x  = 360 - directionalLightAngle.x;		

	if (directionalLightAngle.y > 360) directionalLightAngle.y -= 360;
	if (directionalLightAngle.y <   0) directionalLightAngle.y  = 360 - directionalLightAngle.y;		

	if (directionalLightAngle.z > 360) directionalLightAngle.z -= 360;
	if (directionalLightAngle.z <   0) directionalLightAngle.z  = 360 - directionalLightAngle.z;		

	if ( l_x != 0 || l_y != 0 || l_z != 0 )
	{
		glm::mat4 rot = glm::toMat4(glm::quat(glm::radians(directionalLightAngle)));
		glm::vec4 d = glm::mat4(1) * rot * glm::vec4(0, 0, 1, 1);
		directionalLightDir = d;
		directionalLightDir = glm::normalize(directionalLightDir);
		//std::cout << "dir : " << glm::to_string(directionalLightDir) << std::endl;
	}
	
}


void VkForwardRenderer::doRotate()
{
    if (mouse_keys[GLFW_MOUSE_BUTTON_1]) {
        double now_pos_x, now_pos_y;
        glfwGetCursorPos(windowData.window, &now_pos_x, &now_pos_y);

        float delta_x = now_pos_x - mouse_pos_x;
        float delta_y = now_pos_y - mouse_pos_y;
        if( abs(delta_x) < 5 && abs(delta_y) < 5 )
            return;

        mouse_pos_x = now_pos_x;
        mouse_pos_y = now_pos_y;

        const float kRotRatio = 1 / 10.0f;

        rot_x += delta_x * kRotRatio;
        rot_y += delta_y * kRotRatio;

        // const float kRotRatio = 1 / 10.0f;
        camera.transform.rotation = 
            // glm::normalize(cam.transform.rotation * glm::quat( delta_y * kRotRatio, delta_x * kRotRatio, 0, 0 ));
            glm::normalize(glm::quat(glm::radians(glm::vec3{rot_y, -rot_x, 0.0 })));
            // glm::normalize(cam.transform.rotation * glm::quat({delta_y * kRotRatio, delta_x * kRotRatio, 0 }));
        std::cout << "rotation : " << rot_y << "  " << rot_x << std::endl;
    }
}



VkForwardRenderer::VkForwardRenderer()
{
	screenSetting.x = 1920; // 1280;
	screenSetting.y = 1080; // 900;

	if (!glfwInit())
    return;

	windowData = {};

	//vulkanInstance = VulkanLib::VulkanInstance(extensions.size(), extensions.data());;
	VulkanLib::VulkanInstance * instance = vkSys::VkInstance::GetInstance();
	vulkanInstance = *instance;
	std::vector<const char*> checkExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_SURFACE_EXTENSION_NAME };
	bool extensionSupported = checkDeviceExtensionSupport(vulkanInstance.physicalDevice, &checkExtensions);


	// make sure vulkan instance created.
	checkAndCreateVulkanInstance();

	initCamera();
	initSwapchain();
	initAttachment();
	initRenderPass();
	initFrameBuffer();
	initShader();
	initBuffer();
	initDescriptor();
	initPipeline();

	commandPool = createCommandPool(vulkanInstance.device, vulkanInstance.queueFamily);

	initSceneData();
	initCustomData();
	initCommandBuffer();
}

VkForwardRenderer::~VkForwardRenderer()
{

}

void VkForwardRenderer::initShader()
{
	forwardShader = { vulkanInstance.device, "working/subpassForward/forwardVert.vert.spv", "working/subpassForward/forwardFrag.frag.spv" };
	swapchainShader = { vulkanInstance.device, "working/subpassForward/swapchainVert.vert.spv", "working/subpassForward/swapchainFrag.frag.spv" };
}

void VkForwardRenderer::initBuffer()
{
	forwardUboBuffers = BuildBuffer(vulkanInstance.device, vulkanInstance.deviceMemProps, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(forwardUbo));
	forwardSwapchainUboBuffers = BuildBuffer(vulkanInstance.device, vulkanInstance.deviceMemProps, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(forwardUbo));

	camera.transform.UpdateMatrix();
	globalUniformData = {camera.transform.modelMatrix};
	globalUniformBfferObj = BuildBuffer(vulkanInstance.device, vulkanInstance.deviceMemProps, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(GlobalUniformData));


	shadowPassUniformBufferObj = BuildBuffer(vulkanInstance.device, vulkanInstance.deviceMemProps, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(GlobalUniformData));
	glm::mat4 projectionMatrix = glm::ortho(-30.0f, 30.0f, -30.0f, 30.0f, 1.0f, 300.0f);
	//glm::mat4 projectionMatrix = glm::perspective(glm::radians(90.0f), (GLfloat) 1.0f, 0.001f, 1000.0f);

	//projectionMatrix[1][1] *= -1.0f;
	auto view = glm::lookAt(-directionalLightDir * 30.0f, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	globalUniformData.vp = projectionMatrix * view * glm::mat4(1);
	globalUniformData.shadowVPMatrix = globalUniformData.vp; 
	CopyDataToDeviceMemory(vulkanInstance.device, shadowPassUniformBufferObj.memory, shadowPassUniformBufferObj.size, &globalUniformData);
}


void VkForwardRenderer::initDescriptor()
{
	//create descriptor pool
	std::vector<VkDescriptorPoolSize> descriptorPoolSizes = 
	{
		createDescriptorPoolSize(windowData.swapchainImageViews.size() * 1000 + 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER),
		createDescriptorPoolSize(windowData.swapchainImageViews.size() * 1000 + 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER),
		createDescriptorPoolSize(windowData.swapchainImageViews.size() * 1000 * 2 + 1, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT),
	};

	descriptorPool = createDescriptorPool(vulkanInstance.device, descriptorPoolSizes.size(), descriptorPoolSizes.data(), windowData.swapchainImageViews.size() * 1000 + 1);

	// Attachment write
	{
		std::array<VkDescriptorSetLayoutBinding, 1> setLayoutBindings = 
		{
			createDescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, VK_SHADER_STAGE_VERTEX_BIT)
		};

		VkDescriptorSetLayout attachmentWriteSetLayout = createDescriptorSetLayout(vulkanInstance.device, 1, &setLayoutBindings[0]);
		VkPipelineLayout attachmentWritePipelineLayout = createPipelineLayout(vulkanInstance.device, 1, &attachmentWriteSetLayout);
		AllocateDescriptorSets(vulkanInstance.device, descriptorPool, 1, &attachmentWriteSetLayout, &attachmentWriteDescriptorSet);
		VkWriteDescriptorSet writeSet = createWriteDescriptorSet(attachmentWriteDescriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &forwardUboBuffers.descriptorBufInfo, 0);
		vkUpdateDescriptorSets(vulkanInstance.device, 1, &writeSet, 0, nullptr);
		forwardPipelineLayout = attachmentWritePipelineLayout;
	}

	// Attachment Read
	{
		std::array<VkDescriptorSetLayoutBinding, 3> setLayoutBindings = {};
		setLayoutBindings[0] = createDescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 0, VK_SHADER_STAGE_FRAGMENT_BIT);
		setLayoutBindings[1] = createDescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
		setLayoutBindings[2] = createDescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2, VK_SHADER_STAGE_FRAGMENT_BIT);
		VkDescriptorSetLayout descriptorLayout = createDescriptorSetLayout(vulkanInstance.device, 3, &setLayoutBindings[0]);
		VkPipelineLayout attachmentReadPipelineLayout = createPipelineLayout(vulkanInstance.device, 1, &descriptorLayout);
		swapchainPipelineLayout = attachmentReadPipelineLayout;

		swapchainDescriptorSets.resize(windowData.swapchainImageViews.size());
		for(int i = 0; i < windowData.swapchainImageViews.size(); ++i)
		{
			AllocateDescriptorSets(vulkanInstance.device, descriptorPool, 1, &descriptorLayout, &swapchainDescriptorSets[i]);
			std::array<VkDescriptorImageInfo, 3> descriptorImgInfos = 
			{
				createDescriptorImageInfo(VK_NULL_HANDLE, colorImg.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
				createDescriptorImageInfo(VK_NULL_HANDLE, depthImg.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			};

			std::array<VkWriteDescriptorSet, 3> writeDescriptorSets = 
			{
				createWriteDescriptorSet(swapchainDescriptorSets[i], VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, &descriptorImgInfos[0], 0),
				createWriteDescriptorSet(swapchainDescriptorSets[i], VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, &descriptorImgInfos[1], 1),
				createWriteDescriptorSet(swapchainDescriptorSets[i], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &forwardSwapchainUboBuffers.descriptorBufInfo, 2),
			};

			vkUpdateDescriptorSets(vulkanInstance.device, 3, &writeDescriptorSets[0], 0, nullptr);
		}
	}
}





VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
	for (const auto format : availableFormats) {
		if (format.format == VK_FORMAT_R16G16B16A16_SFLOAT && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return format;
		}
	}

	return availableFormats[0];
}

VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
	for (const auto& availablePresentMode : availablePresentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, int width, int height) {
	if (capabilities.currentExtent.width != UINT32_MAX && false) {
		return capabilities.currentExtent;
	}
	else {
		VkExtent2D actualExtent = { (uint32_t)width, (uint32_t)height };
		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}


void VkForwardRenderer::initSwapchain()
{
	if (windowData.window == nullptr) {
		initWindow();
	}




	presentQueue = {};
	vkGetDeviceQueue(vulkanInstance.device, vulkanInstance.queueFamily, 0, &presentQueue);
	VkSurfaceCapabilitiesKHR capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vulkanInstance.physicalDevice, windowData.surface, &capabilities);

	std::vector<VkSurfaceFormatKHR> surfaceFormats = {};
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(vulkanInstance.physicalDevice, windowData.surface, &formatCount, nullptr);

	if (formatCount != 0) {
		surfaceFormats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(vulkanInstance.physicalDevice, windowData.surface, &formatCount, surfaceFormats.data());
	}

	std::vector<VkPresentModeKHR> presentModes = {};
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(vulkanInstance.physicalDevice, windowData.surface, &presentModeCount, nullptr);
	if (presentModeCount != 0) {
		presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(vulkanInstance.physicalDevice, windowData.surface, &presentModeCount, presentModes.data());
	}

	uint32_t queueCount;
	vkGetPhysicalDeviceQueueFamilyProperties(vulkanInstance.physicalDevice, &queueCount, NULL);

	for(int i = 0; i < queueCount; ++i) {
		VkBool32 supported;
		vkGetPhysicalDeviceSurfaceSupportKHR( vulkanInstance.physicalDevice, i, windowData.surface, &supported);
	}

	windowData.surfaceFormat = chooseSwapSurfaceFormat(surfaceFormats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(presentModes);
	VkExtent2D extent = chooseSwapExtent(capabilities, screenSetting.x, screenSetting.y);
	screenSetting.x = (int)extent.width;
	screenSetting.y = (int)extent.height;
	windowData.swapChainExtent = extent;
	uint32_t imageCount = capabilities.minImageCount + 1;
	if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
		imageCount = capabilities.maxImageCount;
	}
	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = windowData.surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = windowData.surfaceFormat.format;
	createInfo.imageColorSpace = windowData.surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.queueFamilyIndexCount = 0; // Optional
	createInfo.pQueueFamilyIndices = nullptr; // Optional
	createInfo.preTransform = capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;


	VkResult vkResult = vkCreateSwapchainKHR(vulkanInstance.device, &createInfo, nullptr, &windowData.swapchain);
	if (vkResult != VK_SUCCESS) {
		throw std::runtime_error("failed to create swap chain!");
	}
}


void VkForwardRenderer::initWindow()
{
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    windowData.window = glfwCreateWindow(screenSetting.x, screenSetting.y, "Vk Forward Renderer Window", NULL, NULL);

	VkResult vkResult = glfwCreateWindowSurface(vulkanInstance.instance, windowData.window, nullptr, &windowData.surface);
	if (vkResult != VK_SUCCESS) {
		std::cout << vkResult << std::endl;
		throw std::runtime_error("failed to create window surface! ");
		return;
	}

    // glfwMakeContextCurrent(windowData.window);
    glfwSetMouseButtonCallback(windowData.window, mouse_button_callback);
    glfwSetKeyCallback(windowData.window, key_callback);	
}

void VkForwardRenderer::initAttachment()
{
	// create image view
	std::vector<VkImage> swapchainImgs = {};
	uint32_t imageCount;
	vkGetSwapchainImagesKHR(vulkanInstance.device, windowData.swapchain, &imageCount, nullptr);
	swapchainImgs.resize(imageCount);
	vkGetSwapchainImagesKHR(vulkanInstance.device, windowData.swapchain, &imageCount, swapchainImgs.data());

	windowData.swapchainImageViews = {};
	windowData.swapchainImageViews.resize(swapchainImgs.size());

	for (int i = 0; i < swapchainImgs.size(); ++i) {
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = swapchainImgs[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = windowData.surfaceFormat.format;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(vulkanInstance.device, &createInfo, nullptr, &windowData.swapchainImageViews[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create swap chain view!");
		}
	}

	VkFormat depthFormat;
	getSupportedDepthFormat(vulkanInstance.physicalDevice, &depthFormat);
	createImg(screenSetting.x, screenSetting.y, windowData.surfaceFormat.format, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, colorImg);
	createImg(screenSetting.x, screenSetting.y, depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, depthImg);	
	createImg(kShadowMapRes, kShadowMapRes, depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, shadowmapImg);	
}


void VkForwardRenderer::initFrameBuffer()
{
	{
		//frame buffer create set up
		VkImageView fbAttachments[frameBuffer_Max];

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = mainRenderPass;
		framebufferInfo.attachmentCount = 3;
		framebufferInfo.pAttachments = fbAttachments;
		framebufferInfo.width = windowData.swapChainExtent.width;
		framebufferInfo.height = windowData.swapChainExtent.height;
		framebufferInfo.layers = 1;



		windowData.frameBuffers = {};
		windowData.frameBuffers.resize(windowData.swapchainImageViews.size());
		for (int i = 0; i < windowData.frameBuffers.size(); ++i)
		{
			fbAttachments[frameBuffer_Swapchain] = windowData.swapchainImageViews[i]; 
			fbAttachments[frameBuffer_Color] = colorImg.view;
			fbAttachments[frameBuffer_Depth] = depthImg.view; 
	
	
			if (vkCreateFramebuffer(vulkanInstance.device, &framebufferInfo, nullptr, &windowData.frameBuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create frame buffer!");
			}		
		}
	}


	// shadow frame buffer
	{
		//frame buffer create set up
		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = shadowRenderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = &shadowmapImg.view;
		framebufferInfo.width = kShadowMapRes;
		framebufferInfo.height = kShadowMapRes;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(vulkanInstance.device, &framebufferInfo, nullptr, &shadowmapFrameBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to create shadow map frame buffer!");
		}	

	}

}

void VkForwardRenderer::initShadowRenderPass()
{

	std::array<VkAttachmentDescription, 1> shadowPassAttachs = {};

	//todo
	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = depthImg.format;
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	shadowPassAttachs[0] = depthAttachment;


	VkAttachmentReference depthAttachmentRef{};
	depthAttachmentRef.attachment = 0;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;


	VkSubpassDescription subpasses[1] = {};
	// shadow subpass
	subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpasses[0].colorAttachmentCount = 0;
	subpasses[0].pDepthStencilAttachment = &depthAttachmentRef;

	/*
	std::array<VkSubpassDependency, 2> dependencies = {};
    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;	

    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;	
	*/

	shadowRenderPass = {};
	VkPipelineLayout pipelineLayout;
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(shadowPassAttachs.size());
	renderPassInfo.pAttachments = &shadowPassAttachs[0];
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpasses[0];
	renderPassInfo.dependencyCount = 0;
	renderPassInfo.pDependencies = nullptr;

	VkResult result = vkCreateRenderPass(vulkanInstance.device, &renderPassInfo, nullptr, &shadowRenderPass);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("create render pass error!");
	}	
}


void VkForwardRenderer::initRenderPass()
{

	initShadowRenderPass();

	attachmentDes = {};
	attachmentDes.resize(frameBuffer_Max);

	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = colorImg.format;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	attachmentDes[frameBuffer_Color] = colorAttachment;

	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = depthImg.format;
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	attachmentDes[frameBuffer_Depth] = depthAttachment;


	VkAttachmentDescription swapchainAttachment{};
	swapchainAttachment.format = windowData.surfaceFormat.format;
	swapchainAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	swapchainAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	swapchainAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	swapchainAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	swapchainAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	swapchainAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	swapchainAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	attachmentDes[frameBuffer_Swapchain] = swapchainAttachment;

	VkAttachmentReference swapchainAttachmentRef{};
	swapchainAttachmentRef.attachment = frameBuffer_Swapchain;
	swapchainAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = frameBuffer_Color;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef{};
	depthAttachmentRef.attachment = frameBuffer_Depth;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;


	VkAttachmentReference colorAttachmentRefs [] = {swapchainAttachmentRef, colorAttachmentRef};


	VkSubpassDescription subpasses[2] = {};
	// forward subpass
	subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpasses[0].colorAttachmentCount = 1;
	subpasses[0].pColorAttachments = &colorAttachmentRef;
	subpasses[0].pDepthStencilAttachment = &depthAttachmentRef;

	//swapchain subpass
	VkAttachmentReference inputAttachmentInputs[2] = {};
	inputAttachmentInputs[0].attachment = 1;
	inputAttachmentInputs[0].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	inputAttachmentInputs[1].attachment = 2;
	inputAttachmentInputs[1].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;


	VkAttachmentReference colorReferenceSwapchain = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
	subpasses[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpasses[1].colorAttachmentCount = 1;
	subpasses[1].pColorAttachments = &colorReferenceSwapchain;	
    subpasses[1].inputAttachmentCount = 2;
    subpasses[1].pInputAttachments = &inputAttachmentInputs[0];	


	std::array<VkSubpassDependency, 3> dependencies = {};
    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;	

    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = 1;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;	

    dependencies[2].srcSubpass = 1;
    dependencies[2].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[2].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[2].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[2].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[2].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[2].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;



	mainRenderPass = {};
	VkPipelineLayout pipelineLayout;
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDes.size());
	renderPassInfo.pAttachments = &attachmentDes[0];
	renderPassInfo.subpassCount = 2;
	renderPassInfo.pSubpasses = &subpasses[0];
	renderPassInfo.dependencyCount = dependencies.size();
	renderPassInfo.pDependencies = dependencies.data();

	VkResult result = vkCreateRenderPass(vulkanInstance.device, &renderPassInfo, nullptr, &mainRenderPass);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("create render pass error!");
	}
}


void VkForwardRenderer::initPipeline()
{

	VkPipelineLayout pipelineLayout;


	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = 0;
	pipelineLayoutCreateInfo.pSetLayouts = nullptr;
	pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;


	if (vkCreatePipelineLayout(vulkanInstance.device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("Create pipeline layout error!");
	}



	graphicsPipeline = forwardShader.BuildPipeline(mainRenderPass, 0, forwardPipelineLayout);


	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

	swapchainPipeline = swapchainShader.BuildPipeline(mainRenderPass, 1, swapchainPipelineLayout, &vertexInputInfo);

}



void VkForwardRenderer::checkAndCreateVulkanInstance()
{
	if (vulkanInstance.instance != VK_NULL_HANDLE)
	{
		return;
	}

	uint32_t count;
	auto requires = glfwGetRequiredInstanceExtensions(&count);
	std::vector<const char*> extensions = {};
	for (int i = 0; i < count; ++i) {
		extensions.push_back(requires[i]);
	}
	vulkanInstance = VulkanLib::VulkanInstance(extensions.size(), extensions.data());
}


void VkForwardRenderer::createImg(int width, int height, VkFormat format, VkImageUsageFlags usage, VkEngine::Img & img)
{
	VkImageCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    createInfo.imageType = VK_IMAGE_TYPE_2D;
    createInfo.format = format;
    createInfo.extent.width = width;
    createInfo.extent.height = height;
    createInfo.extent.depth = 1;
    createInfo.mipLevels = 1;
    createInfo.arrayLayers = 1;
    createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    createInfo.usage = usage | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;

	if (vkCreateImage(vulkanInstance.device, &createInfo, nullptr, &img.image) != VK_SUCCESS)
	{
		throw std::runtime_error("Create vk image error!");
	}

	VkMemoryRequirements memReqs;
	vkGetImageMemoryRequirements(vulkanInstance.device, img.image, &memReqs);
	VkMemoryAllocateInfo memallocInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
	memallocInfo.allocationSize = memReqs.size;
	memallocInfo.memoryTypeIndex;
	getMemoryType(vulkanInstance.deviceMemProps, memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memallocInfo.memoryTypeIndex);
	VkDeviceMemory imgMem; 
	VkResult result = vkAllocateMemory(vulkanInstance.device, &memallocInfo, nullptr, &imgMem);
	if (result != VK_SUCCESS) 
	{
		throw std::runtime_error("Allocate image memory error");
		return;
	}


	result = vkBindImageMemory( vulkanInstance.device, img.image, imgMem, 0);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("bind image error");
		return;
	}


	VkImageAspectFlags aspect = 0;
	if (usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
	{
		aspect = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	if (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
	{
		aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
	}



	VkImageViewCreateInfo createViewInfo{};
	createViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createViewInfo.image = img.image;
	createViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createViewInfo.format = format;
	createViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	createViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	createViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	createViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	createViewInfo.subresourceRange.aspectMask = aspect;
	createViewInfo.subresourceRange.baseMipLevel = 0;
	createViewInfo.subresourceRange.levelCount = 1;
	createViewInfo.subresourceRange.baseArrayLayer = 0;
	createViewInfo.subresourceRange.layerCount = 1;

	if (vkCreateImageView(vulkanInstance.device, &createViewInfo, nullptr, &img.view) != VK_SUCCESS) {
		throw std::runtime_error("failed to create swap chain view!");
	}

	img.format = format;

}

void VkForwardRenderer::initCommandBuffer()
{
	commandBuffers = AllocateCommandBuffer(vulkanInstance.device, commandPool, windowData.swapchainImageViews.size());

	
	auto buf = BuildBuffer(vulkanInstance.device, vulkanInstance.deviceMemProps, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 4 * sizeof(float));

	VkViewport viewport = {};
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = windowData.swapChainExtent.width;
    viewport.height = windowData.swapChainExtent.height;
    viewport.minDepth = 0;
    viewport.maxDepth = 1;	

	VkRect2D scissor = {};
	scissor.offset = {0, 0};
	scissor.extent = windowData.swapChainExtent;    
	std::cout << scissor.extent.width << std::endl;
	std::cout << scissor.extent.height << std::endl;
	for (int i = 0; i < commandBuffers.size(); ++i) 
	{
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0;
		beginInfo.pInheritanceInfo = nullptr;

		if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("command buffer begin info error!");
		}

		//shadow map pass
		{
			VkExtent2D extent = {};
    		extent.width = kShadowMapRes;
    		extent.height = kShadowMapRes;

			VkRect2D scissor = {};
			scissor.offset = {0, 0};
			scissor.extent = extent;


			VkRenderPassBeginInfo renderPassBeginInfo = {};
			renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassBeginInfo.renderPass = shadowRenderPass;
			renderPassBeginInfo.framebuffer = shadowmapFrameBuffer;
			renderPassBeginInfo.renderArea.offset = {0,0};
			renderPassBeginInfo.renderArea.extent = extent;

			std::array<VkClearValue, 1> clearColors = {
				VkClearValue{{1.0f, 0}},
			};

			VkViewport viewport = {};
			viewport.x = 0;
			viewport.y = 0;
			viewport.width = kShadowMapRes;
			viewport.height = kShadowMapRes;
			viewport.minDepth = 0;
			viewport.maxDepth = 1;	

			renderPassBeginInfo.clearValueCount = clearColors.size();
			renderPassBeginInfo.pClearValues = &clearColors[0];
			vkCmdBeginRenderPass(commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdSetViewport(commandBuffers[i], 0, 1, &viewport);
			vkCmdSetScissor(commandBuffers[i], 0, 1, &scissor);

			const float depthBiasConstant = 1.25f;
			const float depthBiasSlope = 1.75f;
			vkCmdSetDepthBias(commandBuffers[i], depthBiasConstant, 0.0f, depthBiasSlope);

			for(int j = 0; j < customDatas.size(); ++j)
			{
				customDatas[j].renderShadowPass(commandBuffers[i]);
			}

			vkCmdEndRenderPass(commandBuffers[i]);
		}


		VkRenderPassBeginInfo renderPassBeginInfo = {};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = mainRenderPass;
		renderPassBeginInfo.framebuffer = windowData.frameBuffers[i];
		renderPassBeginInfo.renderArea.offset = {0,0};
		renderPassBeginInfo.renderArea.extent = windowData.swapChainExtent;

		std::array<VkClearValue, 3> clearColors = {
			VkClearValue{{0.0f, 0.0f, 0.0f, 1.0f}},
			VkClearValue{{0.0f, 0.0f, 0.0f, 1.0f}},
			VkClearValue{{1.0f, 0}},
		};
		renderPassBeginInfo.clearValueCount = 3;
		renderPassBeginInfo.pClearValues = &clearColors[0];
		vkCmdBeginRenderPass(commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdSetViewport(commandBuffers[i], 0, 1, &viewport);
		vkCmdSetScissor(commandBuffers[i], 0, 1, &scissor);

		// first pass
		{
			/*
			vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
			vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, forwardPipelineLayout, 0, 1, &attachmentWriteDescriptorSet, 0, nullptr);
			VkDeviceSize offsets[1] = { 0 };			
			vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, &buf.buffer, offsets);
			vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);
			*/
		}
		
		{
			// quadObj.render(commandBuffers[i]);
			for(int j = 0; j < customDatas.size(); ++j)
			{
				customDatas[j].render(commandBuffers[i]);
			}
		}

		// swapchain pass
		{
			vkCmdNextSubpass(commandBuffers[i], VK_SUBPASS_CONTENTS_INLINE);
			vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, swapchainPipeline);
			vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, swapchainPipelineLayout, 0, 1, &swapchainDescriptorSets[i], 0, nullptr);
			vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);
		}

		vkCmdEndRenderPass(commandBuffers[i]);
		if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
		    throw std::runtime_error("failed to record command buffer!");
		}

	}


}

void VkForwardRenderer::initCustomData()
{
	std::shared_ptr<Model> pM(new Model);
	std::vector<Model::ModelVertexData> vertexData = 
	{
		//{{-10.0f, -10.0f, 10.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 0.0f}},
		//{{ 10.0f, -10.0f, 10.0f}, {1.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 0.0f}},
		//{{ 10.0f,  10.0f, 10.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 0.0f}},
		//{{-10.0f,  10.0f, 10.0f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 0.0f}},

		//{{-10.0f, -10.0f, -10.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 0.0f}},
		//{{ 10.0f, -10.0f, -10.0f}, {1.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 0.0f}},
		//{{ 10.0f,  10.0f, -10.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 0.0f}},
		//{{-10.0f,  10.0f, -10.0f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 0.0f}},
		

		//
		{{-1.0f, -1.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 0.0f}},
		{{ 1.0f, -1.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 0.0f}},
		{{ 1.0f,  1.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 0.0f}},
		{{-1.0f,  1.0f, 1.0f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 0.0f}},

	};

	std::vector<uint32_t> indices = 
	{
		0, 1, 2, 2, 3, 0, // far plane
		//4, 0, 3, 3, 7, 4, // left plane
		//1, 5, 6, 6, 2, 1, // right plane
		//4, 6, 1, 1, 0, 4, // bot plane
		//3, 2, 6, 6, 7, 3, // top plane
		//5, 4, 7, 7, 6, 5, // back plane
	};

	pM.get()->SetData(vertexData, indices);
	VkMaterial skyboxMat = {};
	skyboxMat.pShader = std::shared_ptr<Shader>(new Shader{vulkanInstance.device, "working/subpassForward/forwardSkybox.vert.spv", "working/subpassForward/forwardSkybox.frag.spv" });
	skyboxMat.ReadMainTexture(vulkanInstance, commandPool, "working/texture/MonValley_G_DirtRoad_8k.jpg");
	//skyboxMat.ReadMainTexture(vulkanInstance, commandPool, "working/texture/Newport_Loft_8k.jpg");
	//skyboxMat.ReadMainTexture(vulkanInstance, commandPool, "working/texture/PaperMill_E_8k.jpg");
	//skyboxMat.ReadMainTexture(vulkanInstance, commandPool, "working/texture/Topanga_Forest_B_8k.jpg");
	

	VkMaterial tmpBRDFMat = {};
	tmpBRDFMat.pShader = std::shared_ptr<Shader>(new Shader{ vulkanInstance.device, "working/subpassForward/forwardSkybox.vert.spv", "working/subpassForward/forwardSkybox.frag.spv" });
	tmpBRDFMat.ReadMainTexture(vulkanInstance, commandPool, "working/texture/BRDF LUT.png");



	VkMaterial mat = {};
	mat.pShader = std::shared_ptr<Shader>(new Shader{ vulkanInstance.device, "working/subpassForward/forward.vert.spv", "working/subpassForward/forward.frag.spv" });
	mat.ReadMainTexture(vulkanInstance, commandPool, "working/texture/wall.jpg");

	for (int i = 0; i < customDatas.size(); ++i)
	{
		customDatas[i].buildProgram(vulkanInstance.device, mainRenderPass, 0);
		customDatas[i].buildRenderData(vulkanInstance.device, vulkanInstance.deviceMemProps, descriptorPool, globalUniformBfferObj.descriptorBufInfo, shadowPassUniformBufferObj.descriptorBufInfo, skyboxMat.mainTexImageView, tmpBRDFMat.mainTexImageView, shadowmapImg.view);
		// customDatas[i].buildRenderData(vulkanInstance.device, vulkanInstance.deviceMemProps, descriptorPool, globalUniformBfferObj.descriptorBufInfo, globalUniformBfferObj.descriptorBufInfo, skyboxMat.mainTexImageView, tmpBRDFMat.mainTexImageView, shadowmapImg.view);
	}


	// skybox
	RenderData skyboxRenderData = {};
	skyboxRenderData.setMaterial(skyboxMat);
	skyboxRenderData.setModel(pM);
	skyboxRenderData.buildProgram(vulkanInstance.device, mainRenderPass, 0);
	//skyboxRenderData.buildRenderData(vulkanInstance.device, vulkanInstance.deviceMemProps, descriptorPool, globalUniformBfferObj.descriptorBufInfo, shadowPassUniformBufferObj.descriptorBufInfo, skyboxMat.mainTexImageView, tmpBRDFMat.mainTexImageView, shadowmapImg.view);
	//customDatas.push_back(skyboxRenderData);
}


void VkForwardRenderer::initSceneData()
{
	std::ifstream file("working/scene/scene1.json", std::ios::ate | std::ios::binary);
	//std::ifstream file("working/scene/scene2.json", std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    size_t 	filesize = (size_t) file.tellg();
    std::vector<char> buffer(filesize+1);
    file.seekg(0);
    file.read(buffer.data(), filesize);
    file.close();
	buffer[buffer.size() - 1] = '\0';

	std::string err;
	sceneJsonData = json11::Json::parse(buffer.data(), err);


	
	materials = {};
	models = {};
	//materials
	{
		auto jsonMaterials = sceneJsonData["materials"];
		if (jsonMaterials.is_array())
		{
			for (int i = 0; i < jsonMaterials.array_items().size(); ++i)
			{
				auto jsonMaterial = jsonMaterials.array_items()[i];
				std::string name = jsonMaterial["name"].string_value();
				std::string vertexSdr = jsonMaterial["vertexShader"].string_value();
				std::string fragmentSdr = jsonMaterial["fragmentShader"].string_value();
				std::string albedoTex = jsonMaterial["albedo"].is_null() ? "" : jsonMaterial["albedo"].string_value();
				std::string specularTex = jsonMaterial["specular"].is_null() ? "" : jsonMaterial["specular"].string_value();
				std::string normalMapTex = jsonMaterial["normalMap"].is_null() ? "" : jsonMaterial["normalMap"].string_value();
				json11::Json assignment = jsonMaterial["assignment"];

				VkMaterial mat = {};

				mat.pShader = std::shared_ptr<Shader>( new Shader{ vulkanInstance.device, vertexSdr.c_str(), fragmentSdr.c_str() });

				mat.SetTexture(*vkSys::TexMgr::GetTexture(albedoTex.c_str(), commandPool));
				mat.SetSpecularTexture(*vkSys::TexMgr::GetTexture(specularTex.c_str(), commandPool));
				mat.SetNormalTexture(*vkSys::TexMgr::GetTexture(normalMapTex.c_str(), commandPool));

				if (!assignment.is_null())
				{
					auto items = assignment.object_items();
					mat.bindingSets.resize(items.size());

					int count = 0;
					for (auto it = items.begin(); it != items.end(); it++)
					{
						mat.bindingSets[count] = { it->first, *vkSys::TexMgr::GetTexture(it->second.string_value().c_str(), commandPool) };
						count++;
					}

				}

				materials[name] = mat;
			}
		}
	}

	//mesh
	{
		auto jsonMeshs = sceneJsonData["meshs"];
		if (jsonMeshs.is_array())
		{
			for (int i = 0; i < jsonMeshs.array_items().size(); ++i)
			{
				auto jsonMesh = jsonMeshs.array_items()[i];
				std::string name = jsonMesh["name"].string_value();
				std::string path = jsonMesh["file"].string_value();
				Model * pModel = new Model();
				pModel->LoadModel(path.c_str());
				models[name] = std::shared_ptr<Model>(pModel);
			}
		}
	}
	
	//models
	{
		auto jsonModels = sceneJsonData["models"];
		if (jsonModels.is_array())
		{
			for (int i = 0; i < jsonModels.array_items().size(); ++i)
			{
				auto jsonModel = jsonModels.array_items()[i];
				bool autoBuild = jsonModel["autoBuild"].is_null() ? false : jsonModel["autoBuild"].bool_value();
				std::string matName = jsonModel["material"].string_value();
				auto posJson = jsonModel["pos"].array_items();
				auto rotJson = jsonModel["rot"].array_items();
				auto scaleJson = jsonModel["scale"].array_items();
				auto castShadowJson = jsonModel["castShadow"];

				glm::vec3 pos = glm::vec3(posJson[0].number_value(), posJson[1].number_value(), posJson[2].number_value());
				glm::vec3 rot = glm::vec3(rotJson[0].number_value(), rotJson[1].number_value(), rotJson[2].number_value());
				glm::vec3 scale = glm::vec3(scaleJson[0].number_value(), scaleJson[1].number_value(), scaleJson[2].number_value());

				glm::mat4 modelMatrix = glm::scale(glm::translate(glm::mat4(1), pos) *  glm::toMat4(glm::quat(glm::radians(rot))), scale);

				bool castShadow = castShadowJson.is_null() ? true : castShadowJson.bool_value();

				auto & mat = materials[matName];
				if (autoBuild)
				{
					std::string path = jsonModel["meshPath"].string_value();
					bool bOverrideMat = jsonModel["overrideMat"].is_null() ? true : jsonModel["overrideMat"].bool_value();
					auto renderDatas = RenderDataUtils::LoadObj(path.c_str(), mat, commandPool);
					for (int j = 0; j < renderDatas.size(); ++j)
					{
						renderDatas[j].modelMatrix = modelMatrix;
						renderDatas[j].setShadow(castShadow);
						if (!bOverrideMat) renderDatas[j].setMaterial(mat); // TODO Mat assignment
					}
					customDatas.insert(customDatas.end(), renderDatas.begin(), renderDatas.end());
				}
				else 
				{
					std::string meshName = jsonModel["mesh"].string_value();
					auto mesh = models[meshName];
					RenderData rd = {};
					rd.setMaterial(mat);
					rd.setModel(mesh);
					rd.modelMatrix = modelMatrix;
					customDatas.push_back(rd);
				}
			}
		}
	}
}



void VkForwardRenderer::initCamera()
{
	camera = {};
	rot_y = 6.3;
	rot_x = 83.5;
	camera.transform = {{7.44123, 2.30499, -1.06367},  glm::normalize(glm::quat(glm::radians(glm::vec3{rot_y, -rot_x, 0.0 })))};
}



void VkForwardRenderer::StartRender()
{
	VkQueue queue = {};
	vkGetDeviceQueue(vulkanInstance.device, vulkanInstance.queueFamily, 0, &queue);  		


	VkSemaphore imageAvailableSemaphore = {};
	VkSemaphore renderFinishedSemaphore = {};

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	if (vkCreateSemaphore(vulkanInstance.device, &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS ||
		vkCreateSemaphore(vulkanInstance.device, &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS) {

		throw std::runtime_error("failed to create semaphores!");
	}	

    while (!glfwWindowShouldClose(windowData.window))
	{
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

        // std::cout << "update frame" << std::endl;
        // renderer->UpdateFrame();



        //stall gpu renderer. clean up gpu usage for image display
        // renderer->Stall();

        /* Render here */


		//Update camera
		{
			doMovement();
			doRotate();

			GlobalUniformData shadowPassData = {};
			//glm::mat4 shadowProjectionMatrix = glm::ortho(-30.0f, 30.0f, -30.0f, 30.0f, 1.0f, 300.0f);
			glm::mat4 shadowProjectionMatrix = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, 1.0f, 70.0f);
			auto shadowview = glm::lookAt(-directionalLightDir * 40.0f, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

			shadowPassData.vp = shadowProjectionMatrix * shadowview * glm::mat4(1);
			shadowPassData.shadowVPMatrix = shadowPassData.vp;
			CopyDataToDeviceMemory(vulkanInstance.device, shadowPassUniformBufferObj.memory, shadowPassUniformBufferObj.size, &shadowPassData);



        	camera.transform.UpdateMatrix();
			glm::mat4 projectionMatrix = glm::perspective(glm::radians(90.0f), (GLfloat)windowData.swapChainExtent.width / (GLfloat)windowData.swapChainExtent.height, 0.001f, 1000.0f);
			projectionMatrix[1][1] *= -1.0f;
			glm::vec3 scale = {0.01f, 0.01f, 0.01f};
			globalUniformData.cameraPos = {camera.transform.position.x, camera.transform.position.y, camera.transform.position.z, 0.0};
			auto view = glm::lookAt(glm::vec3(0.0f, 2.0f, 0.0f), glm::vec3(0.0f, 2.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			view = camera.GetViewMatrix();
			globalUniformData.mvp = projectionMatrix * view * glm::scale(glm::mat4(1), scale);
			globalUniformData.vp = projectionMatrix * view;
			globalUniformData.projectionMatrix = projectionMatrix;
			globalUniformData.worldToCamMatrix = camera.transform.modelMatrix;
			globalUniformData.worldToCamMatrix = view;
			globalUniformData.directionalLightDir = glm::make_vec4(directionalLightDir);
			globalUniformData.directionalLightColor = glm::vec4(1, 1, 1, 1);
			globalUniformData.directionalLightColor = glm::vec4(255.0f / 255.0f, 228.0f / 255.0f, 206.0f / 255.0f, 1);
			globalUniformData.shadowVPMatrix = shadowPassData.vp;
			globalUniformData.floatArrs[0] = currentFrame;
			CopyDataToDeviceMemory(vulkanInstance.device, globalUniformBfferObj.memory, globalUniformBfferObj.size, &globalUniformData);
		}

		
		uint32_t imageIndex;
		vkAcquireNextImageKHR(vulkanInstance.device, windowData.swapchain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);


		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = {imageAvailableSemaphore};
		VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

		VkSemaphore singalSemaphores[] = {renderFinishedSemaphore};
		if (vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
		{
			throw std::runtime_error("Submit queue error");
		}

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 0;
		presentInfo.pWaitSemaphores = nullptr;

		VkSwapchainKHR swapChains[1] = { windowData.swapchain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;

		vkQueuePresentKHR(presentQueue, &presentInfo);

		vkQueueWaitIdle(presentQueue);

        /* Swap front and back buffers */
        glfwSwapBuffers(windowData.window);

        /* Poll for and process events */
        glfwPollEvents();

    }

    glfwTerminate();		
}




