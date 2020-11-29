#include "Model.h"
#include "assimp/cimport.h"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "vkSys.h"


void Model::SetData(std::vector<ModelVertexData> data, std::vector<uint32_t> indices)
{
	this->data = data;
	this->indices = indices;
}


void Model::LoadModel(const char * filename)
{
	aiPropertyStore * props = aiCreatePropertyStore();
	//auto scene = aiImportFileExWithProperties(filename, aiProcess_Triangulate, NULL, props);
	auto scene = aiImportFileExWithProperties(filename, aiProcessPreset_TargetRealtime_Fast, NULL, props);

	if (scene)
	{
		{
			int vertexCount = 0;
			int indicesCount = 0;
			int faceCount = 0;
			for (int i = 0; i < scene->mNumMeshes; ++i)
			{
				auto pMesh = scene->mMeshes[i];
				vertexCount += pMesh->mNumVertices;
				faceCount += pMesh->mNumFaces;
				for (int j = 0; j < pMesh->mNumFaces; ++j)
				{
					indicesCount += pMesh->mFaces[j].mNumIndices;
				}
			}

			data.resize(vertexCount);
			indices.resize(indicesCount);
		}

		int vertexNum = 0;
		int indicesNum = 0;
		for (int i = 0; i < scene->mNumMeshes; ++i)
		{
			auto pMesh = scene->mMeshes[i];

			for (int j = 0; j < pMesh->mNumFaces; ++j)
			{
				auto & faces = pMesh->mFaces[j];
				for (int k = 0; k < faces.mNumIndices; ++k)
				{
					indices[indicesNum] = faces.mIndices[k] + vertexNum;
					indicesNum++;
				}
			}

			for (int j = 0; j < pMesh->mNumVertices; ++j)
			{
				auto vertice = pMesh->mVertices[j];
				auto normal = pMesh->mNormals[j];
				aiVector3D * coords = &pMesh->mTextureCoords[0][j];
				data[vertexNum] = { {vertice.x, vertice.y, vertice.z}, {coords->x, coords->y}, {0, 0, 0, 0}, {normal.x, normal.y, normal.z} }; // vertex, uv, color
				//data[vertexNum] = { {vertice.x, vertice.y, vertice.z}, {0, 0}, {0, 0, 0, 0} }; // vertex, uv, color
				vertexNum++;
			}
		}
	}

	SendToGPU();
}

void Model::SendToGPU()
{
	VulkanLib::VulkanInstance & vulkanInstance = *vkSys::VkInstance::GetInstance();

	vertexBufferObj = BuildBuffer(vulkanInstance.device, vulkanInstance.deviceMemProps, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT , sizeof(Model::ModelVertexData) * data.size());
	indicesBufferObj = BuildBuffer(vulkanInstance.device, vulkanInstance.deviceMemProps, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, sizeof(uint32_t) * indices.size());

	CopyDataToDeviceMemory(vulkanInstance.device, vertexBufferObj.memory, vertexBufferObj.size, data.data());
	CopyDataToDeviceMemory(vulkanInstance.device, indicesBufferObj.memory, indicesBufferObj.size, indices.data());
}
