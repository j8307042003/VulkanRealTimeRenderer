#include "RenderData.h"
#include <array>
#include "vkSys.h"
#include "assimp/cimport.h"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include <string>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/handed_coordinate_space.hpp>
#include "mikktspace.h"

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

void CalculateTangents(const aiVector3D & p0,const aiVector3D & p1, const aiVector3D & p2, 
						const aiVector3D & uv0, const aiVector3D & uv1, const aiVector3D & uv2,
						const aiVector3D & normal, aiVector3D & tangent, aiVector3D & bitangent)
{

	aiVector3D edge1 = p1 - p0;
	aiVector3D edge2 = p2 - p0;
	aiVector3D deltaUV1 = uv1 - uv0;
	aiVector3D deltaUV2 = uv2 - uv0;

	float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

	tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
	tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
	tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
	tangent = tangent.Normalize();

	bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
	bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
	bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
	bitangent = bitangent.Normalize();
}

void CalculateTangentsGLM(const glm::vec3 & p0, const glm::vec3 & p1, const glm::vec3 & p2,
	const glm::vec2 & uv0, const glm::vec2 & uv1, const glm::vec2 & uv2,
	const glm::vec3 & normal, glm::vec3 & tangent, glm::vec3 & bitangent)
{

	glm::vec3 edge1 = p1 - p0;
	glm::vec3 edge2 = p2 - p0;
	glm::vec2 deltaUV1 = uv1 - uv0;
	glm::vec2 deltaUV2 = uv2 - uv0;

	float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

	tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
	tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
	tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
	tangent = glm::normalize(tangent);

	bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
	bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
	bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
	bitangent = glm::normalize(bitangent);
}


std::vector<RenderData> RenderDataUtils::LoadObjGLTF(const char * filename, VkMaterial material, VkCommandPool commandpool)
{

#define LOAD_ACCESSOR(type, nbcomp, acc, dst) \
    { \
        int n = 0; \
        type* buf = (type*)acc->buffer_view->buffer->data + acc->buffer_view->offset/sizeof(type) + acc->offset/sizeof(type); \
        for (unsigned int k = 0; k < acc->count; k++) {\
            for (int l = 0; l < nbcomp; l++) {\
                dst[nbcomp*k + l] = buf[n + l];\
            }\
            n += (int)(acc->stride/sizeof(type));\
        }\
    }

#define LOAD_ACCESSORSINGLE(type, nbcomp, off, acc, dst) \
    { \
        type* buf = (type*)acc->buffer_view->buffer->data + acc->buffer_view->offset/sizeof(type) + acc->offset/sizeof(type); \
        for (int l = 0; l < nbcomp; l++) {\
            dst[l] = buf[off * (int)(acc->stride/sizeof(type)) + l];\
        }\
    }




	cgltf_options options = {};
	cgltf_data* data = NULL;
	cgltf_result result = cgltf_parse_file(&options, filename, &data);

	if (result != cgltf_result_success) return {};

	result = cgltf_load_buffers(&options, data, filename);
	if (result != cgltf_result_success) return {};

	int renderDatasNum = 0;
	for (int i = 0; i < data->meshes_count; ++i)
	{
		renderDatasNum += data->meshes[i].primitives_count;
	}


	std::vector<RenderData> renderDatas = {};
	renderDatas.resize(renderDatasNum);
	for (int i = 0; i < data->meshes_count; ++i)
	{
		auto pMesh = &data->meshes[i];

		for (int j = 0; j < pMesh->primitives_count; ++j)
		{
			auto primitive = pMesh->primitives[j];

			std::shared_ptr<Model> pModel = std::shared_ptr<Model>(new Model());
			Model & model = *pModel.get();
			VkMaterial mat = material;

			int primitiveCount = 0;
			primitiveCount += pMesh->primitives_count;
			model.indices.resize(primitive.indices->count);
			LOAD_ACCESSOR(unsigned short, 1, primitive.indices, model.indices);

			for (int k = 0; k < model.indices.size() / 3; ++k)
			{
				int head = k * 3;
				std::swap(model.indices[head], model.indices[head + 2]);
			}


			int maxArr = 0;

			for (int k = 0; k < primitive.attributes_count; ++k)
			{
				auto attri = &primitive.attributes[k];

				if (attri->type == cgltf_attribute_type_position)
				{
					maxArr = std::max(maxArr, (int)attri->data->count);
				}
				else if (attri->type == cgltf_attribute_type_normal)
				{
					maxArr = std::max(maxArr, (int)attri->data->count);
				}
				else if (attri->type == cgltf_attribute_type_tangent)
				{
					maxArr = std::max(maxArr, (int)attri->data->count);
				}
				else if (attri->type == cgltf_attribute_type_texcoord)
				{
					maxArr = std::max(maxArr, (int)attri->data->count);
				}
			}

			model.data.resize(maxArr);


			for (int z = 0; z < maxArr; z++)
			{

				glm::vec3 pos = {};
				glm::vec3 normal = {};
				glm::vec3 tangent = {};
				glm::vec3 bitangent = {};
				glm::vec2 uv = {};
				float tanW = 0;
				unsigned short indices = 0;

				for (int k = 0; k < primitive.attributes_count; ++k)
				{
					auto attri = &primitive.attributes[k];
					if (attri->type == cgltf_attribute_type_position)
					{
						LOAD_ACCESSORSINGLE(float, 3, z, attri->data, pos);
					}
					else if (attri->type == cgltf_attribute_type_normal)
					{
						LOAD_ACCESSORSINGLE(float, 3, z, attri->data, normal);
					}
					else if (attri->type == cgltf_attribute_type_tangent)
					{
						glm::vec4 attrTangent;
						LOAD_ACCESSORSINGLE(float, 4, z, attri->data, attrTangent);

						tangent = glm::make_vec3(attrTangent);
						tanW = attrTangent.w;
					}
					else if (attri->type == cgltf_attribute_type_texcoord)
					{
						LOAD_ACCESSORSINGLE(float, 2, z, attri->data, uv);
					}
				}

				model.data[z] =
				{
					pos,
					uv,
					{ 0, 0, 0, 0 },
					normal,
					tangent,
					glm::cross(normal, tangent) * tanW,
				};
			}

			for (int k = 0; false && k < model.indices.size() / 3; ++k)
			{
				int head = k * 3;
				CalculateTangentsGLM(model.data[model.indices[head]].pos, model.data[model.indices[head + 1]].pos, model.data[model.indices[head + 2]].pos,
					model.data[model.indices[head]].uv, model.data[model.indices[head + 1]].uv, model.data[model.indices[head + 2]].uv,
					model.data[model.indices[head]].normal, model.data[model.indices[head]].tangent, model.data[model.indices[head]].bitangent);

				model.data[model.indices[head+1]].tangent = model.data[model.indices[head]].tangent;
				model.data[model.indices[head+1]].bitangent = model.data[model.indices[head]].bitangent;
				model.data[model.indices[head+2]].tangent = model.data[model.indices[head]].tangent;
				model.data[model.indices[head+2]].bitangent = model.data[model.indices[head]].bitangent;
			}


			auto tex = vkSys::TexMgr::GetTexture("working/texture/wall.jpg", commandpool);
			mat.SetTexture(*tex);
			auto norTex = vkSys::TexMgr::GetTexture("working/texture/wall.jpg", commandpool);
			mat.SetNormalTexture(*norTex);
			auto specTex = vkSys::TexMgr::GetTexture("working/texture/wall.jpg", commandpool);
			mat.SetSpecularTexture(*specTex);


			renderDatas[j].setModel(pModel);
			renderDatas[j].setMaterial(mat);
		}

	}
	

	cgltf_free(data);

	return renderDatas;
}


void mikktGetNormal(const SMikkTSpaceContext *pContext, float fvNormOut[], const int iFace, const int iVert) 
{
	const aiMesh * pMesh = reinterpret_cast<const aiMesh *>(pContext->m_pUserData);
	int vertex = pMesh->mFaces[iFace].mIndices[iVert];
	fvNormOut[0] = pMesh->mNormals[vertex].x;
	fvNormOut[1] = pMesh->mNormals[vertex].y;
	fvNormOut[2] = pMesh->mNormals[vertex].z;
}

int mikktGetNumFaces(const SMikkTSpaceContext * pContext)
{
	const aiMesh * pMesh = reinterpret_cast<const aiMesh *>(pContext->m_pUserData);
	return pMesh->mNumFaces;
}

int mikktGetNumVerticesOfFace(const SMikkTSpaceContext * pContext, const int iFace)
{
	const aiMesh * pMesh = reinterpret_cast<const aiMesh *>(pContext->m_pUserData);
	return pMesh->mFaces[iFace].mNumIndices;
}

void mikktGetPosition(const SMikkTSpaceContext * pContext, float fvPosOut[], const int iFace, const int iVert)
{
	const aiMesh * pMesh = reinterpret_cast<const aiMesh *>(pContext->m_pUserData);
	int vertex = pMesh->mFaces[iFace].mIndices[iVert];
	fvPosOut[0] = pMesh->mVertices[vertex].x;
	fvPosOut[1] = pMesh->mVertices[vertex].y;
	fvPosOut[2] = pMesh->mVertices[vertex].z;
}

void mikktGetTexCoord(const SMikkTSpaceContext * pContext, float fvTexcOut[], const int iFace, const int iVert)
{
	const aiMesh * pMesh = reinterpret_cast<const aiMesh *>(pContext->m_pUserData);
	int vertex = pMesh->mFaces[iFace].mIndices[iVert];
	fvTexcOut[0] = pMesh->mTextureCoords[0][vertex].x;
	fvTexcOut[1] = pMesh->mTextureCoords[0][vertex].y;
}

void mikkSetSpace(const SMikkTSpaceContext * pContext, const float fvTangent[], const float fvBiTangent[], const float fMagS, const float fMagT,
	const tbool bIsOrientationPreserving, const int iFace, const int iVert)
{
	const aiMesh * pMesh = reinterpret_cast<const aiMesh *>(pContext->m_pUserData);
	int vertex = pMesh->mFaces[iFace].mIndices[iVert];
	pMesh->mTangents[vertex] = aiVector3D(- fvTangent[0], fvTangent[1], fvTangent[2] );
	auto n = pMesh->mNormals[vertex];
	n.x *= 1.0f;
	pMesh->mBitangents[vertex] = (n ^ pMesh->mTangents[vertex]).SymMul(aiVector3D(bIsOrientationPreserving ? 1.0f : -1.0));
	//pMesh->mBitangents[vertex] = (n ^ pMesh->mTangents[vertex]);
	//pMesh->mBitangents[vertex].x = bIsOrientationPreserving ? 1.0f : -1.0;
	//pMesh->mBitangents[vertex] = aiVector3D(fvBiTangent[0], fvBiTangent[1], fvBiTangent[2]);
}

void mikkSetSpaceBasic(const SMikkTSpaceContext * pContext, const float fvTangent[], const float fSign, const int iFace, const int iVert)
{

}

std::vector<RenderData> RenderDataUtils::LoadObj(const char * filename, VkMaterial material, VkCommandPool commandpool)
{

	aiPropertyStore * props = aiCreatePropertyStore();
	//auto scene = aiImportFileExWithProperties(filename, aiProcessPreset_TargetRealtime_Quality /*aiProcessPreset_TargetRealtime_Fast*/, NULL, props);
	auto scene = aiImportFileEx(filename, aiProcess_FlipWindingOrder | aiProcessPreset_TargetRealtime_Quality, NULL);
	if (!scene) return {};

	std::vector<RenderData> renderDatas;
	renderDatas.resize(scene->mNumMeshes);

	std::string path = std::string(filename);
	std::string directory = path.substr(0, path.find_last_of('/'));

	for (int i = 0; i < scene->mNumMeshes; ++i)
	{
		std::shared_ptr<Model> pModel = std::shared_ptr<Model>(new Model());
		Model & model = *pModel.get();
		VkMaterial mat = material;
		int indicesCount = 0;
		auto pMesh = scene->mMeshes[i];

		if (pMesh->mTextureCoords[0]) {
			SMikkTSpaceContext mikkTSpaceContext = {};
			SMikkTSpaceInterface mif = {};
			mif.m_getNormal = mikktGetNormal;
			mif.m_getNumFaces = mikktGetNumFaces;
			mif.m_getNumVerticesOfFace = mikktGetNumVerticesOfFace;
			mif.m_getPosition = mikktGetPosition;
			mif.m_getTexCoord = mikktGetTexCoord;
			mif.m_setTSpace = mikkSetSpace;
			mif.m_setTSpaceBasic = mikkSetSpaceBasic;

			mikkTSpaceContext.m_pInterface = &mif;
			mikkTSpaceContext.m_pUserData = pMesh;
			genTangSpaceDefault(&mikkTSpaceContext);
		}

		for (int j = 0; j < pMesh->mNumFaces; ++j)
		{
			indicesCount += pMesh->mFaces[j].mNumIndices;
		}
		model.data.resize(pMesh->mNumVertices);
		model.indices.resize(indicesCount);

		aiVector3D basicCoord = aiVector3D();
		bool hasTangent = pMesh->HasTangentsAndBitangents();

		if (pMesh->mTextureCoords[0]&&false) {
			for (int j = 0; j < pMesh->mNumFaces; ++j)
			{
				auto & faces = pMesh->mFaces[j];
				if (faces.mNumIndices == 3)
				{
					int v0 = faces.mIndices[0];
					int v1 = faces.mIndices[1];
					int v2 = faces.mIndices[2];

					CalculateTangents(pMesh->mVertices[v0], pMesh->mVertices[v1], pMesh->mVertices[v2],
						pMesh->mTextureCoords[0][v0], pMesh->mTextureCoords[0][v1], pMesh->mTextureCoords[0][v2],
						pMesh->mNormals[v0], pMesh->mTangents[v0], pMesh->mBitangents[v0]);

					pMesh->mTangents[v1] = pMesh->mTangents[v0];
					pMesh->mBitangents[v1] = pMesh->mBitangents[v0];

					pMesh->mTangents[v2] = pMesh->mTangents[v0];
					pMesh->mBitangents[v2] = pMesh->mBitangents[v0];
				}
			}
		}


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



			if (glm::dot(glm::cross(model.data[j].normal, model.data[j].tangent), model.data[j].bitangent) < 0.0)
			{
				//model.data[j].tangent = model.data[j].tangent * -1.0f;
			}
			//model.data[j].bitangent = glm::cross(model.data[j].normal, model.data[j].tangent) * -1.0f;

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

		renderDatas[i].setModel(pModel);
		renderDatas[i].setMaterial(mat);
	}



	return renderDatas;
}




RenderData::RenderData()
{
	modelMatrix = glm::mat4(1);
	//model = {};
	mat = {};
	shadowPassMat = {};
}


void RenderData::setModel(std::shared_ptr<Model> & model)
{
	this->pModel = model;
}

void RenderData::setMaterial(VkMaterial & mat)
{
	this->mat = mat;
}

void RenderData::buildProgram(VkDevice device, VkRenderPass renderpass, int subpass)
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

	Shader * pShader = mat.pShader.get();
	std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings(pShader->fragmentVariables.size());
	for (int i = 0; i < pShader->fragmentVariables.size(); ++i)
	{
		VkDescriptorType descriptorType = {};
		switch (pShader->fragmentVariables[i]->descriptor_type)
		{

			case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER :	descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLER; break;
			case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER :	descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; break;
			case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE :	descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE; break;
			case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE :	descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE; break;
			case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER :	descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER; break;
			case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER :	descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER; break;
			case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER :	descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; break;
			case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER :	descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER; break;
			case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC :	descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC; break;
			case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC :	descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC; break;
			case SPV_REFLECT_DESCRIPTOR_TYPE_INPUT_ATTACHMENT :	descriptorType  = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT; break;

		default:
			break;
		}
		setLayoutBindings[i] = createDescriptorSetLayoutBinding(descriptorType, pShader->fragmentVariables[i]->binding, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
	}

	program = vkMatBuildProgram(mat, device, renderpass, subpass, &vertexInputInfo, setLayoutBindings.size(), setLayoutBindings.data());
	

	// shadow pass
	{
		shadowPassMat.pShader = std::shared_ptr<Shader>( new Shader{ device, "working/subpassForward/shadow/shadow.vert.spv", "working/subpassForward/shadow/shadow.frag.spv" });
		std::array<VkDescriptorSetLayoutBinding, 1> setLayoutBindings = 
		{
			createDescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT), // global data
		};

		shadowPassProgram = vkMatBuildShadowProgram(shadowPassMat, device, renderpass, subpass, &vertexInputInfo, setLayoutBindings.size(), setLayoutBindings.data());
	}
}



void RenderData::buildRenderData(VkDevice & device, const VkPhysicalDeviceMemoryProperties & deviceMemProps, VkDescriptorPool descriptorPool, VkDescriptorBufferInfo globalUniformBufInfo, VkDescriptorBufferInfo shadowpassUniform, 
								VkImageView GIImageView, VkImageView BRDFLUTImageView, VkImageView shadowmapImgView)
{
	vertexBufferObj = BuildBuffer(device, deviceMemProps, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT , sizeof(Model::ModelVertexData) * pModel.get()->data.size());
	indicesBufferObj = BuildBuffer(device, deviceMemProps, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, sizeof(uint32_t) * pModel.get()->indices.size());
	customUboBufferObj = BuildBuffer(device, deviceMemProps, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(VkMaterial::MatProperty));

	CopyDataToDeviceMemory(device, vertexBufferObj.memory, vertexBufferObj.size, pModel.get()->data.data());
	CopyDataToDeviceMemory(device, indicesBufferObj.memory, indicesBufferObj.size, pModel.get()->indices.data());
	CopyDataToDeviceMemory(device, customUboBufferObj.memory, customUboBufferObj.size, &mat.matProperty);

    VkSampler sampler = vkSys::Sampler::GetSampler(vkSys::Sampler2D, mat.mipmapsLevel);
	VkDescriptorImageInfo mainTexDescriptorImgInfo = createDescriptorImageInfo(sampler, mat.mainTexImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	VkDescriptorImageInfo specTexDescriptorImgInfo = createDescriptorImageInfo(sampler, mat.specImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	VkDescriptorImageInfo normalTexDescriptorImgInfo = createDescriptorImageInfo(sampler, mat.normalTexImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	VkDescriptorImageInfo GIImgBufInfo = createDescriptorImageInfo(sampler, GIImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	VkDescriptorImageInfo BRDFLUTImgBufInfo = createDescriptorImageInfo(sampler, BRDFLUTImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	VkDescriptorImageInfo ShadowmapImgBufInfo = createDescriptorImageInfo(sampler, shadowmapImgView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);


	// allocate descriptor set
	AllocateDescriptorSets(device, descriptorPool, 1, &program.descriptorSetLayout, &descriptorSet);
	VkWriteDescriptorSet globalWriteSet = createWriteDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &globalUniformBufInfo, 0);
	VkWriteDescriptorSet writeSet = createWriteDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &customUboBufferObj.descriptorBufInfo, 1);
	VkWriteDescriptorSet mainTexWriteSet = createWriteDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &mainTexDescriptorImgInfo, 2);
	VkWriteDescriptorSet specTexWriteSet = createWriteDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &specTexDescriptorImgInfo, 3);
	VkWriteDescriptorSet normalTexWriteSet = createWriteDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &normalTexDescriptorImgInfo, 4);
	VkWriteDescriptorSet giTexWriteSet = createWriteDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &GIImgBufInfo, 5);
	VkWriteDescriptorSet BRDFLUTTexWriteSet = createWriteDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &BRDFLUTImgBufInfo, 6);
	VkWriteDescriptorSet shadowmapWriteSet = createWriteDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &ShadowmapImgBufInfo, 7);
	std::array<VkWriteDescriptorSet, 8> writes = 
	{
		globalWriteSet,
		writeSet,
		mainTexWriteSet,
		specTexWriteSet,
		normalTexWriteSet,
		giTexWriteSet,
		BRDFLUTTexWriteSet,
		shadowmapWriteSet
	};
	vkUpdateDescriptorSets(device, writes.size(), writes.data(), 0, nullptr);

	if (mat.bindingSets.size() > 0)
	{
		std::vector<VkWriteDescriptorSet> sets = {};
		std::vector<VkDescriptorImageInfo> imageInfos = {};
		sets.reserve(mat.bindingSets.size());
		imageInfos.reserve(mat.bindingSets.size());

		for (int i = 0; i < mat.bindingSets.size();++i)
		{
			std::string & name = mat.bindingSets[i].name;
			Shader & shader = *mat.pShader.get();
			for (int j = 0; j < shader.fragmentVariables.size(); ++j)
			{
				if (strcmp(shader.fragmentVariables[j]->name, name.c_str()) == 0)
				{
					imageInfos.push_back(createDescriptorImageInfo(sampler, mat.bindingSets[i].image.imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));
					VkWriteDescriptorSet desSet = createWriteDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &imageInfos[imageInfos.size()-1], shader.fragmentVariables[j]->binding);
					sets.push_back(desSet);

					break;
				}
			}
		}

		vkUpdateDescriptorSets(device, sets.size(), sets.data(), 0, nullptr);
	}



	//shadow pass 
	{
		// allocate descriptor set
		AllocateDescriptorSets(device, descriptorPool, 1, &shadowPassProgram.descriptorSetLayout, &shadowpassDescriptorSet);
		VkWriteDescriptorSet globalWriteSet = createWriteDescriptorSet(shadowpassDescriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &shadowpassUniform, 0);
		std::array<VkWriteDescriptorSet, 1> writes = 
		{
			globalWriteSet,
		};
		vkUpdateDescriptorSets(device, writes.size(), writes.data(), 0, nullptr);		
	}

}


void RenderData::render(VkCommandBuffer & commandBuffer)
{
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, program.pipeline);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, program.pipelinelayout, 0, 1, &descriptorSet, 0, nullptr);
	VkDeviceSize offsets[1] = { 0 };		
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBufferObj.buffer, offsets);
	vkCmdBindIndexBuffer(commandBuffer, indicesBufferObj.buffer, offsets[0], VK_INDEX_TYPE_UINT32);
	vkCmdPushConstants(commandBuffer, program.pipelinelayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &modelMatrix);
	vkCmdDrawIndexed(commandBuffer, pModel.get()->indices.size(), 1, 0, 0, 0);
}

void RenderData::renderShadowPass(VkCommandBuffer & commandBuffer)
{
	if (!bCastShadow) return;
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shadowPassProgram.pipeline);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shadowPassProgram.pipelinelayout, 0, 1, &shadowpassDescriptorSet, 0, nullptr);
	VkDeviceSize offsets[1] = { 0 };		
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBufferObj.buffer, offsets);
	vkCmdBindIndexBuffer(commandBuffer, indicesBufferObj.buffer, offsets[0], VK_INDEX_TYPE_UINT32);
	vkCmdPushConstants(commandBuffer, shadowPassProgram.pipelinelayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &modelMatrix);
	vkCmdDrawIndexed(commandBuffer, pModel.get()->indices.size(), 1, 0, 0, 0);
}
