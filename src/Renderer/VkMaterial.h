
#include "Shader.h"
#include "Model.h"
#include "VulkanLib/vkHelper.h"
#include "VulkanLib/VulkanInstance.h"
#ifndef VK_MATERIAL_HEADER
#define VK_MATERIAL_HEADER
#include "vkSys.h"
 

class VkMaterial;

struct VkMaterialProgram {
	VkPipeline pipeline;
	VkRenderPass renderpass;
	int subpass; 
	VkPipelineLayout pipelinelayout;
	VkDescriptorSetLayout descriptorSetLayout;

	const VkMaterial * pMat;
};


class VkMaterial 
{
public:
	struct MatProperty
	{
		float ambient;
		float diffuse;
		float specular;
		float shininess;

		glm::vec3 specularColor;
	};

	MatProperty matProperty;
	Shader shader;	

	VkImage mainTexImg;
	VkImageView mainTexImageView;
	int mipmapsLevel;

	VkImage specTexImg;
	VkImageView specImageView;

	VkImage normalTexImg;
	VkImageView normalTexImageView;

	void ReadMainTexture(const VulkanLib::VulkanInstance & vulkanInstance, const VkCommandPool & commandPool, const char * fileName);
	void SetTexture(vkSys::TexMgr::Texture texture);
	void SetSpecularTexture(vkSys::TexMgr::Texture texture);
	void SetNormalTexture(vkSys::TexMgr::Texture texture);
};


VkMaterialProgram vkMatBuildProgram(const VkMaterial & mat, VkDevice device, VkRenderPass renderpass, int subpass, const VkPipelineVertexInputStateCreateInfo * vertexInputInfo);
#endif // !VK_MATERIAL_HEADER
