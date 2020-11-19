#include "VkMaterial.h"
#include <array>

void VkMaterial::ReadMainTexture(const VulkanLib::VulkanInstance & vulkanInstance, const VkCommandPool & commandPool, const char * fileName)
{
	/*
	int width, height, channels;
	stbi_uc * imageFileData = stbi_load(fileName, &width, &height, &channels, STBI_rgb_alpha);

	if (imageFileData == nullptr) 
	{
		throw std::runtime_error("Load Image Failed. file not found!");
		return;
	}

	VkDeviceSize imageSize = width * height * 4;
	BufferObject imageBuffer = BuildBuffer(vulkanInstance.device, vulkanInstance.deviceMemProps, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, imageSize, 
											VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	CopyDataToDeviceMemory(vulkanInstance.device, imageBuffer.memory, imageBuffer.size, imageFileData);

	stbi_image_free(imageFileData);

	VkImage image;
	VkImageCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    createInfo.imageType = VK_IMAGE_TYPE_2D;
    createInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    createInfo.extent.width = width;
    createInfo.extent.height = height;
    createInfo.extent.depth = 1;
    createInfo.mipLevels = 1;
    createInfo.arrayLayers = 1;
    createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    createInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT ;
    createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateImage(vulkanInstance.device, &createInfo, nullptr, &image) != VK_SUCCESS)
	{
		throw std::runtime_error("Create vk image error!");
	}

	VkMemoryRequirements memReqs;
	vkGetImageMemoryRequirements(vulkanInstance.device, image, &memReqs);
	VkMemoryAllocateInfo memallocInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
	memallocInfo.allocationSize = memReqs.size;
	getMemoryType(vulkanInstance.deviceMemProps, memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memallocInfo.memoryTypeIndex);
	VkDeviceMemory imgMem; 
	VkResult result = vkAllocateMemory(vulkanInstance.device, &memallocInfo, nullptr, &imgMem);
	if (result != VK_SUCCESS) 
	{
		throw std::runtime_error("Allocate image memory error");
		return;
	}


	result = vkBindImageMemory( vulkanInstance.device, image, imgMem, 0);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("bind image error");
		return;
	}


	transitImageLayout(commandPool, image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	CopyToImageBuffer(commandPool, imageBuffer.buffer, image, width, height);
	transitImageLayout(commandPool, image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkDestroyBuffer(vulkanInstance.device, imageBuffer.buffer, nullptr);
    vkFreeMemory(vulkanInstance.device, imageBuffer.memory, nullptr);


	VkImageAspectFlags aspect = 0;
	if (createInfo.usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
	{
		aspect = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	if (createInfo.usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
	{
		aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
	}



	VkImageViewCreateInfo createViewInfo{};
	createViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createViewInfo.image = image;
	createViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createViewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
	createViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	createViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	createViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	createViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	createViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	createViewInfo.subresourceRange.baseMipLevel = 0;
	createViewInfo.subresourceRange.levelCount = 1;
	createViewInfo.subresourceRange.baseArrayLayer = 0;
	createViewInfo.subresourceRange.layerCount = 1;

	VkImageView imageView = {};
	if (vkCreateImageView(vulkanInstance.device, &createViewInfo, nullptr, &imageView) != VK_SUCCESS) {
		throw std::runtime_error("failed to create swap chain view!");
	}
	

	mainTexImg = image;
	mainTexImageView = imageView;
	*/

	LoadVkImage(fileName, commandPool, mainTexImg, mainTexImageView);
}

void VkMaterial::SetTexture(vkSys::TexMgr::Texture texture)
{
	mainTexImg = texture.image;
	mainTexImageView = texture.imageView;	
}



VkMaterialProgram vkMatBuildProgram(const VkMaterial & mat, VkDevice device, VkRenderPass renderpass, int subpass, const VkPipelineVertexInputStateCreateInfo * vertexInputInfo)
{
	VkMaterialProgram program = {};

	std::array<VkDescriptorSetLayoutBinding, 3> setLayoutBindings = 
	{
		createDescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, VK_SHADER_STAGE_VERTEX_BIT), // global data
		createDescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT),  // custom data
		createDescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, VK_SHADER_STAGE_FRAGMENT_BIT)  // mainTexture
	};

	program.descriptorSetLayout = createDescriptorSetLayout(device, 3, &setLayoutBindings[0]);
	program.pipelinelayout = createPipelineLayout(device, 1, &program.descriptorSetLayout);
	program.pipeline = mat.shader.BuildPipeline(renderpass, subpass, program.pipelinelayout, vertexInputInfo);
	program.renderpass = renderpass;
	program.subpass = subpass;
	program.pMat = &mat;

	return program;
}
