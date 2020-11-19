
#include "Shader.h"
#include "Model.h"
#include "VkMaterial.h"
#include "VulkanLib/vkHelper.h"


class RenderData;

class RenderDataUtils 
{
public:
	static std::vector<RenderData> LoadObj(const char * filename, VkMaterial material, VkCommandPool commandPool);

};

class RenderData {
	Model model;
	VkMaterial mat;

	VkMaterialProgram program;
	BufferObject vertexBufferObj;
	BufferObject indicesBufferObj;
	BufferObject customUboBufferObj;
	VkDescriptorSet descriptorSet;
public:
	RenderData();
	void setModel(Model & model);
	void setMaterial(VkMaterial & mat);
	void buildRenderData(VkDevice & device, const VkPhysicalDeviceMemoryProperties & deviceMemProps, VkDescriptorPool descriptorPool, VkDescriptorBufferInfo globalUniformBufInfo);
	void render(VkCommandBuffer & commandBuffer);
	VkMaterialProgram buildProgram(VkDevice device, VkRenderPass renderpass, int subpass);
};