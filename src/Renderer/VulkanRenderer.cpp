#include "VulkanRenderer.h"
#include "VulkanLib/VulkanInstance.h"
#include "VulkanLib/vkHelper.h"
#include <vector>
#include <typeinfo>
#include <chrono>
#include <ratio>
#include <Windows.h>


typedef unsigned int uint;



Vec3 make_normal(const Vec3 & v1, const Vec3 & v2, const Vec3 & v3) {
  Vec3 d1 = v2 - v1;
  Vec3 d2 = v3 - v1;

  return (Vec3::Cross(d1, d2)).normalized();
}


VulkanRenderer::VulkanRenderer(VulkanLib::VulkanInstance vkInstance) {

	//if (HMODULE mod = GetModuleHandleA("renderdoc.dll"))
	if (HMODULE mod = LoadLibraryA("renderdoc.dll"))
	{
		std::cout << "handle loaded" << std::endl;

	}


	//vulkanInstance = VulkanLib::VulkanInstance();
	vulkanInstance = vkInstance;


	renderBufferAttribute = 
	{
    	Shader_Attribute_Buffer,   // 0 
    	Shader_Attribute_Unifrom,  // 1
    	Shader_Attribute_Buffer,   // 2 
    	Shader_Attribute_Buffer,   // 3 
    	Shader_Attribute_Buffer,   // 4 
    	Shader_Attribute_Buffer,   // 5 
    	Shader_Attribute_Buffer,   // 6 
    	Shader_Attribute_Buffer,   // 7
    	Shader_Attribute_Buffer,   // 8	
    	Shader_Attribute_Buffer,   // 9
	};


	// Load shader module
	std::string filename = "pathTracing.spv";
	VkShaderModule shaderModule = createShaderModule(vulkanInstance.device, filename);
	pipeline = vulkanInstance.BuildComputeShaderPipeline(renderBufferAttribute, shaderModule);

	// Load Clear Image Shader
	filename = "clearImg.spv";
	VkShaderModule clearShaderModule = createShaderModule(vulkanInstance.device, filename);
	clearPipeline = vulkanInstance.BuildComputeShaderPipeline( 
		{
			Shader_Attribute_Buffer,
			Shader_Attribute_Unifrom,
			Shader_Attribute_Buffer
		},
		clearShaderModule
	);


	commandPool = createCommandPool(vulkanInstance.device, vulkanInstance.queueFamily);
	commandBuffers = AllocateCommandBuffer(vulkanInstance.device, commandPool, 2);


}


VulkanRenderer::~VulkanRenderer() {

}

void VulkanRenderer::StartRender() {
	InitBuffer();
	//std::cout << "init buffer!" << std::endl;
	SetData();
	//std::cout << "set up date done!" << std::endl;
	InitRenderCommand();
	//std::cout << "init render command!" << std::endl;
	InitClearImageCommand();

	iteratorCount = 0;
	renderThread = std::thread(&VulkanRenderer::RenderTask, this);
}

void VulkanRenderer::SetData() {

	// Set Up data

	//shape instance
	std::vector<ShapeInstance> shapeDatas = {};

	// shape sphere
	std::vector<SphereShape> sphereDatas = {};

	// shape triangle
	std::vector<TriangleShape> traingleDatas = {};

	// shape Plane
	std::vector<PlaneShape> planeDatas = {};

	// material
	std::vector<GPU_Material> materials = {};

	// integrator


	//std::cout << "shape material done!" << std::endl;


	// Set up buffer
	VkWriteDescriptorSet * writeDescriptorSets = new VkWriteDescriptorSet[buffers.size()];
	// std::cout << "buffer size " << buffers.size() << std::endl;
	for(int i = 0; i < buffers.size(); ++i) {
		VkWriteDescriptorSet writeDescriptorSet = createWriteDescriptorSet(
			pipeline.descriptorSet, 
			GetDescriptorType(renderBufferAttribute[i]), 
			1, 
			&buffers[i].descriptorBufInfo, 
			i);
		writeDescriptorSets[i] = writeDescriptorSet;
	}

	vkUpdateDescriptorSets(vulkanInstance.device, buffers.size(), &writeDescriptorSets[0], 0, NULL);
	//std::cout << "update descriptorSet done!" << std::endl;


	//Init Clear Image shader 
	VkWriteDescriptorSet clearWriteDescriptorSets[3] = 
	{
		// Image
		createWriteDescriptorSet(
			clearPipeline.descriptorSet, 
			GetDescriptorType(renderBufferAttribute[0]), 
			1, 
			&buffers[0].descriptorBufInfo, 
			0
		),
		// Render setting
		createWriteDescriptorSet(
			clearPipeline.descriptorSet, 
			GetDescriptorType(renderBufferAttribute[1]), 
			1, 
			&buffers[1].descriptorBufInfo, 
			1
		),
		// Integrator
		createWriteDescriptorSet(
			clearPipeline.descriptorSet, 
			GetDescriptorType(renderBufferAttribute[7]), 
			1, 
			&buffers[7].descriptorBufInfo, 
			2
		),						
	};

	vkUpdateDescriptorSets(vulkanInstance.device, 3, &clearWriteDescriptorSets[0], 0, NULL);




	VkDevice & device = vulkanInstance.device;
	// Set Buffer Data

	//0 Render Image
	//CopyDataToDeviceMemory(device, buffers[0].memory, buffers[0].size, cam->GetBuffer());

	//1 Render Setting
	CopyDataToDeviceMemory(device, buffers[1].memory, buffers[1].size, &render_data);

	//2 Shape Instance
	size_t shapeBufSize = sizeof(int) + sizeof(ShapeInstance) * shapeDatas.size();
	void * shapeInstanceMem = new char[shapeBufSize];
	// int shapeSize = shapeDatas.size();
	int shapeSize;// = s->tree.nodes.size();
	memcpy(shapeInstanceMem, &shapeSize, sizeof(int));
	shapeInstanceMem = static_cast<char*>(shapeInstanceMem) + sizeof(int);
	memcpy(shapeInstanceMem, shapeDatas.data(), sizeof(ShapeInstance) * shapeDatas.size());
	shapeInstanceMem = static_cast<char*>(shapeInstanceMem) - sizeof(int);
	CopyDataToDeviceMemory(device, buffers[2].memory, shapeBufSize, shapeInstanceMem); 

	// if (shapeDatas.size() > 0 ) CopyDataToDeviceMemory(device, buffers[2].memory, sizeof(ShapeInstance) * shapeDatas.size(), shapeDatas.data());

	//3 Shape Sphere
	if (sphereDatas.size() > 0 ) CopyDataToDeviceMemory(device, buffers[3].memory, sizeof(SphereShape) * sphereDatas.size(), sphereDatas.data());

	//4 Shape Triangle
	//std::cout << "triangle data size : " << traingleDatas.size() << std::endl;
	if (traingleDatas.size() > 0 ) CopyDataToDeviceMemory(device, buffers[4].memory, sizeof(TriangleShape) * traingleDatas.size(), traingleDatas.data());
	//std::cout << "triangle data copy done : " << std::endl;

	//5 Shape Plane
	//std::cout << "plane data size : " << planeDatas.size() << std::endl;
	if (planeDatas.size() > 0 ) CopyDataToDeviceMemory(device, buffers[5].memory, sizeof(PlaneShape) * planeDatas.size(), planeDatas.data());

	//6 Material
	//std::cout << "material data size : " << materials.size() << std::endl;
	if (materials.size() > 0 ) CopyDataToDeviceMemory(device, buffers[6].memory, sizeof(GPU_Material) * materials.size(), materials.data());

	//7 Sample Integrator	
	CopyDataToDeviceMemory(device, buffers[7].memory, sizeof(SampleIntegrator) * cam->GetWidth() * cam->GetHeight(), cam->GetIntegrator());

	//9 BVH Tree
	//std::cout << "bvh data size : " << s->tree.nodes.size() << std::endl;	
	
	//if (s->tree.nodes.size() > 0 ) CopyDataToDeviceMemory(device, buffers[9].memory, sizeof(bvh_node) * s->tree.nodes.size(), s->tree.nodes.data());
	//std::cout << "copy buffer done!" << std::endl;


}

void VulkanRenderer::InitRenderCommand() {
	vkGetDeviceQueue(vulkanInstance.device, vulkanInstance.queueFamily, 0, &queue);  	

	int width = cam->GetWidth();
	int height = cam->GetHeight();

	
	AddComputeShaderCommand(
	  //width / 16, height / 16, 1,
	  width / 8, height / 8, 1,
	  // width / 4, height / 4, 1,
	  vulkanInstance.device,
	  commandBuffers[0], 
	  pipeline.pipeline, 
	  pipeline.pipelineLayout,
	  &pipeline.descriptorSet
	);		
}

void VulkanRenderer::InitClearImageCommand() {
	vkGetDeviceQueue(vulkanInstance.device, vulkanInstance.queueFamily, 0, &queue);  	

	int width = cam->GetWidth();
	int height = cam->GetHeight();

	
	AddComputeShaderCommand(
	  width / 16, height / 16, 1,
	  vulkanInstance.device,
	  commandBuffers[1], 
	  clearPipeline.pipeline, 
	  clearPipeline.pipelineLayout,
	  &clearPipeline.descriptorSet
	);
}

void VulkanRenderer::SubmitRender() {


	if (cam == nullptr) return;
	submitCommand(vulkanInstance.device, commandBuffers[0], queue);

}

void VulkanRenderer::SubmitClear() {

	if (cam == nullptr) return;
	submitCommand(vulkanInstance.device, commandBuffers[1], queue);

}



void VulkanRenderer::ClenaBuffer() {
	for ( int i = 0; i < buffers.size(); ++i) {
  		vkDestroyBuffer(vulkanInstance.device, buffers[i].buffer, nullptr);
	}
}

void VulkanRenderer::InitBuffer() {



	//init render data
	render_data = { uint(cam->GetWidth()), uint(cam->GetHeight()), int(time(NULL)), 0, {}, 0};




	ClenaBuffer();

  	const int kShapeInstanceSize = 1000;
  	const int kMaterialInstanceSize = 1000;
  	uint imageBufferSize = 3 * sizeof(float) * cam->GetWidth() * cam->GetHeight();
	size_t shapeBufSize = sizeof(int) + sizeof(ShapeInstance) * kShapeInstanceSize * 30000;

	VkDevice & device = vulkanInstance.device;
	VkPhysicalDeviceMemoryProperties & deviceMemProps = vulkanInstance.deviceMemProps;

	buffers = //std::vector<BufferObject>() 
	{
		//0 Render Image
		BuildBuffer(device, deviceMemProps, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, imageBufferSize),
		//1 Render Setting
		BuildBuffer(device, deviceMemProps, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(vkRenderData)),
		//2 Shape Instance
		BuildBuffer(device, deviceMemProps, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, shapeBufSize),
		//3 Shape Sphere
		BuildBuffer(device, deviceMemProps, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, sizeof(SphereShape) * kShapeInstanceSize),
		//4 Shape Triangle
		BuildBuffer(device, deviceMemProps, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, max(1, (int)(sizeof(TriangleShape) * kShapeInstanceSize * 3000))),
		//5 Shape Plane
		BuildBuffer(device, deviceMemProps, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, sizeof(PlaneShape) * kShapeInstanceSize),
		//6 Material
		BuildBuffer(device, deviceMemProps, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, sizeof(GPU_Material) * kMaterialInstanceSize),
		//7 Sample Integrator
		BuildBuffer(device, deviceMemProps, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, sizeof(SampleIntegrator) * cam->GetWidth() * cam->GetHeight()),
		//8 Debug
		BuildBuffer(device, deviceMemProps, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, sizeof(SampleIntegrator)),
		//9 BVH
		//BuildBuffer(device, deviceMemProps, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, sizeof(bvh_node) * kShapeInstanceSize * 3000),
	};

}


void VulkanRenderer::RenderTask() {
	int imageBufferSize = 3 * sizeof(float) * cam->GetWidth() * cam->GetHeight();
	// void * mem;
	vkMapMemory(vulkanInstance.device, buffers[0].memory, 0, imageBufferSize, 0, &mem);

	//std::cout << "Width : " << render_data.width << std::endl;
	//std::cout << "Height : " << render_data.height << std::endl;


	bool debugFlag = false;

	struct debugType
	{
		float v1_x, v1_y, v1_z, v1_w;
		float v2_x, v2_y, v2_z, v2_w;
		float v3_x, v3_y, v3_z, v3_w;
		int   v4_x, v4_y, v4_z, v4_w;
	};

	//cam->transform.position = {1, 2, -10};
	//cam->transform.UpdateMatrix();
    auto start = std::chrono::steady_clock::now();

	void * pMemData;
	vkMapMemory(vulkanInstance.device, buffers[1].memory, 0, buffers[1].size, 0, &pMemData);
	//memcpy(pMemData, &render_data, buffers[1].size);



	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	while(true) {
		if (!running) continue;

		iteratorCount++;
		render_data.sampleCount = iteratorCount;
		render_data.time = time(NULL) + iteratorCount;
		render_data.camera_pos = cam->transform.position;
		render_data.camMatrix = cam->transform.modelMatrix;
		//CopyDataToDeviceMemory(vulkanInstance.device, buffers[1].memory, buffers[1].size, &render_data);
		memcpy(pMemData, &render_data, buffers[1].size);

		if (clearFlag) {
			SubmitClear();
			clearFlag = false;
		}

		SubmitRender();

    	// memcpy(cam->GetBuffer(), mem, imageBufferSize);

		/*
		if (!debugFlag) {
			debugFlag = true;
			void* debugMem;
			vkMapMemory(vulkanInstance.device, buffers[8].memory, 0, sizeof(debugType), 0, &debugMem);
		    
			bvh_node node = s->tree.nodes[6];

		    std::cout << "node data : " << std::endl;
		    std::cout << "v1 " << node.boundingBox.min.x << "  " << node.boundingBox.min.y << "  " << node.boundingBox.min.z << "  " << node.boundingBox.pad << std::endl;
			std::cout << "v2 " << node.boundingBox.max.x << "  " << node.boundingBox.max.y << "  " << node.boundingBox.max.z << "  " << node.boundingBox.pad1 << std::endl;
			// std::cout << "v3 " << debug.v3_x << "  " << debug.v3_y << "  " << debug.v3_z << "  " << debug.v3_w << std::endl;
			std::cout << "v4 " << node.idx << "  " << node.primitiveId << "  " << node.isLeaf << "  " << node.left << std::endl;


		    debugType debug;
		    memcpy(&debug, debugMem, sizeof(debugType));
		    std::cout << "Debug : " << std::endl;
		    std::cout << "v1 " << debug.v1_x << "  " << debug.v1_y << "  " << debug.v1_z << "  " << debug.v1_w << std::endl;
			std::cout << "v2 " << debug.v2_x << "  " << debug.v2_y << "  " << debug.v2_z << "  " << debug.v2_w << std::endl;
			std::cout << "v3 " << debug.v3_x << "  " << debug.v3_y << "  " << debug.v3_z << "  " << debug.v3_w << std::endl;
			std::cout << "v4 " << debug.v4_x << "  " << debug.v4_y << "  " << debug.v4_z << "  " << debug.v4_w << std::endl;
		}
		*/		

    	//if (iteratorCount % 10 == 0 ) 
    	//	std::cout << "sample count " << iteratorCount << std::endl;

		if (iteratorCount == 20) {
			auto end = std::chrono::steady_clock::now();

			// Store the time difference between start and end
			auto diff = end - start;
			std::cout << "Render 20 iterate time " << std::chrono::duration <double, std::milli> (diff).count() << " ms" << std::endl;
		}    

		//vkQueuePresentKHR(queue,&presentInfo);
	}

	vkUnmapMemory(vulkanInstance.device, buffers[1].memory);

}

void VulkanRenderer::UpdateFrame() {
	int imageBufferSize = 3 * sizeof(float) * cam->GetWidth() * cam->GetHeight();

    //vkUnmapMemory(vulkanInstance.device, buffers[0].memory);
}



void VulkanRenderer::ClearImage() {
	clearFlag = true;
}

