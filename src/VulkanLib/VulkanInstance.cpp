#include "VulkanInstance.h"
#include "vkHelper.h"
#include <vulkan/vulkan.h>
#include <vector>

VkDescriptorType GetDescriptorType(ShaderAttri & attributeType) {
	switch(attributeType) {
		case Shader_Attribute_Unifrom: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		case Shader_Attribute_Buffer: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	}

	return VK_DESCRIPTOR_TYPE_MAX_ENUM;
}

using namespace VulkanLib;

VulkanInstance::VulkanInstance() {
	VulkanInstance(0, nullptr);
}

VulkanInstance::VulkanInstance(int extensionCount, const char ** extensions) {
	
  const char * names[] = {
      "VK_LAYER_KHRONOS_validation",
      "VK_LAYER_LUNARG_standard_validation",
      "VK_LAYER_LUNARG_api_dump",
  };

  instance = CreateVulkanInstance(0, names, extensionCount, extensions);
  // instance = CreateVulkanInstance(1, names);

  physicalDevice = CreatePhysicalDevice(instance);
  vkGetPhysicalDeviceMemoryProperties(physicalDevice, &deviceMemProps);  
  queueFamily = findQueueFamilies(physicalDevice);
  device = CreateLogicalDevice(physicalDevice, queueFamily);
  InitDescriptorSet();	
}



VulkanLib::PipelineObj VulkanLib::VulkanInstance::BuildComputeShaderPipeline(std::vector<ShaderAttri> shaderAttributes, const VkShaderModule & shaderModule) {
	int attributeCount = shaderAttributes.size();


	// build desciptor set layout
  	VkDescriptorSetLayoutBinding * layoutBindings = new VkDescriptorSetLayoutBinding[attributeCount];
	for( int i = 0; i < attributeCount; ++i) {
		VkDescriptorType descriptorType = GetDescriptorType(shaderAttributes[i]);
	    layoutBindings[i] = createDescriptorSetLayoutBinding(descriptorType, i);
	}

	VkDescriptorSetLayout setLayout = createDescriptorSetLayout(device, attributeCount, &layoutBindings[0]);
	VkPipelineLayout pipelineLayout = createPipelineLayout(device, 1, &setLayout);

	delete[] layoutBindings;

	VkPipeline compPipeline;
  	VkPipelineCache pipelineCache;
  	createComputePipeline(device, pipelineLayout, shaderModule, VK_SHADER_STAGE_COMPUTE_BIT, compPipeline, pipelineCache);

	VkDescriptorSet descriptorSet;
	AllocateDescriptorSets(device, descriptorPool, 1, &setLayout, &descriptorSet);


	PipelineObj pipeline = 
	{
		compPipeline,	// VkPipeline pipeline;
		pipelineCache,	// VkPipelineCache pipelineCache;
		descriptorSet,	// VkDescritorSet descriptorSet;
		pipelineLayout, // VkPipelineLayout pipelineLayout;		
	};

	return pipeline;
}



void VulkanLib::VulkanInstance::InitDescriptorSet() {
	const int kPoolSize = 128;
	VkDescriptorPoolSize storagePoolsize = createDescriptorPoolSize(kPoolSize, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	VkDescriptorPoolSize uniformPoolsize = createDescriptorPoolSize(kPoolSize, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

	VkDescriptorPoolSize descriptorPoolSizes[2] = {storagePoolsize, uniformPoolsize};
	descriptorPool = createDescriptorPool(device, 2, &descriptorPoolSizes[0], 2);
}

