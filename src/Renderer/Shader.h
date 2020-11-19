
#include "VulkanLib/vkHelper.h"

#ifndef SHADER_HEADER
#define SHADER_HEADER

class Shader {
public:
	Shader(){}
	Shader(VkDevice & device, const char * vertexName, const char * fragmentName);
	~Shader();

private:
	VkShaderModule vertexShader;
	VkShaderModule fragmentShader;
	VkDevice device;
public:
	VkPipeline BuildPipeline(const VkRenderPass & renderpass, const int & subpass, const VkPipelineLayout & pipelineLayout,
							const VkPipelineVertexInputStateCreateInfo * pCustomVertexInputState = nullptr) const;

private:
	inline VkPipelineShaderStageCreateInfo makeShaderStageCreateInfo(VkShaderStageFlagBits shaderStage, const VkShaderModule & shaderModule) const;

};

#endif // !SHADERHEADER