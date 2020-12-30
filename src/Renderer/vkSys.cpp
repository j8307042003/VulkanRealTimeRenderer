#include "vkSys.h"
#include "VulkanLib/vkHelper.h"

VulkanLib::VulkanInstance vkSys::VkInstance::instance;
bool vkSys::VkInstance::bInited;
VkSampler vkSys::Sampler::samplers[SamplerMax];
std::map<std::string, vkSys::TexMgr::Texture> vkSys::TexMgr::map;


VkSampler vkSys::Sampler::GetSampler(uint32_t samplerKind, int mipmaps)
{
	return Create2DSampler(mipmaps);

	if (samplers[Sampler2D] == VK_NULL_HANDLE) 
	{
		samplers[Sampler2D] = Create2DSampler(mipmaps);
	}


	return samplers[samplerKind];
}


VkSampler vkSys::Sampler::Create2DSampler(int mipmaps)
{
	auto & vulkanInstance = *VkInstance::GetInstance();

	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 16.0f;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = static_cast<float>(mipmaps);

	VkSampler sampler = {};
	if (vkCreateSampler(vulkanInstance.device, &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
		return sampler;
    }	

    return sampler;
}


vkSys::TexMgr::Texture * vkSys::TexMgr::GetTexture(const char * filename, const VkCommandPool & commandPool)
{
	int len = strlen(filename);
	if (filename[len-1] == '/' || len == 0)
	{
		filename = "working/texture/87face.png";
	}
	else
	{
		//filename = "working/model/sponza-gltf-pbr/1180362371129094107.png";
	}

	auto it = map.find(filename);
	if (it == map.end())
	{
		Texture tex = {};
		tex.mipmapsLevel = 1;
		//LoadVkImage(filename, commandPool, tex.image, tex.imageView);
		LoadVkImageMipmap(filename, commandPool, tex.image, tex.imageView, tex.mipmapsLevel);
		map[filename] = tex;
		return &map[filename];
	}


	return &it->second;
}
