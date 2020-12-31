#ifndef VK_SYS_HEADER
#define VK_SYS_HEADER
#pragma once

#include "VulkanLib/vkHelper.h"
#include "VulkanLib/VulkanInstance.h"
#include "GLFW/glfw3.h"
#include <string>
#include <map>

namespace vkSys {
	class VkInstance {
	private:		
		static VulkanLib::VulkanInstance instance;
		static bool bInited;
	public:
		static VulkanLib::VulkanInstance * GetInstance() {
			if (!VkInstance::bInited)
			{
				uint32_t count;
				auto requires = glfwGetRequiredInstanceExtensions(&count);
				std::vector<const char*> extensions = {};
				for (int i = 0; i < count; ++i) {
					extensions.push_back(requires[i]);
				}
				VkInstance::instance = VulkanLib::VulkanInstance(extensions.size(), extensions.data());;
				VkInstance::bInited = true;
			}
			return &VkInstance::instance;
		}
	};

	static const uint32_t Sampler2D = 1;
	static const uint32_t Sampler2D_ClampEdge = 2;
	static const uint32_t SamplerMax = 10;
	/*	
	enum SamplerKind 
	{
		Sampler2D = 1,
		SamplerMax = 10,
	};
	*/

	class Sampler {
	public:
		static VkSampler samplers[SamplerMax];


		static VkSampler GetSampler(uint32_t samplerKind, int mipmaps);
		static VkSampler Create2DSampler(int mipmaps);
		static VkSampler Create2DClampEdgeSampler(int mipmaps);
	};


	class TexMgr
	{
	public:
		struct Texture 
		{
			VkImage image;
			VkImageView imageView;
			int mipmapsLevel;
		};


		static Texture * GetTexture(const char * filename, const VkCommandPool & commandPool);


	private:
		static std::map<std::string, Texture> map;

	};

}


#endif