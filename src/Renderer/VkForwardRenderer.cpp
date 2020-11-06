#include "VkRealTimeRenderer.h"
#include<stdlib.h>
#include<stdio.h>
#include<iostream>
#include "VulkanLib/vkHelper.h"
#include "VulkanLib/VulkanInstance.h"
#include<vector>
#include<set>
#define GL_SILENCE_DEPRECATION
#include "GLFW/include/GLFW/glfw3.h"



VkForwardRenderer::VkForwardRenderer()
{
	screenSetting.x = 1280;
	screenSetting.y = 900;


	initSwapchain();
	initRenderPass();
	initFrameBuffer();
}

VkForwardRenderer::~VkForwardRenderer()
{

}


void VkForwardRenderer::initSwapchain()
{
	if (windowData.window == nullptr) {
		initWindow();
	}

	// make sure vulkan instance created.
	checkAndCreateVulkanInstance();



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
	windowData.surfaceFormat = chooseSwapSurfaceFormat(surfaceFormats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(presentModes);
	VkExtent2D extent = chooseSwapExtent(capabilities, width, height);
	swapChainExtent = extent;
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


	vkResult = vkCreateSwapchainKHR(vulkanInstance.device, &createInfo, nullptr, &windowData.swapchain);
	if (vkResult != VK_SUCCESS) {
		throw std::runtime_error("failed to create swap chain!");
	}
}


void VkForwardRenderer::initWindow()
{
    if (!glfwInit())
        return;

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    windowData.window = glfwCreateWindow(screenSetting.x, screenSetting.y, "Vk Forward Renderer Window", NULL, NULL);

	VkResult vkResult = glfwCreateWindowSurface(vulkanInstance.instance, windowData.window, nullptr, &windowData.surface);
	if (vkResult != VK_SUCCESS) {
		throw std::runtime_error("failed to create window surface!");
		return;
	}
}


void VkForwardRenderer::initFrameBuffer()
{


	// create image view
	std::vector<VkImage> swapchainImgs = {};
	vkGetSwapchainImagesKHR(vulkanInstance.device, swapchain, &imageCount, nullptr);
	swapchainImgs.resize(imageCount);
	vkGetSwapchainImagesKHR(vulkanInstance.device, swapchain, &imageCount, swapchainImgs.data());

	windowData.swapchainImageViews = {};
	windowData.swapchainImageViews.resize(swapchainImgs.size());

	for (int i = 0; i < swapchainImgs.size(); ++i) {
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		createInfo.image = swapchainImgs[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = surfaceFormat.format;
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

	createImg(screenSetting.x, screenSetting.y, surfaceFormat.format, colorImg);
	createImg(screenSetting.x, screenSetting.y, surfaceFormat.format, depthImg);



	//frame buffer create set up
	VkImageView fbAttachments[frameBuffer_Max];

	VkFramebufferCreateInfo framebufferInfo = {};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = mainRenderPass;
	framebufferInfo.attachmentCount = 3;
	framebufferInfo.pAttachments = fbAttachments;
	framebufferInfo.width = extent.width;
	framebufferInfo.height = extent.height;
	framebufferInfo.layers = 1;



	frameBuffers = {};
	frameBuffers.resize(windowData.swapchainImageViews.size());
	for (int i = 0; i < frameBuffers.size(); ++i) 
	{
		fbAttachments[frameBuffer_Swapchain] = windowData.swapchainImageViews[i]; 
		fbAttachments[frameBuffer_Color] = colorImg.view;
		fbAttachments[frameBuffer_Depth] = depthImg.view; 


		if (vkCreateFramebuffer(vulkanInstance.device, &framebufferInfo, nullptr, &frameBuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create frame buffer!");
		}		
	}
}


void VkForwardRenderer::initRenderPass()
{
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
	swapchainAttachment.format = surfaceFormat.format;
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


	VkSubpassDescription subpasses[2];
	// forward subpass
	VkSubpassDescription subpass{};
	subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpasses[0].colorAttachmentCount = 1;
	subpasses[0].pColorAttachments = &colorAttachmentRef;
	subpasses[0].pDepthStencilAttachment = &depthAttachmentRef;

	//swapchain subpass
	VkAttachmentReference inputAttachmentInputs[2];
	inputAttachmentInputs[0].attachment = 1;
	inputAttachmentInputs[0].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	inputAttachmentInputs[1].attachment = 2;
	inputAttachmentInputs[1].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    subpasses[1].inputAttachmentCount = 2;
    subpasses[1].pInputAttachments = inputAttachmentInputs;	
	subpasses[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;


	mainRenderPass = {};
	VkPipelineLayout pipelineLayout;
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDes.size());
	renderPassInfo.pAttachments = &attachmentDes[0];
	renderPassInfo.subpassCount = 2;
	renderPassInfo.pSubpasses = subpasses;

	if (vkCreateRenderPass(vulkanInstance.device, &renderPassInfo, nullptr, &mainRenderPass) != VK_SUCCESS) 
	{
		throw runtime_error("create render pass error!");
	}
}


void VkForwardRenderer::initPipeline()
{
	graphicsPipeline = {};
	
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


void VkForwardRenderer::createImg(int width, int height, VkFormat format, VkEngine::Img & img)
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
    createInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

	if (vkCreateImage(vulkanInstance.device, &createInfo, nullptr, &img.image) != VK_SUCCESS)
	{
		throw std::runtime_error("Create vk image error!");
	}

	VkImageViewCreateInfo createViewInfo{};
	createViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	createViewInfo.image = img.image;
	createViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createViewInfo.format = format;
	createViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	createViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	createViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	createViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	createViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	createViewInfo.subresourceRange.baseMipLevel = 0;
	createViewInfo.subresourceRange.levelCount = 1;
	createViewInfo.subresourceRange.baseArrayLayer = 0;
	createViewInfo.subresourceRange.layerCount = 1;

	if (vkCreateImageView(vulkanInstance.device, &createInfo, nullptr, &img.view) != VK_SUCCESS) {
		throw std::runtime_error("failed to create swap chain view!");
	}

	img.format = format;

}

void VkForwardRenderer::render()
{

}




