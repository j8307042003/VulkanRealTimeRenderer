#ifndef VULKAN_INSTANCE_H_
#define VULKAN_INSTANCE_H_
#pragma once

namespace VulkanLib { class VulkanInstance; }

#include "vkHelper.h"
#include <vulkan/vulkan.h>
#include <vector>

typedef unsigned int uint;

typedef enum ShaderAttribute {
	Shader_Attribute_Unifrom = 0x00000001,
	Shader_Attribute_Buffer = 0x00000002,
} ShaderAttribute; 
typedef uint32_t ShaderAttri;


VkDescriptorType GetDescriptorType(ShaderAttri & attributeType);

namespace VulkanLib {
struct shaderAttribute {
	ShaderAttri attribute;
	size_t size;
	uint binding;
	void * pData;
};

struct PipelineObj {
	VkPipeline pipeline;
	VkPipelineCache pipelineCache;
	VkDescriptorSet descriptorSet;
	VkPipelineLayout pipelineLayout;
};

class VulkanInstance {

public:
	VulkanInstance();
	VulkanInstance(int extensionCount, const char ** extensions);



public:
	VkInstance instance;
  	VkPhysicalDevice physicalDevice;
  	VkPhysicalDeviceMemoryProperties deviceMemProps;
  	uint32_t queueFamily;
  	VkDevice device;
  	VkDescriptorSet descriptorSet;
	VkDescriptorPool descriptorPool;


public:
	PipelineObj BuildComputeShaderPipeline(std::vector<ShaderAttri> shaderAttributes, const VkShaderModule & shaderModule);

private:
	void InitDescriptorSet();

};
}
#endif
