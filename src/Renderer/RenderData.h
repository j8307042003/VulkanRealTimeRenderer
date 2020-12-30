
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
	static std::vector<RenderData> LoadObjGLTF(const char * filename, VkMaterial material, VkCommandPool commandpool);

};

class RenderData {
	bool bCastShadow = true;
	std::shared_ptr<Model> pModel;
	VkMaterial mat;
	VkMaterial shadowPassMat;

	VkMaterialProgram program;
	VkMaterialProgram shadowPassProgram;
	BufferObject vertexBufferObj;
	BufferObject indicesBufferObj;
	BufferObject customUboBufferObj;
	VkDescriptorSet descriptorSet;
	VkDescriptorSet shadowpassDescriptorSet;
public:
	glm::mat4 modelMatrix;

	RenderData();
	void setShadow(bool bEnableShadow) { bCastShadow = bEnableShadow; };
	void setModel(std::shared_ptr<Model> & model);
	void setMaterial(VkMaterial & mat);
	void buildRenderData(VkDevice & device, const VkPhysicalDeviceMemoryProperties & deviceMemProps, VkDescriptorPool descriptorPool, 
						 VkDescriptorBufferInfo globalUniformBufInfo, VkDescriptorBufferInfo shadowpassUniform,
						 VkImageView GIImageView, VkImageView BRDFLUTImageView, VkImageView shadowmapImgView);
	void render(VkCommandBuffer & commandBuffer);
	void renderShadowPass(VkCommandBuffer & commandBuffer);
	void buildProgram(VkDevice device, VkRenderPass renderpass, int subpass);
};