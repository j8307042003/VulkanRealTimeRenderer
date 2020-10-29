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


VkRealTimeRenderer::VkRealTimeRenderer()
{
	createSwapchain();
	createGraphicPipeline();
	createRenderCommandPool();
}


VkRealTimeRenderer::~VkRealTimeRenderer() 
{
	// vkDestroyShaderModule(vulkanInstance.device, vertexShaderModule, nullptr);
	// vkDestroyShaderModule(vulkanInstance.device, fragmentShaderModule, nullptr);
}


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
	if (capabilities.currentExtent.width != UINT32_MAX) {
		return capabilities.currentExtent;
	}
	else {
		VkExtent2D actualExtent = { width, height };

		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}



void mouse_button_callback(GLFWwindow* window, int button, int action, int mods){
    //if(action == GLFW_PRESS) {
    //    double xpos, ypos;
    //    glfwGetCursorPos(window, &xpos, &ypos);
    //    std::cout << "-----------------" << std::endl;
    //    std::cout << std::endl << "x " << xpos << ". y " << ypos << std::endl;
    //    renderer->ClearImage();
    //    std::cout << "-----------------" << std::endl;
    //}

	/*
    if (action == GLFW_PRESS){
        glfwGetCursorPos(window, &mouse_pos_x, &mouse_pos_y);
        mouse_keys[button] = true;
    }
    else if (action == GLFW_RELEASE)
        mouse_keys[button] = false;
	*/
}


// Moves/alters the camera positions based on user input
void Do_Movement()
{
	/*
	float x = 0, y = 0, z = 0;

	if (keys[GLFW_KEY_W]) {
		z += -1;
	}
	if (keys[GLFW_KEY_S]) {
		z += 1;
	}
	if (keys[GLFW_KEY_A]) {
		x += -1;
	}
	if (keys[GLFW_KEY_D]) {
		x += 1;
	}
	if (keys[GLFW_KEY_E]) {
		y += 1;
	}
	if (keys[GLFW_KEY_Q]) {
		y += -1;
	}

	if (x != 0 || y != 0 || z != 0) {
        Vec3 dir = cam.transform.TransformDir({x,y,z});
		cam.transform.position += dir * deltaTime * 4.0f;
        cam.transform.UpdateMatrix();
		renderer->ClearImage();
        std::cout << "position : " << cam.transform.position.tostring() << std::endl;

	}
	*/
}


void Do_Rotate()
{
	/*
    if (mouse_keys[GLFW_MOUSE_BUTTON_1]) {
        double now_pos_x, now_pos_y;
        glfwGetCursorPos(window, &now_pos_x, &now_pos_y);

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
        cam.transform.rotation = 
            // glm::normalize(cam.transform.rotation * glm::quat( delta_y * kRotRatio, delta_x * kRotRatio, 0, 0 ));
            glm::normalize(glm::quat(glm::radians(glm::vec3{rot_y, -rot_x, 180 })));
            // glm::normalize(cam.transform.rotation * glm::quat({delta_y * kRotRatio, delta_x * kRotRatio, 0 }));
        cam.transform.UpdateMatrix();
        renderer->ClearImage();

        std::cout << "rotation : " << rot_y << "  " << rot_x << std::endl;

    }
	*/
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	/*
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (action == GLFW_PRESS)
		keys[key] = true;
	else if (action == GLFW_RELEASE)
		keys[key] = false;
	*/
}









void VkRealTimeRenderer::createSwapchain()
{


    if (!glfwInit())
        return;

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	uint32_t count;
	auto requires = glfwGetRequiredInstanceExtensions(&count);
	std::vector<const char*> extensions = {};
	for (int i = 0; i < count; ++i) {
		extensions.push_back(requires[i]);
	}
	vulkanInstance = VulkanLib::VulkanInstance(extensions.size(), extensions.data());
	std::vector<const char*> checkExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	bool extensionSupported = checkDeviceExtensionSupport(vulkanInstance.physicalDevice, &checkExtensions);


    int width = 1280; //640;
    int height = 960; //480;

    // width = 320;
    // height = 480;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(width, height, "Renderer", NULL, NULL);

    if (!window)
    {
        glfwTerminate();
        return;
    }


	
	
	VkSurfaceKHR surface;
	VkResult vkResult = glfwCreateWindowSurface(vulkanInstance.instance, window, nullptr, &surface);
	if (vkResult != VK_SUCCESS) {
		throw std::runtime_error("failed to create window surface!");
		return;
	}



	presentQueue = {};
	vkGetDeviceQueue(vulkanInstance.device, vulkanInstance.queueFamily, 0, &presentQueue);
	VkSurfaceCapabilitiesKHR capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vulkanInstance.physicalDevice, surface, &capabilities);

	std::vector<VkSurfaceFormatKHR> surfaceFormats = {};
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(vulkanInstance.physicalDevice, surface, &formatCount, nullptr);

	if (formatCount != 0) {
		surfaceFormats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(vulkanInstance.physicalDevice, surface, &formatCount, surfaceFormats.data());
	}

	std::vector<VkPresentModeKHR> presentModes = {};
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(vulkanInstance.physicalDevice, surface, &presentModeCount, nullptr);

	if (presentModeCount != 0) {
		presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(vulkanInstance.physicalDevice, surface, &presentModeCount, presentModes.data());
	}
	surfaceFormat = chooseSwapSurfaceFormat(surfaceFormats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(presentModes);
	VkExtent2D extent = chooseSwapExtent(capabilities, width, height);
	swapChainExtent = extent;
	uint32_t imageCount = capabilities.minImageCount + 1;
	if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
		imageCount = capabilities.maxImageCount;
	}
	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
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


	swapchain = {};
	vkResult = vkCreateSwapchainKHR(vulkanInstance.device, &createInfo, nullptr, &swapchain);
	if (vkResult != VK_SUCCESS) {
		throw std::runtime_error("failed to create swap chain!");
	}

	std::vector<VkImage> swapchainImgs = {};
	vkGetSwapchainImagesKHR(vulkanInstance.device, swapchain, &imageCount, nullptr);
	swapchainImgs.resize(imageCount);
	vkGetSwapchainImagesKHR(vulkanInstance.device, swapchain, &imageCount, swapchainImgs.data());

	std::vector<VkImageView> swapchainImageViews = {};
	swapchainImageViews.resize(swapchainImgs.size());

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

		if (vkCreateImageView(vulkanInstance.device, &createInfo, nullptr, &swapchainImageViews[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create swap chain view!");
		}
	}


	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = surfaceFormat.format;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	renderPass = {};
	VkPipelineLayout pipelineLayout;
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;

	if (vkCreateRenderPass(vulkanInstance.device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}


	swapchainFrameBuffer = {};
	swapchainFrameBuffer.resize(swapchainImageViews.size());

	for (int i = 0; i < swapchainFrameBuffer.size(); ++i) {
		VkImageView attachments[1] = {
			swapchainImageViews[i]
		};


		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = extent.width;
		framebufferInfo.height = extent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(vulkanInstance.device, &framebufferInfo, nullptr, &swapchainFrameBuffer[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create frame buffer!");
		}
	}

	imageAvailableSemaphore = {};
	renderFinishedSemaphore = {};

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	if (vkCreateSemaphore(vulkanInstance.device, &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS ||
		vkCreateSemaphore(vulkanInstance.device, &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS) {

		throw std::runtime_error("failed to create semaphores!");
	}	


    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSwapInterval(1);

}



void VkRealTimeRenderer::createRenderPass()
{
}




void VkRealTimeRenderer::createGraphicPipeline()
{
	std::string filename;

	filename = "vertexColorVert.spv";
	auto vertexShaderModule = createShaderModule(vulkanInstance.device, filename);
	filename = "vertexColorFrag.spv";
	auto fragmentShaderModule = createShaderModule(vulkanInstance.device, filename);


	VkPipelineShaderStageCreateInfo vertexShaderStageInfo = {};
	vertexShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertexShaderStageInfo.module = vertexShaderModule;
	vertexShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragmentShaderModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = {vertexShaderStageInfo, fragShaderStageInfo};



	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional


	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;


	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float) swapChainExtent.width;
	viewport.height = (float) swapChainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = {0, 0};
	scissor.extent = swapChainExtent;


	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;


	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f;
	rasterizer.depthBiasClamp = 0.0f;
	rasterizer.depthBiasSlopeFactor = 0.0f;



	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f;
	multisampling.pSampleMask = nullptr;
	multisampling.alphaToCoverageEnable = VK_FALSE;
	multisampling.alphaToOneEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional
	colorBlendAttachment.blendEnable = VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;



	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional






	VkDynamicState dynamicStates[] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_LINE_WIDTH
	};


	VkPipelineDynamicStateCreateInfo dynamicStateInfo = {};
	dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateInfo.dynamicStateCount = 2;
	dynamicStateInfo.pDynamicStates = dynamicStates;




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


	VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.stageCount = 2;
	pipelineCreateInfo.pStages = shaderStages;
	pipelineCreateInfo.pVertexInputState = &vertexInputInfo;
	pipelineCreateInfo.pInputAssemblyState = &inputAssembly;
	pipelineCreateInfo.pViewportState = &viewportState;
	pipelineCreateInfo.pRasterizationState = &rasterizer;
	pipelineCreateInfo.pMultisampleState = &multisampling;
	pipelineCreateInfo.pDepthStencilState = nullptr;
	pipelineCreateInfo.pColorBlendState = &colorBlending;
	pipelineCreateInfo.pDynamicState = nullptr;
	pipelineCreateInfo.layout = pipelineLayout;
	pipelineCreateInfo.renderPass = renderPass;
	pipelineCreateInfo.subpass = 0;
	pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineCreateInfo.basePipelineIndex = -1;


	graphicsPipeline = {};

	if (vkCreateGraphicsPipelines(vulkanInstance.device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
	{
		throw std::runtime_error("create graphic pipeline error!!");
	}


}


void VkRealTimeRenderer::createRenderCommandPool()
{
	VkCommandPool commandPool = createCommandPool(vulkanInstance.device, vulkanInstance.queueFamily);
	commandBuffers = AllocateCommandBuffer(vulkanInstance.device, commandPool, swapchainFrameBuffer.size());

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

		VkRenderPassBeginInfo renderPassBeginInfo = {};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = renderPass;
		renderPassBeginInfo.framebuffer = swapchainFrameBuffer[i];
		renderPassBeginInfo.renderArea.offset = {0,0};
		renderPassBeginInfo.renderArea.extent = swapChainExtent;

		VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
		renderPassBeginInfo.clearValueCount = 1;
		renderPassBeginInfo.pClearValues = &clearColor;
		vkCmdBeginRenderPass(commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
		vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);
		vkCmdEndRenderPass(commandBuffers[i]);

		if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
		    throw std::runtime_error("failed to record command buffer!");
		}

	}



}



void VkRealTimeRenderer::StartRender()
{
	VkQueue queue = {};
	vkGetDeviceQueue(vulkanInstance.device, vulkanInstance.queueFamily, 0, &queue);  		

    while (!glfwWindowShouldClose(window))
    {
		GLfloat currentFrame = glfwGetTime();
		//deltaTime = currentFrame - lastFrame;
		//lastFrame = currentFrame;

        Do_Rotate();
		Do_Movement();
        // std::cout << "update frame" << std::endl;
        // renderer->UpdateFrame();



        //stall gpu renderer. clean up gpu usage for image display
        // renderer->Stall();

        /* Render here */

		
		uint32_t imageIndex;
		vkAcquireNextImageKHR(vulkanInstance.device, swapchain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);


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

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;		
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;		


		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 0;
		presentInfo.pWaitSemaphores = nullptr;

		VkSwapchainKHR swapChains[1] = { swapchain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;

		vkQueuePresentKHR(presentQueue, &presentInfo);

		vkQueueWaitIdle(presentQueue);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();

    }

    glfwTerminate();	
}
