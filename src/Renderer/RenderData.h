
#include "Shader.h"
#include "Model.h"
#include "VkMaterial.h"
#include "VulkanLib/vkHelper.h"
#include <memory>

class RenderData;

class RenderDataUtils 
{
public:
	static std::vector<RenderData> LoadObj(const char * filename, VkMaterial material, VkCommandPool commandPool);

};

class RenderData {
	VkMaterial mat;

	VkMaterialProgram program;
	BufferObject vertexBufferObj;
	BufferObject indicesBufferObj;
	BufferObject customUboBufferObj;
	VkDescriptorSet descriptorSet;
public:
	Model model;
	Model * sharedModel;
	glm::mat4 modelMatrix;

	RenderData();
	void setModel(Model & model);
	void setSharedModel(Model * model);
	void setMaterial(VkMaterial & mat);
	void buildRenderData(VkDevice & device, const VkPhysicalDeviceMemoryProperties & deviceMemProps, VkDescriptorPool descriptorPool, VkDescriptorBufferInfo globalUniformBufInfo, VkImageView GIImageView, VkImageView BRDFLUTImageView);
	void render(VkCommandBuffer & commandBuffer);
	VkMaterialProgram buildProgram(VkDevice device, VkRenderPass renderpass, int subpass);
};