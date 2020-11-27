#include "RenderData.h"
#include <array>
#include "vkSys.h"
#include "assimp/cimport.h"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include <string>
#include <glm/gtx/string_cast.hpp>

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

		aiVector3D basicCoord = aiVector3D();
		bool hasTangent = pMesh->HasTangentsAndBitangents();

		for (int j = 0; j < pMesh->mNumVertices; ++j)
		{
			auto vertice = pMesh->mVertices[j];
			auto normal = pMesh->mNormals[j];
			aiVector3D tangent = pMesh->mTangents == nullptr ? aiVector3D(0, 0, 0) : pMesh->mTangents[j];
			aiVector3D bitangent = pMesh->mBitangents == nullptr ? aiVector3D(0, 0, 0) : pMesh->mBitangents[j];
			aiVector3D * coords = pMesh->mTextureCoords[0] == nullptr ? &basicCoord : &pMesh->mTextureCoords[0][j];
			model.data[j] = { 
								{vertice.x, vertice.y, vertice.z}, 
								{coords->x, coords->y}, 
								{0, 0, 0, 0}, 
								{normal.x, normal.y, normal.z},
								{tangent.x, tangent.y, tangent.z},
								{bitangent.x, bitangent.y, bitangent.z},
							}; // vertex, uv, color
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
			aiString * pDiffStr = nullptr;
			
			for (int j = 0; j < pMaterial->mNumProperties; ++j)
			{
				auto property = pMaterial->mProperties[j];
				float data = 0;
				aiString * pStr;
				float * fdata;

				//std::cout << property->mKey.C_Str() << "  " << property->mType << "  ";
				switch (property->mType)
				{
					case aiPTI_Float:
						fdata = reinterpret_cast<float*>(property->mData);
						//std::cout << "ai float property  " << *fdata << std::endl;
						break;
					case aiPTI_String: 
						pStr = reinterpret_cast<aiString*>(property->mData);
						//std::cout << "ai string property  " << pStr->C_Str() << std::endl;
						break;
				}

				if (strcmp(property->mKey.C_Str(), "$mat.shininess") == 0)
				{
					float shininess = *reinterpret_cast <float *>(property->mData);
					mat.matProperty.shininess = shininess;
				}
				else if (strcmp(property->mKey.C_Str(), "$clr.specular") == 0)
				{
					float * specularColor = reinterpret_cast <float *>(property->mData);
					mat.matProperty.specularColor = glm::vec3(specularColor[0], specularColor[1], specularColor[2]);
					//std::cout << "specularColor " << glm::to_string(mat.matProperty.specularColor) << std::endl;

				}
				else if (strcmp(property->mKey.C_Str(), "$mat.gltf.pbrMetallicRoughness.metallicFactor") == 0)
				{
					//std::cout << "metallicFactor " << *reinterpret_cast <float *>(property->mData) << std::endl;

				}
				else if (strcmp(property->mKey.C_Str(), "$mat.gltf.pbrMetallicRoughness.roughnessFactor") == 0)
				{
					//std::cout << "roughnessFactor " << *reinterpret_cast <float *>(property->mData) << std::endl;

				}
				else if (strcmp(property->mKey.C_Str(), "$raw.DiffuseColor | file") == 0)
				{
					pDiffStr = reinterpret_cast <aiString *>(property->mData);
				}

				//std::cout << std::endl;
			}
			
			auto & vulkanInstance = *vkSys::VkInstance::GetInstance();

			pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &str);
			std::string texName = (directory + '/' + std::string(pDiffStr != nullptr ? pDiffStr->C_Str() : str.C_Str()));
			for(int j = 0; j < texName.size(); ++j)
			{
				if (texName[j] == '\\') texName[j] = '/';
			}

			// std::cout << "tex " << str.C_Str() << "\n";
			auto tex = vkSys::TexMgr::GetTexture(texName.c_str(), commandpool);
			mat.SetTexture(*tex);


			aiString specStr;
			//std::cout << "spec Count " << pMaterial->GetTextureCount(aiTextureType_SPECULAR);
			pMaterial->GetTexture(aiTextureType_SPECULAR, 0, &specStr);
			std::string specTexName = (directory + '/' + std::string(specStr.C_Str()));
			for (int j = 0; j < specTexName.size(); ++j)
			{
				if (specTexName[j] == '\\') specTexName[j] = '/';
			}
			//std::cout << "spec tex " << specStr.C_Str() << std::endl;
			auto specTex = vkSys::TexMgr::GetTexture(specTexName.c_str(), commandpool);
			mat.SetSpecularTexture(*specTex);

			//normal
			aiString normalStr;
			//std::cout << "spec Count " << pMaterial->GetTextureCount(aiTextureType_SPECULAR);
			pMaterial->GetTexture(aiTextureType_NORMALS, 0, &normalStr);
			std::string normalTexName = (directory + '/' + std::string(normalStr.C_Str()));
			for (int j = 0; j < normalTexName.size(); ++j)
			{
				if (normalTexName[j] == '\\') normalTexName[j] = '/';
			}
			//std::cout << "normal tex " << normalTexName.C_Str() << std::endl;
			auto normalTex = vkSys::TexMgr::GetTexture(normalTexName.c_str(), commandpool);
			mat.SetNormalTexture(*normalTex);


			//std::cout << "roughness tex count " << pMaterial->GetTextureCount(aiTextureType_DIFFUSE_ROUGHNESS) << std::endl;
			//std::cout << "METALNESS tex count " << pMaterial->GetTextureCount(aiTextureType_METALNESS) << std::endl;
			//std::cout << "NORMALS tex count " << pMaterial->GetTextureCount(aiTextureType_NORMALS) << std::endl;
			//std::cout << "BASE_COLOR tex count " << pMaterial->GetTextureCount(aiTextureType_BASE_COLOR) << std::endl;
			//std::cout << "DIFFUSE tex count " << pMaterial->GetTextureCount(aiTextureType_DIFFUSE) << std::endl;
			//std::cout << "aiTextureType_UNKNOWN tex count " << pMaterial->GetTextureCount(aiTextureType_UNKNOWN) << std::endl;
			if (pMaterial->GetTextureCount(aiTextureType_UNKNOWN) > 0)
			{
				aiString utex = {};
				pMaterial->GetTexture(aiTextureType_UNKNOWN, 0, &utex);
				std::string specTexName = (directory + '/' + std::string(utex.C_Str()));
				for (int j = 0; j < specTexName.size(); ++j)
				{
					if (specTexName[j] == '\\') specTexName[j] = '/';
				}
				auto specTex = vkSys::TexMgr::GetTexture(specTexName.c_str(), commandpool);
				mat.SetSpecularTexture(*specTex);

			}

		}

		renderDatas[i].setModel(model);
		renderDatas[i].setMaterial(mat);
	}



	return renderDatas;
}




RenderData::RenderData()
{
	modelMatrix = glm::mat4(1);
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

    std::array<VkVertexInputAttributeDescription, 6> vertexInputAttributeDescriptions = {};
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

	vertexInputAttributeDescriptions[4].location = 4;
	vertexInputAttributeDescriptions[4].binding = 0;
	vertexInputAttributeDescriptions[4].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexInputAttributeDescriptions[4].offset = offsetof(Model::ModelVertexData, tangent);

	vertexInputAttributeDescriptions[5].location = 5;
	vertexInputAttributeDescriptions[5].binding = 0;
	vertexInputAttributeDescriptions[5].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexInputAttributeDescriptions[5].offset = offsetof(Model::ModelVertexData, bitangent);

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &vertexBindingDescriptions[0]; // Optional
	vertexInputInfo.vertexAttributeDescriptionCount = vertexInputAttributeDescriptions.size();
	vertexInputInfo.pVertexAttributeDescriptions = &vertexInputAttributeDescriptions[0]; // Optional


	program = vkMatBuildProgram(mat, device, renderpass, subpass, &vertexInputInfo);

	return program;	
}



void RenderData::buildRenderData(VkDevice & device, const VkPhysicalDeviceMemoryProperties & deviceMemProps, VkDescriptorPool descriptorPool, VkDescriptorBufferInfo globalUniformBufInfo, VkImageView GIImageView, VkImageView BRDFLUTImageView)
{
	vertexBufferObj = BuildBuffer(device, deviceMemProps, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT , sizeof(Model::ModelVertexData) * model.data.size());
	indicesBufferObj = BuildBuffer(device, deviceMemProps, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, sizeof(uint32_t) * model.indices.size());
	customUboBufferObj = BuildBuffer(device, deviceMemProps, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(VkMaterial::MatProperty));

	CopyDataToDeviceMemory(device, vertexBufferObj.memory, vertexBufferObj.size, model.data.data());
	CopyDataToDeviceMemory(device, indicesBufferObj.memory, indicesBufferObj.size, model.indices.data());
	CopyDataToDeviceMemory(device, customUboBufferObj.memory, customUboBufferObj.size, &mat.matProperty);

    VkSampler sampler = vkSys::Sampler::GetSampler(vkSys::Sampler2D, mat.mipmapsLevel);
	VkDescriptorImageInfo mainTexDescriptorImgInfo = createDescriptorImageInfo(sampler, mat.mainTexImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	VkDescriptorImageInfo specTexDescriptorImgInfo = createDescriptorImageInfo(sampler, mat.specImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	VkDescriptorImageInfo normalTexDescriptorImgInfo = createDescriptorImageInfo(sampler, mat.normalTexImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	VkDescriptorImageInfo GIImgBufInfo = createDescriptorImageInfo(sampler, GIImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	VkDescriptorImageInfo BRDFLUTImgBufInfo = createDescriptorImageInfo(sampler, BRDFLUTImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);


	// allocate descriptor set
	AllocateDescriptorSets(device, descriptorPool, 1, &program.descriptorSetLayout, &descriptorSet);
	VkWriteDescriptorSet globalWriteSet = createWriteDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &globalUniformBufInfo, 0);
	VkWriteDescriptorSet writeSet = createWriteDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &customUboBufferObj.descriptorBufInfo, 1);
	VkWriteDescriptorSet mainTexWriteSet = createWriteDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &mainTexDescriptorImgInfo, 2);
	VkWriteDescriptorSet specTexWriteSet = createWriteDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &specTexDescriptorImgInfo, 3);
	VkWriteDescriptorSet normalTexWriteSet = createWriteDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &normalTexDescriptorImgInfo, 4);
	VkWriteDescriptorSet giTexWriteSet = createWriteDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &GIImgBufInfo, 5);
	VkWriteDescriptorSet BRDFLUTTexWriteSet = createWriteDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &BRDFLUTImgBufInfo, 6);
	std::array<VkWriteDescriptorSet, 7> writes = 
	{
		globalWriteSet,
		writeSet,
		mainTexWriteSet,
		specTexWriteSet,
		normalTexWriteSet,
		giTexWriteSet,
		BRDFLUTTexWriteSet,
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
	vkCmdPushConstants(commandBuffer, program.pipelinelayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &modelMatrix);
	vkCmdDrawIndexed(commandBuffer, model.indices.size(), 1, 0, 0, 0);
}