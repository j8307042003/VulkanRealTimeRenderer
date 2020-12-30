#include<vector>
#include"Shader.h"
#include "VulkanLib/vkHelper.h"
#include <array>
#include "SPIRV-Reflect/spirv_reflect.h"


void LoadReflectionData(int codeSize, const char * code, SpvReflectShaderModule * spvModule, std::vector<SpvReflectDescriptorBinding*> & variables)
{
	auto result = spvReflectCreateShaderModule(codeSize, code, spvModule);

	if (result != SPV_REFLECT_RESULT_SUCCESS)
	{
		std::cout << "vertex shader reflection fail!";
	}

	uint32_t variableCount;
	spvReflectEnumerateDescriptorBindings(spvModule, &variableCount, nullptr);
	variables.resize(variableCount);
	spvReflectEnumerateDescriptorBindings(spvModule, &variableCount, variables.data());
}


Shader::Shader(VkDevice & device, const char * vertexName, const char * fragmentName) : device(device)
{

	ShaderModules * pShaderModules = new ShaderModules();
	modules = std::shared_ptr<ShaderModules>(pShaderModules);

	std::vector<char> vertexShaderCode = {};
	std::vector<char> fragmentShaderCode = {};

	LoadFile(vertexName, vertexShaderCode);
	LoadFile(fragmentName, fragmentShaderCode);

	pShaderModules->vertexShader = createShaderModule(device, vertexShaderCode);
	pShaderModules->fragmentShader = createShaderModule(device, fragmentShaderCode);

	LoadReflectionData(vertexShaderCode.size(), vertexShaderCode.data(), &vertexSpvModule, vertexVariables);
	LoadReflectionData(fragmentShaderCode.size(), fragmentShaderCode.data(), &fragmentSpvModule, fragmentVariables);


	//todo delete reflect data
}	


Shader::~Shader()
{
}



 VkPipelineShaderStageCreateInfo Shader::makeShaderStageCreateInfo(VkShaderStageFlagBits shaderStage, const VkShaderModule & shaderModule) const
 {
	VkPipelineShaderStageCreateInfo stageInfo = {};
	stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stageInfo.stage = shaderStage;
	stageInfo.module = shaderModule;
	stageInfo.pName = "main"; 	

	return stageInfo;
 }


VkPipeline Shader::BuildPipeline(const VkRenderPass & renderpass, const int & subpass, const VkPipelineLayout & pipelineLayout, const VkPipelineVertexInputStateCreateInfo * pCustomVertexInputState) const
{

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages = {};
	ShaderModules * pShaderModules = modules.get();
	if (pShaderModules->vertexShader != VK_NULL_HANDLE) shaderStages.push_back(makeShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, pShaderModules->vertexShader));
	if (pShaderModules->fragmentShader != VK_NULL_HANDLE) shaderStages.push_back(makeShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, pShaderModules->fragmentShader));



	std::array<VkVertexInputBindingDescription, 1> vertexBindingDescriptions = {};
	vertexBindingDescriptions[0].binding = 0;
    vertexBindingDescriptions[0].stride = 4 * sizeof(float);
    vertexBindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX ;

    std::array<VkVertexInputAttributeDescription, 1> vertexInputAttributeDescriptions = {};
	vertexInputAttributeDescriptions[0].location = 0;
	vertexInputAttributeDescriptions[0].binding = 0;
	vertexInputAttributeDescriptions[0].format = VK_FORMAT_R32_SFLOAT;
	vertexInputAttributeDescriptions[0].offset = 0;

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &vertexBindingDescriptions[0]; // Optional
	vertexInputInfo.vertexAttributeDescriptionCount = 1;
	vertexInputInfo.pVertexAttributeDescriptions = &vertexInputAttributeDescriptions[0]; // Optional


	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;


	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;


	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f;
	rasterizer.depthBiasClamp = 0.0f;
	rasterizer.depthBiasSlopeFactor = 0.0f;



	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f;
	multisampling.pSampleMask = nullptr;
	multisampling.alphaToCoverageEnable = VK_FALSE;
	multisampling.alphaToOneEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional
	colorBlendAttachment.blendEnable = VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;



	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional






	VkDynamicState dynamicStates[] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
		VK_DYNAMIC_STATE_LINE_WIDTH
	};


	VkPipelineDynamicStateCreateInfo dynamicStateInfo = {};
	dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateInfo.dynamicStateCount = 3;
	dynamicStateInfo.pDynamicStates = dynamicStates;


	VkPipelineDepthStencilStateCreateInfo depthStencilStateInfo = { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
    depthStencilStateInfo.depthTestEnable = VK_TRUE;
    depthStencilStateInfo.depthWriteEnable = VK_TRUE;
    depthStencilStateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depthStencilStateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;


	VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.stageCount = shaderStages.size();
	pipelineCreateInfo.pStages = shaderStages.data();
	pipelineCreateInfo.pVertexInputState = pCustomVertexInputState != nullptr ? pCustomVertexInputState : &vertexInputInfo;
	pipelineCreateInfo.pInputAssemblyState = &inputAssembly;
	pipelineCreateInfo.pViewportState = &viewportState;
	pipelineCreateInfo.pRasterizationState = &rasterizer;
	pipelineCreateInfo.pMultisampleState = &multisampling;
	pipelineCreateInfo.pDepthStencilState = &depthStencilStateInfo;
	pipelineCreateInfo.pColorBlendState = &colorBlending;
	pipelineCreateInfo.pDynamicState = &dynamicStateInfo;
	pipelineCreateInfo.layout = pipelineLayout;
	pipelineCreateInfo.renderPass = renderpass;
	pipelineCreateInfo.subpass = subpass;
	pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineCreateInfo.basePipelineIndex = -1;



	VkPipeline pipeline = {};

	VkResult result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pipeline);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("create graphic pipeline error!!");
	}

	return pipeline;
}



VkPipeline Shader::BuildShadowPipeline(const VkRenderPass & renderpass, const int & subpass, const VkPipelineLayout & pipelineLayout, const VkPipelineVertexInputStateCreateInfo * pCustomVertexInputState) const
{

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages = {};
	ShaderModules * pShaderModules = modules.get();
	if (pShaderModules->vertexShader != VK_NULL_HANDLE) shaderStages.push_back(makeShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, pShaderModules->vertexShader));
	if (pShaderModules->fragmentShader != VK_NULL_HANDLE) shaderStages.push_back(makeShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, pShaderModules->fragmentShader));



	std::array<VkVertexInputBindingDescription, 1> vertexBindingDescriptions = {};
	vertexBindingDescriptions[0].binding = 0;
    vertexBindingDescriptions[0].stride = 4 * sizeof(float);
    vertexBindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX ;

    std::array<VkVertexInputAttributeDescription, 1> vertexInputAttributeDescriptions = {};
	vertexInputAttributeDescriptions[0].location = 0;
	vertexInputAttributeDescriptions[0].binding = 0;
	vertexInputAttributeDescriptions[0].format = VK_FORMAT_R32_SFLOAT;
	vertexInputAttributeDescriptions[0].offset = 0;

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &vertexBindingDescriptions[0]; // Optional
	vertexInputInfo.vertexAttributeDescriptionCount = 1;
	vertexInputInfo.pVertexAttributeDescriptions = &vertexInputAttributeDescriptions[0]; // Optional


	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;


	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;


	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f;
	rasterizer.depthBiasClamp = 0.0f;
	rasterizer.depthBiasSlopeFactor = 0.0f;



	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f;
	multisampling.pSampleMask = nullptr;
	multisampling.alphaToCoverageEnable = VK_FALSE;
	multisampling.alphaToOneEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional
	colorBlendAttachment.blendEnable = VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;



	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional






	VkDynamicState dynamicStates[] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
		VK_DYNAMIC_STATE_LINE_WIDTH
	};


	VkPipelineDynamicStateCreateInfo dynamicStateInfo = {};
	dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateInfo.dynamicStateCount = 3;
	dynamicStateInfo.pDynamicStates = dynamicStates;


	VkPipelineDepthStencilStateCreateInfo depthStencilStateInfo = { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
    depthStencilStateInfo.depthTestEnable = VK_TRUE;
    depthStencilStateInfo.depthWriteEnable = VK_TRUE;
    depthStencilStateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depthStencilStateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;


	VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.stageCount = shaderStages.size();
	pipelineCreateInfo.pStages = shaderStages.data();
	pipelineCreateInfo.pVertexInputState = pCustomVertexInputState != nullptr ? pCustomVertexInputState : &vertexInputInfo;
	pipelineCreateInfo.pInputAssemblyState = &inputAssembly;
	pipelineCreateInfo.pViewportState = &viewportState;
	pipelineCreateInfo.pRasterizationState = &rasterizer;
	pipelineCreateInfo.pMultisampleState = &multisampling;
	pipelineCreateInfo.pDepthStencilState = &depthStencilStateInfo;
	pipelineCreateInfo.pColorBlendState = &colorBlending;
	pipelineCreateInfo.pDynamicState = &dynamicStateInfo;
	pipelineCreateInfo.layout = pipelineLayout;
	pipelineCreateInfo.renderPass = renderpass;
	pipelineCreateInfo.subpass = subpass;
	pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineCreateInfo.basePipelineIndex = -1;



	VkPipeline pipeline = {};

	VkResult result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pipeline);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("create graphic pipeline error!!");
	}

	return pipeline;
}

