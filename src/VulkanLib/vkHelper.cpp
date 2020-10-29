#include "vkHelper.h"
#include<iostream>
#include<vector>
#include <vulkan/vulkan.h>
#include <fstream>      // std::ifstream
// #include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
// #include <unistd.h>



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

    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
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

std::vector<VkCommandBuffer> AllocateCommandBuffer(VkDevice & device, VkCommandPool & commandPool, int num) {
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
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.flags = 0;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

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

VkDescriptorBufferInfo createDescriptorBufferInfo(VkBuffer & buffer, int range) {
	VkDescriptorBufferInfo descriptor = {};
	descriptor.buffer = buffer;
	descriptor.range = range;
	return descriptor;
}


void CopyDataToDeviceMemory(VkDevice & device, VkDeviceMemory & mem, size_t size, void * pData) {
  void * pMemData;
  vkMapMemory(device, mem, 0, size, 0, &pMemData);
  memcpy(pMemData, pData, size);
  vkUnmapMemory(device, mem);	
}



BufferObject BuildBuffer(VkDevice & device, const VkPhysicalDeviceMemoryProperties & deviceMemProps, VkBufferUsageFlags usafeFlag, size_t size) {
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

