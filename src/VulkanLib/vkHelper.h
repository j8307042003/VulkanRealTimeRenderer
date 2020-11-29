#ifndef VULKAN_HELPER_H_
#define VULKAN_HELPER_H_

#pragma once
#include<iostream>
#include<vector>
#include <vulkan/vulkan.h>
#include "VulkanInstance.h"
#include <fstream>      // std::ifstream
// #include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
// #include <unistd.h>
#include <assert.h>

struct BufferObject
{
  VkBuffer buffer;
  VkDeviceMemory memory;
  size_t size;
  VkDescriptorBufferInfo descriptorBufInfo;
};

VkInstance CreateVulkanInstance(int layerCount, const char* const* ppLayerNames, int extensionCount, const char ** extensions) ;
bool isDeviceSuitable(VkPhysicalDevice device) ;
VkPhysicalDevice CreatePhysicalDevice(const VkInstance & instance) ;
uint32_t findQueueFamilies(VkPhysicalDevice device) ;
VkDevice CreateLogicalDevice(VkPhysicalDevice & physicalDevice, uint32_t queueFamily) ;
VkQueue CreateQueue(VkDevice & device, uint32_t queueFamily) ;
VkCommandPool createCommandPool(VkDevice & device, uint32_t & queueFamily) ;
std::vector<VkCommandBuffer> AllocateCommandBuffer(const VkDevice & device, VkCommandPool & commandPool, int num) ;
VkShaderModule createShaderModule(VkDevice & device, const std::vector<char> & code);
VkShaderModule createShaderModule(VkDevice & device, const int & codeLen, const char * code) ;
VkShaderModule createShaderModule(VkDevice & device, std::string & filename) ;
VkShaderModule createShaderModule(VkDevice & device, const char * filename);
VkPipelineShaderStageCreateInfo createShaderStageCreateInfo(const VkShaderModule & shaderModule, VkShaderStageFlagBits shaderStageFlagBits) ;
VkDescriptorSetLayoutBinding createDescriptorSetLayoutBinding(VkDescriptorType descriptorType, int bindingNum) ;
VkDescriptorSetLayoutBinding createDescriptorSetLayoutBinding(VkDescriptorType descriptorType, int bindingNum, VkShaderStageFlags shaderStageFlags, int descriptorCount = 1) ;
VkDescriptorSetLayout createDescriptorSetLayout(const VkDevice & device, int bindingNum, const VkDescriptorSetLayoutBinding* pBindings) ;
VkPipelineLayout createPipelineLayout(const VkDevice & device, int layoutCount, const VkDescriptorSetLayout * pSetLayouts) ;
VkPipelineCache createPipelineCache(const VkDevice & device) ;
void createComputePipeline(VkDevice & device, const VkPipelineLayout & pipelineLayout, const VkPipelineShaderStageCreateInfo & pipelineStageCreateInfo, VkPipeline & pipeline, VkPipelineCache & pipelineCache) ;
void createComputePipeline(VkDevice & device, const VkPipelineLayout & pipelineLayout, const VkShaderModule & shaderModule, VkShaderStageFlagBits shaderStageFlagBits, VkPipeline & pipeline, VkPipelineCache & pipelineCache) ;
VkFence createComputeShaderFence(VkDevice & device) ;
void AddComputeShaderCommand(
	int x,
	int y, 
	int z,
	VkDevice & device,
	VkCommandBuffer & commandBuffer, 
	VkPipeline & pipeline, 
	VkPipelineLayout & pipelineLayout,
	const VkDescriptorSet * pDescriptorSets
);

void submitCommand(VkDevice & device, VkCommandBuffer & commandBuffer, VkQueue & queue);
VkDescriptorPoolSize createDescriptorPoolSize(int descriptorCount, VkDescriptorType type);
VkDescriptorPool createDescriptorPool(VkDevice & device, int poolSize, const VkDescriptorPoolSize * pDescriptorPoolSize, int descriptorSetNum);
void AllocateDescriptorSets(VkDevice & device, VkDescriptorPool & descriptorPool, int descriptorSetNum, const VkDescriptorSetLayout * pDescriptorLayout, VkDescriptorSet * pDescriptorSets );
VkBool32 getMemoryType(VkPhysicalDeviceMemoryProperties deviceMemoryProperties, uint32_t typeBits, VkFlags properties, uint32_t * typeInde);
void createBuffer(const VkDevice & device, const VkPhysicalDeviceMemoryProperties & deviceMemProps, const VkBufferCreateInfo & bufCreateInfo, VkFlags usage, VkBuffer & buffer, VkDeviceMemory & deviceMem);
VkWriteDescriptorSet createWriteDescriptorSet(const VkDescriptorSet & descriptorSet, const VkDescriptorType & descriptorType, int bufferInfoNum, const VkDescriptorBufferInfo * pBufferInfo, int binding);
VkWriteDescriptorSet createWriteDescriptorSet(const VkDescriptorSet & descriptorSet, const VkDescriptorType & descriptorType, int bufferInfoNum, const VkDescriptorImageInfo * pImageInfo, int binding);
VkDescriptorBufferInfo createDescriptorBufferInfo(VkBuffer & buffer, int range);
VkDescriptorImageInfo createDescriptorImageInfo(const VkSampler & sampler, const VkImageView & imageView, const VkImageLayout & imageLayout);
void CopyDataToDeviceMemory(const VkDevice & device, const VkDeviceMemory & mem, size_t size, void * pData);
BufferObject BuildBuffer(const VkDevice & device, const VkPhysicalDeviceMemoryProperties & deviceMemProps, VkBufferUsageFlags usafeFlag, size_t size);
BufferObject BuildBuffer(const VkDevice & device, const VkPhysicalDeviceMemoryProperties & deviceMemProps, VkBufferUsageFlags usafeFlag, size_t size, VkFlags usage);
VkBool32 getSupportedDepthFormat(VkPhysicalDevice physicalDevice, VkFormat *depthFormat);
void CopyToImageBuffer(VkCommandPool commandPool, VkBuffer sourceBuffer, VkImage dstImage, int width, int height);
void transitImageLayout(VkCommandPool commandPool, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, int mipmapsCount = 1);
void submitCommandBuffer(VkCommandBuffer commandBuffer);
void LoadVkImage(const char * filename, const VkCommandPool & commandPool, VkImage & image, VkImageView & imageView);
void LoadVkImageMipmap(const char * filename, const VkCommandPool & commandPool, VkImage & image, VkImageView & imageView, int & mipmapsLevel);
void MakeMipmaps(VkImage image, VkFormat format, VkCommandPool commandPool, int width, int height, int mipmaps);

#endif