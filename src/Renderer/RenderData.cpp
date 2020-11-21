#include "RenderData.h"
#include <array>
#include "vkSys.h"
#include "assimp/cimport.h"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include <string>

std::vector<RenderData> RenderDataUtils::LoadObj(const char * filename, VkMaterial material, VkCommandPool commandpool)
{

	aiPropertyStore * props = aiCreatePropertyStore();
	auto scene = aiImportFileExWithProperties(filename, aiProcessPreset_TargetRealtime_Fast, NULL, props);	

	if (!scene) return {};

	std::vector<RenderData> renderDatas;
	renderDatas.resize(scene->mNumMeshes);

	std::string path = std::string(filename);
	std::string directory = path.substr(0, path.find_last_of('/'));

	for (int i = 0; i < scene->mNumMeshes; ++i)
	{
		Model model = {};
		VkMaterial mat = material;
		int indicesCount = 0;
		auto pMesh = scene->mMeshes[i];
		for (int j = 0; j < pMesh->mNumFaces; ++j)
		{
			indicesCount += pMesh->mFaces[j].mNumIndices;
		}
		model.data.resize(pMesh->mNumVertices);
		model.indices.resize(indicesCount);

		for (int j = 0; j < pMesh->mNumVertices; ++j)
		{
			auto vertice = pMesh->mVertices[j];
			auto normal = pMesh->mNormals[j];
			aiVector3D * coords = &pMesh->mTextureCoords[0][j];
			model.data[j] = { {vertice.x, vertice.y, vertice.z}, {coords->x, coords->y}, {0, 0, 0, 0}, {normal.x, normal.y, normal.z} }; // vertex, uv, color
		}

		int indicesNum = 0;
		for (int j = 0; j < pMesh->mNumFaces; ++j)
		{
			auto & faces = pMesh->mFaces[j];
			for (int k = 0; k < faces.mNumIndices; ++k)
			{
				model.indices[indicesNum] = faces.mIndices[k];
				indicesNum++;
			}
		}

		if (pMesh->mMaterialIndex >= 0) {
			auto pMaterial = scene->mMaterials[pMesh->mMaterialIndex];



			//Diffuse
			aiString str;
			pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &str);
			// std::cout << "tex " << str.C_Str() << "\n";

			std::string texName = (directory + '/' + std::string(str.C_Str()));
			for(int i = 0; i < texName.size(); ++i)
			{
				if (texName[i] == '\\') texName[i] = '/'; 
			}

			auto & vulkanInstance = *vkSys::VkInstance::GetInstance();
			auto tex = vkSys::TexMgr::GetTexture(texName.c_str(), commandpool);
			mat.SetTexture(*tex);
		}

		renderDatas[i].setModel(model);
		renderDatas[i].setMaterial(mat);
	}



	return renderDatas;
}




RenderData::RenderData()
{
	model = {};
	mat = {};
}


void RenderData::setModel(Model & model)
{
	this->model = model;
}

void RenderData::setMaterial(VkMaterial & mat)
{
	this->mat = mat;
}

VkMaterialProgram RenderData::buildProgram(VkDevice device, VkRenderPass renderpass, int subpass)
{

	std::array<VkVertexInputBindingDescription, 1> vertexBindingDescriptions = {};
	vertexBindingDescriptions[0].binding = 0;
    vertexBindingDescriptions[0].stride = sizeof(Model::ModelVertexData);
    vertexBindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX ;

    std::array<VkVertexInputAttributeDescription, 4> vertexInputAttributeDescriptions = {};
	vertexInputAttributeDescriptions[0].location = 0;
	vertexInputAttributeDescriptions[0].binding = 0;
	vertexInputAttributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexInputAttributeDescriptions[0].offset = offsetof(Model::ModelVertexData, pos);

	vertexInputAttributeDescriptions[1].location = 1;
	vertexInputAttributeDescriptions[1].binding = 0;
	vertexInputAttributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
	vertexInputAttributeDescriptions[1].offset = offsetof(Model::ModelVertexData, uv);

	vertexInputAttributeDescriptions[2].location = 2;
	vertexInputAttributeDescriptions[2].binding = 0;
	vertexInputAttributeDescriptions[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	vertexInputAttributeDescriptions[2].offset = offsetof(Model::ModelVertexData, color);

	vertexInputAttributeDescriptions[3].location = 3;
	vertexInputAttributeDescriptions[3].binding = 0;
	vertexInputAttributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexInputAttributeDescriptions[3].offset = offsetof(Model::ModelVertexData, normal);

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &vertexBindingDescriptions[0]; // Optional
	vertexInputInfo.vertexAttributeDescriptionCount = vertexInputAttributeDescriptions.size();
	vertexInputInfo.pVertexAttributeDescriptions = &vertexInputAttributeDescriptions[0]; // Optional


	program = vkMatBuildProgram(mat, device, renderpass, subpass, &vertexInputInfo);

	return program;	
}



void RenderData::buildRenderData(VkDevice & device, const VkPhysicalDeviceMemoryProperties & deviceMemProps, VkDescriptorPool descriptorPool, VkDescriptorBufferInfo globalUniformBufInfo)
{
	vertexBufferObj = BuildBuffer(device, deviceMemProps, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT , sizeof(Model::ModelVertexData) * model.data.size());
	indicesBufferObj = BuildBuffer(device, deviceMemProps, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, sizeof(uint32_t) * model.indices.size());
	customUboBufferObj = BuildBuffer(device, deviceMemProps, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 4 * sizeof(float));

	CopyDataToDeviceMemory(device, vertexBufferObj.memory, vertexBufferObj.size, model.data.data());
	CopyDataToDeviceMemory(device, indicesBufferObj.memory, indicesBufferObj.size, model.indices.data());

    VkSampler sampler = vkSys::Sampler::GetSampler(vkSys::Sampler2D);
	VkDescriptorImageInfo mainTexDescriptorImgInfo = createDescriptorImageInfo(sampler, mat.mainTexImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);


	// allocate descriptor set
	AllocateDescriptorSets(device, descriptorPool, 1, &program.descriptorSetLayout, &descriptorSet);
	VkWriteDescriptorSet globalWriteSet = createWriteDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &globalUniformBufInfo, 0);
	VkWriteDescriptorSet writeSet = createWriteDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &customUboBufferObj.descriptorBufInfo, 1);
	VkWriteDescriptorSet mainTexWriteSet = createWriteDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &mainTexDescriptorImgInfo, 2);
	std::array<VkWriteDescriptorSet, 3> writes = 
	{
		globalWriteSet,
		writeSet,
		mainTexWriteSet
	};
	vkUpdateDescriptorSets(device, writes.size(), writes.data(), 0, nullptr);
}


void RenderData::render(VkCommandBuffer & commandBuffer)
{
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, program.pipeline);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, program.pipelinelayout, 0, 1, &descriptorSet, 0, nullptr);
	VkDeviceSize offsets[1] = { 0 };		
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBufferObj.buffer, offsets);
	vkCmdBindIndexBuffer(commandBuffer, indicesBufferObj.buffer, offsets[0], VK_INDEX_TYPE_UINT32);
	vkCmdDrawIndexed(commandBuffer, model.indices.size(), 1, 0, 0, 0);
}

