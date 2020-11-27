#include "vkHelper.h"
#include <iostream>
#include <vector>
#include <stdlib.h>
#include <algorithm>
#include <vulkan/vulkan.h>
#include "Renderer/vkSys.h"
#include <fstream>      // std::ifstream
// #include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
// #include <unistd.h>

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"

VkInstance CreateVulkanInstance(int layerCount, const char* const* ppLayerNames, int extensionCount, const char ** extensions) {
	VkInstance instance;
    VkInstanceCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    info.enabledLayerCount = layerCount;
    info.ppEnabledLayerNames = ppLayerNames;
	info.enabledExtensionCount = extensionCount;
	info.ppEnabledExtensionNames = extensions;

    VkResult result = vkCreateInstance(&info, NULL, &instance);
    assert(result == VK_SUCCESS);

    return instance;
}

bool isDeviceSuitable(VkPhysicalDevice device) {
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);


	return deviceFeatures.tessellationShader;
}


VkPhysicalDevice CreatePhysicalDevice(const VkInstance & instance) {
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
	if (deviceCount == 0){
		throw std::runtime_error("failed to find gpu with vulkan support");
	}
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());	



	for (const auto & device : devices) {
		if (isDeviceSuitable(device)){
			physicalDevice = device;
			break;
		}
	}

	if (physicalDevice == VK_NULL_HANDLE) {
		throw std::runtime_error("failed to find suitable gpu");
	}

	return physicalDevice;
}



uint32_t findQueueFamilies(VkPhysicalDevice device) {
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	uint32_t result = 0;
	bool everSet = false;
	for (const auto& queueFamily : queueFamilies) {
	    if ((queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) && (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
	        result = i;
	        everSet = true;
	        //std::cout << "Success!" << std::endl;
	    }
	
	    i++;
	}	

	if (!everSet) {
		throw std::runtime_error("no suitable queue family found");
	}

	return result;
}


VkDevice CreateLogicalDevice(VkPhysicalDevice & physicalDevice, uint32_t queueFamily) {
	VkDevice device;

	VkDeviceQueueCreateInfo queueCreateInfo = {};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = queueFamily;
	queueCreateInfo.queueCount = 1;
	float queuePriority = 1.0f;
	queueCreateInfo.pQueuePriorities = &queuePriority;

	VkPhysicalDeviceFeatures deviceFeatures = {};

	std::vector<char *> extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = &queueCreateInfo;
	createInfo.queueCreateInfoCount = 1;

	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = extensions.size();
	createInfo.ppEnabledExtensionNames = extensions.data();
    createInfo.enabledLayerCount = 0;

	VkResult result = vkCreateDevice(physicalDevice, &createInfo, nullptr, &device);
    if (result != VK_SUCCESS) {
    	throw std::runtime_error("failed to create logical device!");
    }

    return device;
}

VkQueue CreateQueue(VkDevice & device, uint32_t queueFamily) {
	VkQueue graphicsQueue;
	vkGetDeviceQueue(device, queueFamily, 0, &graphicsQueue);
	return graphicsQueue;
}

VkCommandPool createCommandPool(VkDevice & device, uint32_t & queueFamily) {
	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = queueFamily;
	poolInfo.flags = 0;

	VkCommandPool commandPool;
	if(vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create command pool!");
	}

	return commandPool;
}

std::vector<VkCommandBuffer> AllocateCommandBuffer(const VkDevice & device, VkCommandPool & commandPool, int num) {
	std::vector<VkCommandBuffer> commandBuffers(num);	
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t) commandBuffers.size();

	if(vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers!");
	}

	return commandBuffers;
}


VkShaderModule createShaderModule(VkDevice & device, const std::vector<char> & code) {
	/*
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.flags = 0;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module");
	}
	*/


	return createShaderModule(device, code.size(), code.data());	
}

VkShaderModule createShaderModule(VkDevice & device, const int & codeLen, const char * code) {
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.flags = 0;
	createInfo.codeSize = codeLen;
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code);

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module");
	}


	return shaderModule;
}


VkShaderModule createShaderModule(VkDevice & device, std::string & filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    size_t 	filesize = (size_t) file.tellg();
    std::vector<char> buffer(filesize);
    file.seekg(0);
    file.read(buffer.data(), filesize);
    file.close();

    return createShaderModule(device, buffer);	
}


VkShaderModule createShaderModule(VkDevice & device, const char * filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    size_t 	filesize = (size_t) file.tellg();
    std::vector<char> buffer(filesize);
    file.seekg(0);
    file.read(buffer.data(), filesize);
    file.close();

    return createShaderModule(device, buffer);	
}


VkPipelineShaderStageCreateInfo createShaderStageCreateInfo(const VkShaderModule & shaderModule, VkShaderStageFlagBits shaderStageFlagBits) {
	VkPipelineShaderStageCreateInfo shaderCreateInfo = {};
	shaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	// shaderCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	shaderCreateInfo.stage = shaderStageFlagBits;
	shaderCreateInfo.module = shaderModule;
	shaderCreateInfo.pName = "main";
	shaderCreateInfo.flags = 0;

	return shaderCreateInfo;
}

VkDescriptorSetLayoutBinding createDescriptorSetLayoutBinding(VkDescriptorType descriptorType, int bindingNum) {
	VkDescriptorSetLayoutBinding layoutBinding = {};
	layoutBinding.binding = bindingNum;
	layoutBinding.descriptorType = descriptorType;
	layoutBinding.descriptorCount = 1;
	layoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

	return layoutBinding;
}

VkDescriptorSetLayoutBinding createDescriptorSetLayoutBinding(VkDescriptorType descriptorType, int bindingNum, VkShaderStageFlags shaderStageFlags, int descriptorCount)
{
	VkDescriptorSetLayoutBinding layoutBinding = {};
	layoutBinding.binding = bindingNum;
	layoutBinding.descriptorType = descriptorType;
	layoutBinding.descriptorCount = descriptorCount;
	layoutBinding.stageFlags = shaderStageFlags;

	return layoutBinding;	
}


VkDescriptorSetLayout createDescriptorSetLayout(const VkDevice & device, int bindingNum, const VkDescriptorSetLayoutBinding* pBindings) {
	VkDescriptorSetLayoutCreateInfo descriptorLayoutInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
	descriptorLayoutInfo.bindingCount = bindingNum;
	descriptorLayoutInfo.pBindings = pBindings;
    
    VkDescriptorSetLayout descriptorLayout;
	if (vkCreateDescriptorSetLayout(device, &descriptorLayoutInfo, nullptr, &descriptorLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout!");
	}

	return descriptorLayout;
}

VkPipelineLayout createPipelineLayout(const VkDevice & device, int layoutCount, const VkDescriptorSetLayout * pSetLayouts) {
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = layoutCount;
	pipelineLayoutCreateInfo.pSetLayouts = pSetLayouts;


	VkPushConstantRange pcr = {};
	pcr.size = 4*4*4;
	pcr.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
	pipelineLayoutCreateInfo.pPushConstantRanges = &pcr;


	VkPipelineLayout pipelineLayout;
	if (vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create pipeline layout");
	}

	return pipelineLayout;
}

VkPipelineCache createPipelineCache(const VkDevice & device) {
	VkPipelineCache pipelineCache;
	VkPipelineCacheCreateInfo pipelineCacheInfo = {};
	pipelineCacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	pipelineCacheInfo.flags = 0;
	vkCreatePipelineCache(device, &pipelineCacheInfo, nullptr, &pipelineCache);

	return pipelineCache;
}


void createComputePipeline(VkDevice & device, const VkPipelineLayout & pipelineLayout, const VkPipelineShaderStageCreateInfo & pipelineStageCreateInfo, VkPipeline & pipeline, VkPipelineCache & pipelineCache) {
	VkComputePipelineCreateInfo pipelineCreateInfo = {};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.stage = pipelineStageCreateInfo;
	pipelineCreateInfo.layout = pipelineLayout;
	pipelineCreateInfo.flags  = 0;


	pipelineCache = createPipelineCache(device);
	if (vkCreateComputePipelines(device, pipelineCache, 1, &pipelineCreateInfo, nullptr, &pipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create compute pipeline");
	}
	//std::cout << "Create compute pipeline Success!" << std::endl;
}

void createComputePipeline(VkDevice & device, const VkPipelineLayout & pipelineLayout, const VkShaderModule & shaderModule, VkShaderStageFlagBits shaderStageFlagBits, VkPipeline & pipeline, VkPipelineCache & pipelineCache) {
	VkPipelineShaderStageCreateInfo shaderStageInfo = createShaderStageCreateInfo(shaderModule, shaderStageFlagBits);
	createComputePipeline(device, pipelineLayout, shaderStageInfo, pipeline, pipelineCache);
}

VkFence createComputeShaderFence(VkDevice & device) {
	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = 0;
	VkFence fence;
	if (vkCreateFence(device, &fenceInfo, nullptr, &fence) != VK_SUCCESS) {
		throw std::runtime_error("failed to create fence!");
	}

	return fence;
}


void AddComputeShaderCommand(
	int x,
	int y, 
	int z,
	VkDevice & device,
	VkCommandBuffer & commandBuffer, 
	VkPipeline & pipeline, 
	VkPipelineLayout & pipelineLayout,
	const VkDescriptorSet * pDescriptorSets
)
{
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0;
	beginInfo.pInheritanceInfo = nullptr;

	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1, pDescriptorSets, 0, 0);
	vkCmdDispatch(commandBuffer, x, y, z);
	vkEndCommandBuffer(commandBuffer);
}

void submitCommand(VkDevice & device, VkCommandBuffer & commandBuffer, VkQueue & queue) 
{
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	VkFence fence = createComputeShaderFence(device);
	if(vkQueueSubmit(queue, 1, &submitInfo, fence) != VK_SUCCESS) {
		throw std::runtime_error("submit failed!");
	}
	
	//vkWaitForFences(device, 1, &fence, true, 1000 * 1000 * 1000);
	VkResult result = vkQueueWaitIdle(queue);



	vkDestroyFence(device, fence, nullptr);
}


VkDescriptorPoolSize createDescriptorPoolSize(int descriptorCount, VkDescriptorType type) {
	VkDescriptorPoolSize descriptorPoolSize = {};
	descriptorPoolSize.type = type;
	descriptorPoolSize.descriptorCount = descriptorCount;
	return descriptorPoolSize;
}

VkDescriptorPool createDescriptorPool(VkDevice & device, int poolSize, const VkDescriptorPoolSize * pDescriptorPoolSize, int descriptorSetNum) {
	VkDescriptorPoolCreateInfo descriptorPoolInfo = {};
	descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolInfo.pNext = NULL;
	descriptorPoolInfo.poolSizeCount = poolSize;
	descriptorPoolInfo.pPoolSizes = pDescriptorPoolSize;
	descriptorPoolInfo.maxSets = descriptorSetNum;

	VkDescriptorPool descriptorPool;

	VkResult result = vkCreateDescriptorPool(device, &descriptorPoolInfo, nullptr, &descriptorPool);
	assert(result == VK_SUCCESS);
	return descriptorPool;
}

void AllocateDescriptorSets(VkDevice & device, VkDescriptorPool & descriptorPool, int descriptorSetNum, const VkDescriptorSetLayout * pDescriptorLayout, VkDescriptorSet * pDescriptorSets ) {
	VkDescriptorSetAllocateInfo allocInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = descriptorSetNum;
	allocInfo.pSetLayouts = pDescriptorLayout;

	VkResult result = vkAllocateDescriptorSets(device, &allocInfo, pDescriptorSets);
	assert(result == VK_SUCCESS);
}

VkBool32 getMemoryType(VkPhysicalDeviceMemoryProperties deviceMemoryProperties, uint32_t typeBits, VkFlags properties, uint32_t * typeIndex)
{
	for (uint32_t i = 0; i < 32; i++)
	{
		if ((typeBits & 1) == 1)
		{
			if ((deviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				*typeIndex = i;
				return true;
			}
		}
		typeBits >>= 1;
	}
	return false;
}

void createBuffer(const VkDevice & device, const VkPhysicalDeviceMemoryProperties & deviceMemProps, const VkBufferCreateInfo & bufCreateInfo, VkFlags usage, VkBuffer & buffer, VkDeviceMemory & deviceMem) {
	VkResult result;
	VkMemoryRequirements memReqs;
	VkMemoryAllocateInfo memAllocInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};

	result = vkCreateBuffer(device, &bufCreateInfo, nullptr, &buffer);
	vkGetBufferMemoryRequirements(device, buffer, &memReqs);
	memAllocInfo.allocationSize = memReqs.size;
	//std::cout << "allocate size " << memReqs.size << std::endl;
	getMemoryType(deviceMemProps, memReqs.memoryTypeBits, usage, &memAllocInfo.memoryTypeIndex);
	result = vkAllocateMemory(device, &memAllocInfo, nullptr, &deviceMem);
	assert(result == VK_SUCCESS);
}


//VkDeviceMemory AllocateDeviceMemory(size_t size) {
//	VkMemoryAllocateInfo memAllocInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
//	memAllocInfo.allocationSize = size;
//	getMemoryType(deviceMemProps, memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &memAllocInfo.memoryTypeIndex);
//	result = vkAllocateMemory(device, &memAllocInfo, nullptr, &memory);		
//}


VkWriteDescriptorSet createWriteDescriptorSet(const VkDescriptorSet & descriptorSet, const VkDescriptorType & descriptorType, int bufferInfoNum, const VkDescriptorBufferInfo * pBufferInfo, int binding) {
	VkWriteDescriptorSet writeDescriptorSet = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
	writeDescriptorSet.dstSet = descriptorSet;
	writeDescriptorSet.descriptorType = descriptorType;
	writeDescriptorSet.dstBinding = binding;
	writeDescriptorSet.descriptorCount = bufferInfoNum;	// poolSize.type = VK_DESCRITPTOR_TYPE_UNIFORM_BUFFER;
	writeDescriptorSet.pBufferInfo = pBufferInfo;
	return writeDescriptorSet;
}

VkWriteDescriptorSet createWriteDescriptorSet(const VkDescriptorSet & descriptorSet, const VkDescriptorType & descriptorType, int bufferInfoNum, const VkDescriptorImageInfo * pImageInfo, int binding)
{
	VkWriteDescriptorSet writeDescriptorSet = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
	writeDescriptorSet.dstSet = descriptorSet;
	writeDescriptorSet.descriptorType = descriptorType;
	writeDescriptorSet.dstBinding = binding;
	writeDescriptorSet.descriptorCount = bufferInfoNum;
	writeDescriptorSet.pImageInfo = pImageInfo;
	return writeDescriptorSet;	
}

VkDescriptorBufferInfo createDescriptorBufferInfo(VkBuffer & buffer, int range) {
	VkDescriptorBufferInfo descriptor = {};
	descriptor.buffer = buffer;
	descriptor.range = range;
	return descriptor;
}

VkDescriptorImageInfo createDescriptorImageInfo(const VkSampler & sampler, const VkImageView & imageView, const VkImageLayout & imageLayout)
{
	VkDescriptorImageInfo dstImgInfo = {};
    dstImgInfo.sampler = sampler;
    dstImgInfo.imageView = imageView;
    dstImgInfo.imageLayout = imageLayout;	

	return dstImgInfo;
}



void CopyDataToDeviceMemory(const VkDevice & device, const VkDeviceMemory & mem, size_t size, void * pData) {
  void * pMemData;
  vkMapMemory(device, mem, 0, size, 0, &pMemData);
  memcpy(pMemData, pData, size);
  vkUnmapMemory(device, mem);	
}



BufferObject BuildBuffer(const VkDevice & device, const VkPhysicalDeviceMemoryProperties & deviceMemProps, VkBufferUsageFlags usafeFlag, size_t size) {
	BufferObject bufferObj;
  	VkBufferCreateInfo bufCreateInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
  	bufCreateInfo.usage = usafeFlag;
  	bufCreateInfo.size  = size;
  	createBuffer(device, deviceMemProps, bufCreateInfo, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, bufferObj.buffer, bufferObj.memory);
  	vkBindBufferMemory(device, bufferObj.buffer, bufferObj.memory, 0);
  	bufferObj.descriptorBufInfo = createDescriptorBufferInfo(bufferObj.buffer, size);
  	bufferObj.size = size;
  	
  	return bufferObj;
}

BufferObject BuildBuffer(const VkDevice & device, const VkPhysicalDeviceMemoryProperties & deviceMemProps, VkBufferUsageFlags usafeFlag, size_t size, VkFlags usage)
{
	BufferObject bufferObj;
  	VkBufferCreateInfo bufCreateInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
  	bufCreateInfo.usage = usafeFlag;
  	bufCreateInfo.size  = size;
  	createBuffer(device, deviceMemProps, bufCreateInfo, usage, bufferObj.buffer, bufferObj.memory);
  	vkBindBufferMemory(device, bufferObj.buffer, bufferObj.memory, 0);
  	bufferObj.descriptorBufInfo = createDescriptorBufferInfo(bufferObj.buffer, size);
  	bufferObj.size = size;
  	
  	return bufferObj;
}

VkBool32 getSupportedDepthFormat(VkPhysicalDevice physicalDevice, VkFormat *depthFormat)
{
	// Since all depth formats may be optional, we need to find a suitable depth format to use
	// Start with the highest precision packed format
	std::vector<VkFormat> depthFormats = {
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		VK_FORMAT_D32_SFLOAT,
		VK_FORMAT_D24_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM
	};

	for (auto& format : depthFormats)
	{
		VkFormatProperties formatProps;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProps);
		// Format must support depth stencil attachment for optimal tiling
		if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
		{
			*depthFormat = format;
			return true;
		}
	}

	return false;
}


void CopyToImageBuffer(VkCommandPool commandPool, VkBuffer sourceBuffer, VkImage dstImage, int width, int height)
{
	VulkanLib::VulkanInstance & vulkanInstance = *vkSys::VkInstance::GetInstance();

	auto commandBuffer = AllocateCommandBuffer(vulkanInstance.device, commandPool, 1)[0];

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;	
	vkBeginCommandBuffer(commandBuffer, &beginInfo);


	VkBufferImageCopy imgCopy = {};
    imgCopy.bufferOffset = 0; 
    imgCopy.bufferRowLength = 0; 
    imgCopy.bufferImageHeight = 0; 

	imgCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imgCopy.imageSubresource.mipLevel = 0;
	imgCopy.imageSubresource.baseArrayLayer = 0;
	imgCopy.imageSubresource.layerCount = 1;

    imgCopy.imageOffset = {0, 0, 0}; 
    imgCopy.imageExtent = {(uint32_t)width, (uint32_t)height, 1};

	vkCmdCopyBufferToImage(commandBuffer, sourceBuffer, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imgCopy);

	
	submitCommandBuffer(commandBuffer);
	vkFreeCommandBuffers(vulkanInstance.device, commandPool, 1, &commandBuffer);
}

void transitImageLayout(VkCommandPool commandPool, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	VulkanLib::VulkanInstance & vulkanInstance = *vkSys::VkInstance::GetInstance();	
	auto commandBuffer = AllocateCommandBuffer(vulkanInstance.device, commandPool, 1)[0];

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;	
	vkBeginCommandBuffer(commandBuffer, &beginInfo);	


	VkImageMemoryBarrier  barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;


VkPipelineStageFlags sourceStage;
VkPipelineStageFlags destinationStage;

if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
} else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
} else {
    throw std::invalid_argument("unsupported layout transition!");
}



    vkCmdPipelineBarrier(
    commandBuffer,
    sourceStage, destinationStage,
    0,
    0, nullptr,
    0, nullptr,
    1, &barrier
	);




	submitCommandBuffer(commandBuffer);
	vkFreeCommandBuffers(vulkanInstance.device, commandPool, 1, &commandBuffer);

}

void submitCommandBuffer(VkCommandBuffer commandBuffer)
{
	VulkanLib::VulkanInstance & vulkanInstance = *vkSys::VkInstance::GetInstance();

	vkEndCommandBuffer(commandBuffer);

	VkQueue queue = {};
	vkGetDeviceQueue(vulkanInstance.device, vulkanInstance.queueFamily, 0, &queue);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(queue);
}

void LoadVkImage(const char * filename, const VkCommandPool & commandPool, VkImage & image, VkImageView & imageView)
{

	int width, height, channels;
	stbi_uc * imageFileData = stbi_load(filename, &width, &height, &channels, STBI_rgb_alpha);

	if (imageFileData == nullptr) 
	{
		throw std::runtime_error("Load Image Failed. file not found! " + std::string(filename));
		return ;
	}

	auto & vulkanInstance = *vkSys::VkInstance::GetInstance();

	VkDeviceSize imageSize = width * height * 4;
	BufferObject imageBuffer = BuildBuffer(vulkanInstance.device, vulkanInstance.deviceMemProps, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, imageSize, 
											VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	CopyDataToDeviceMemory(vulkanInstance.device, imageBuffer.memory, imageBuffer.size, imageFileData);

	stbi_image_free(imageFileData);

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
		return ;
	}


	result = vkBindImageMemory( vulkanInstance.device, image, imgMem, 0);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("bind image error");
		return ;
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

	if (vkCreateImageView(vulkanInstance.device, &createViewInfo, nullptr, &imageView) != VK_SUCCESS) {
		throw std::runtime_error("failed to create swap chain view!");
	}	

}

void LoadVkImageMipmap(const char * filename, const VkCommandPool & commandPool, VkImage & image, VkImageView & imageView, int & mipmapsLevel)
{
	int width, height, channels;
	stbi_uc * imageFileData = stbi_load(filename, &width, &height, &channels, STBI_rgb_alpha);

	if (imageFileData == nullptr)
	{
		throw std::runtime_error("Load Image Failed. file not found! " + std::string(filename));
		return;
	}

	auto & vulkanInstance = *vkSys::VkInstance::GetInstance();


	uint32_t mipmaps = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
	mipmapsLevel = mipmaps;

	VkDeviceSize imageSize = width * height * 4;
	BufferObject imageBuffer = BuildBuffer(vulkanInstance.device, vulkanInstance.deviceMemProps, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, imageSize,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	CopyDataToDeviceMemory(vulkanInstance.device, imageBuffer.memory, imageBuffer.size, imageFileData);

	stbi_image_free(imageFileData);

	VkImageCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	createInfo.imageType = VK_IMAGE_TYPE_2D;
	createInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
	createInfo.extent.width = width;
	createInfo.extent.height = height;
	createInfo.extent.depth = 1;
	createInfo.mipLevels = mipmaps;
	createInfo.arrayLayers = 1;
	createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	createInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateImage(vulkanInstance.device, &createInfo, nullptr, &image) != VK_SUCCESS)
	{
		throw std::runtime_error("Create vk image error!");
	}

	VkMemoryRequirements memReqs;
	vkGetImageMemoryRequirements(vulkanInstance.device, image, &memReqs);
	VkMemoryAllocateInfo memallocInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
	memallocInfo.allocationSize = memReqs.size;
	getMemoryType(vulkanInstance.deviceMemProps, memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memallocInfo.memoryTypeIndex);
	VkDeviceMemory imgMem;
	VkResult result = vkAllocateMemory(vulkanInstance.device, &memallocInfo, nullptr, &imgMem);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Allocate image memory error");
		return;
	}


	result = vkBindImageMemory(vulkanInstance.device, image, imgMem, 0);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("bind image error");
		return;
	}


	transitImageLayout(commandPool, image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	CopyToImageBuffer(commandPool, imageBuffer.buffer, image, width, height);
	//transitImageLayout(commandPool, image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkDestroyBuffer(vulkanInstance.device, imageBuffer.buffer, nullptr);
	vkFreeMemory(vulkanInstance.device, imageBuffer.memory, nullptr);

	MakeMipmaps(image, VK_FORMAT_R8G8B8A8_SRGB, commandPool, width, height, mipmaps);

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
	createViewInfo.subresourceRange.levelCount = mipmaps;
	createViewInfo.subresourceRange.baseArrayLayer = 0;
	createViewInfo.subresourceRange.layerCount = 1;

	if (vkCreateImageView(vulkanInstance.device, &createViewInfo, nullptr, &imageView) != VK_SUCCESS) {
		throw std::runtime_error("failed to create swap chain view!");
	}
}

void MakeMipmaps(VkImage image, VkFormat format, VkCommandPool commandPool, int width, int height, int mipmaps)
{
	VulkanLib::VulkanInstance & vulkanInstance = *vkSys::VkInstance::GetInstance();
	auto commandBuffer = AllocateCommandBuffer(vulkanInstance.device, commandPool, 1)[0];
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;


	int mipmapWidth = width;
	int mipmapHeight = height;

	for (int i = 1; i < mipmaps; ++i)
	{
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		VkImageBlit imageBlit = {};
		imageBlit.srcOffsets[0] = {0, 0, 0};
		imageBlit.srcOffsets[1] = { mipmapWidth, mipmapHeight, 1};
		imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageBlit.srcSubresource.baseArrayLayer = 0;
		imageBlit.srcSubresource.layerCount = 1;
		imageBlit.srcSubresource.mipLevel = i - 1;
		imageBlit.dstOffsets[0] = {0, 0, 0};
		imageBlit.dstOffsets[1] = { mipmapWidth > 1 ? mipmapWidth / 2 : 1, mipmapHeight > 1 ? mipmapHeight / 2 : 1, 1};
		imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageBlit.dstSubresource.baseArrayLayer = 0;
		imageBlit.dstSubresource.layerCount = 1;
		imageBlit.dstSubresource.mipLevel = i;

		vkCmdBlitImage(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageBlit, VK_FILTER_LINEAR);

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		if (mipmapWidth > 1) mipmapWidth /= 2;
		if (mipmapHeight > 1) mipmapHeight /= 2;

	}


	barrier.subresourceRange.baseMipLevel = mipmaps - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(commandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
		0, nullptr,
		0, nullptr,
		1, &barrier);

	submitCommandBuffer(commandBuffer);
	vkFreeCommandBuffers(vulkanInstance.device, commandPool, 1, &commandBuffer);

}

