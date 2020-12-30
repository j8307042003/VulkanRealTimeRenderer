
#include "Shader.h"
#include "Model.h"
#include "VulkanLib/vkHelper.h"
#include "VulkanLib/VulkanInstance.h"
#ifndef VK_MATERIAL_HEADER
#define VK_MATERIAL_HEADER
#include "vkSys.h"

#include <memory>
 

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

	struct MatBindingAssignmentSet
	{
		std::string name;
		vkSys::TexMgr::Texture image;
	};


	MatProperty matProperty;
	std::shared_ptr<Shader> pShader;	

	VkImage mainTexImg;
	VkImageView mainTexImageView;
	int mipmapsLevel;

	VkImage specTexImg;
	VkImageView specImageView;

	VkImage normalTexImg;
	VkImageView normalTexImageView;

	std::vector<MatBindingAssignmentSet> bindingSets = {};


	void ReadMainTexture(const VulkanLib::VulkanInstance & vulkanInstance, const VkCommandPool & commandPool, const char * fileName);
	void SetTexture(vkSys::TexMgr::Texture texture);
	void SetSpecularTexture(vkSys::TexMgr::Texture texture);
	void SetNormalTexture(vkSys::TexMgr::Texture texture);
};


VkMaterialProgram vkMatBuildProgram(const VkMaterial & mat, VkDevice device, VkRenderPass renderpass, int subpass, 
									const VkPipelineVertexInputStateCreateInfo * vertexInputInfo,
									int numDescriptorSetLayoutBinding,
									VkDescriptorSetLayoutBinding * descriptorSetLayoutBindings);
VkMaterialProgram vkMatBuildShadowProgram(const VkMaterial & mat, VkDevice device, VkRenderPass renderpass, int subpass, 
									const VkPipelineVertexInputStateCreateInfo * vertexInputInfo,
									int numDescriptorSetLayoutBinding,
									VkDescriptorSetLayoutBinding * descriptorSetLayoutBindings);
#endif // !VK_MATERIAL_HEADER
