
#include "VulkanLib/vkHelper.h"
#include "SPIRV-Reflect/spirv_reflect.h"
#include <memory>
#include "vkSys.h"

#ifndef SHADER_HEADER
#define SHADER_HEADER

class Shader {
public:
	Shader(){}
	Shader(VkDevice & device, const char * vertexName, const char * fragmentName);
	~Shader();

	std::vector<SpvReflectDescriptorBinding*> vertexVariables;
	std::vector<SpvReflectDescriptorBinding*> fragmentVariables;

private:

	class ShaderModules
	{
	public:
		VkShaderModule vertexShader;
		VkShaderModule fragmentShader;

		~ShaderModules()
		{
			auto pInstance = vkSys::VkInstance::GetInstance();

			if (vertexShader != VK_NULL_HANDLE) vkDestroyShaderModule(pInstance->device, vertexShader, nullptr);
			if (fragmentShader != VK_NULL_HANDLE) vkDestroyShaderModule(pInstance->device, fragmentShader, nullptr);
		}
	};

	std::shared_ptr<ShaderModules> modules;

	SpvReflectShaderModule vertexSpvModule;
	SpvReflectShaderModule fragmentSpvModule;


	VkDevice device;
public:
	VkPipeline BuildPipeline(const VkRenderPass & renderpass, const int & subpass, const VkPipelineLayout & pipelineLayout,
							const VkPipelineVertexInputStateCreateInfo * pCustomVertexInputState = nullptr) const;
	VkPipeline BuildShadowPipeline(const VkRenderPass & renderpass, const int & subpass, const VkPipelineLayout & pipelineLayout,
							const VkPipelineVertexInputStateCreateInfo * pCustomVertexInputState = nullptr) const;
private:
	inline VkPipelineShaderStageCreateInfo makeShaderStageCreateInfo(VkShaderStageFlagBits shaderStage, const VkShaderModule & shaderModule) const;

};

#endif // !SHADERHEADER