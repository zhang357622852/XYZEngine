#include "stdafx.h"
#include "VulkanPipeline.h"

#include "VulkanContext.h"
#include "VulkanFramebuffer.h"

namespace XYZ {
	namespace Utils
	{
		static VkPrimitiveTopology GetVulkanTopology(PrimitiveTopology topology)
		{
			switch (topology)
			{
			case PrimitiveTopology::Points:			return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
			case PrimitiveTopology::Lines:			return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
			case PrimitiveTopology::Triangles:		return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			case PrimitiveTopology::LineStrip:		return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
			case PrimitiveTopology::TriangleStrip:	return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
			case PrimitiveTopology::TriangleFan:	return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
			}

			XYZ_ASSERT(false, "Unknown toplogy");
			return VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
		}
		static VkFormat ShaderDataTypeToVulkanFormat(ShaderDataType type)
		{
			switch (type)
			{
			case ShaderDataType::Float:     return VK_FORMAT_R32_SFLOAT;
			case ShaderDataType::Float2:    return VK_FORMAT_R32G32_SFLOAT;
			case ShaderDataType::Float3:    return VK_FORMAT_R32G32B32_SFLOAT;
			case ShaderDataType::Float4:    return VK_FORMAT_R32G32B32A32_SFLOAT;
			}
			XYZ_ASSERT(false, "");
			return VK_FORMAT_UNDEFINED;
		}

	}


	VulkanPipeline::VulkanPipeline(const PipelineSpecification& specs)
		:
		m_Specification(specs),
		m_PipelineLayout(VK_NULL_HANDLE),
		m_VulkanPipeline(VK_NULL_HANDLE)
	{
		XYZ_ASSERT(specs.Shader.Raw() && specs.RenderPass.Raw(), "");
		Invalidate();
	}
	VulkanPipeline::~VulkanPipeline()
	{
		VkPipelineLayout pipelineLayout = m_PipelineLayout;
		VkPipeline		 vulkanPipeline = m_VulkanPipeline;
		Renderer::Submit([pipelineLayout, vulkanPipeline]() {		
			destroy(pipelineLayout, vulkanPipeline);
		});	
	}
	void VulkanPipeline::Invalidate()
	{
		Ref<VulkanPipeline> instance = this;
		Renderer::Submit([instance]() mutable {

			instance->destroy(instance->m_PipelineLayout, instance->m_VulkanPipeline);
			VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {};
			inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			inputAssemblyState.topology = Utils::GetVulkanTopology(instance->m_Specification.Topology);
			
			instance->createPipelineLayoutInfo();
			VkPipelineRasterizationStateCreateInfo rasterizationInfo = instance->createRasterizationInfo();
			VkPipelineViewportStateCreateInfo      viewportStateInfo = instance->createViewportStateInfo();
			VkVertexInputBindingDescription vertexInputBinding;
			std::vector<VkVertexInputAttributeDescription> vertexInputAttributs;
			VkPipelineVertexInputStateCreateInfo   vertexInputInfo	 = instance->createVertexInputInfo(vertexInputBinding, vertexInputAttributs);
			VkPipelineMultisampleStateCreateInfo   multisampleInfo	 = instance->createMultisampleInfo();
			VkPipelineDepthStencilStateCreateInfo  depthStencilInfo  = instance->createDepthStencilInfo();
			std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
			VkPipelineColorBlendStateCreateInfo    colorBlendInfo	 = instance->createColorBlendInfo(colorBlendAttachments);		
			std::vector<VkDynamicState> dynamicStateEnables;
			VkPipelineDynamicStateCreateInfo	   dynamicStateInfo  = instance->createDynamicStateInfo(dynamicStateEnables);
			
			Ref<VulkanFramebuffer> framebuffer = instance->m_Specification.RenderPass->GetSpecification().TargetFramebuffer;
	
			VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
			pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			pipelineCreateInfo.layout = instance->m_PipelineLayout;
			pipelineCreateInfo.renderPass = framebuffer->GetRenderPass();
			
			pipelineCreateInfo.pVertexInputState = &vertexInputInfo;
			pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
			pipelineCreateInfo.pViewportState = &viewportStateInfo;
			pipelineCreateInfo.pRasterizationState = &rasterizationInfo;
			pipelineCreateInfo.pMultisampleState = &multisampleInfo;
			pipelineCreateInfo.pDepthStencilState = &depthStencilInfo;
			pipelineCreateInfo.pColorBlendState = &colorBlendInfo;
			pipelineCreateInfo.pDynamicState = &dynamicStateInfo;

			Ref<VulkanShader> vulkanShader = Ref<VulkanShader>(instance->m_Specification.Shader);
			const auto& shaderStages = vulkanShader->GetPipelineShaderStageCreateInfos();

			pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
			pipelineCreateInfo.pStages = shaderStages.data();
			// It is possible to derive from existing pipeline ( better performance )
			// pipelineCreateInfo.flags = VK_PIPELINE_CREATE_DERIVATIVE_BIT;
			pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
			pipelineCreateInfo.basePipelineIndex = -1; // Optional

			VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
			
			VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &instance->m_VulkanPipeline));
		});
	}
	void VulkanPipeline::SetUniformBuffer(Ref<UniformBuffer> uniformBuffer, uint32_t binding, uint32_t set)
	{
	}
	VkPipelineMultisampleStateCreateInfo VulkanPipeline::createMultisampleInfo() const
	{
		VkPipelineMultisampleStateCreateInfo multisampleState = {};
		multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampleState.pSampleMask = nullptr;
		return multisampleState;
	}
	VkPipelineRasterizationStateCreateInfo VulkanPipeline::createRasterizationInfo() const
	{
		// Rasterization state
		VkPipelineRasterizationStateCreateInfo rasterizationState = {};
		rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizationState.polygonMode = m_Specification.Wireframe ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
		rasterizationState.cullMode = m_Specification.BackfaceCulling ? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_NONE;
		rasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizationState.depthClampEnable = VK_FALSE;
		rasterizationState.rasterizerDiscardEnable = VK_FALSE;
		rasterizationState.depthBiasEnable = VK_FALSE;
		rasterizationState.lineWidth = m_Specification.LineWidth; // this is dynamic
		return rasterizationState;
	}
	VkPipelineViewportStateCreateInfo VulkanPipeline::createViewportStateInfo() const
	{
		VkPipelineViewportStateCreateInfo viewportState = {};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;


		return viewportState;
	}
	VkPipelineVertexInputStateCreateInfo VulkanPipeline::createVertexInputInfo(VkVertexInputBindingDescription& vertexInputBinding, std::vector<VkVertexInputAttributeDescription>& vertexInputAttributs) const
	{
		const BufferLayout& layout = m_Specification.Layout;
		vertexInputBinding.binding = 0;
		vertexInputBinding.stride = layout.GetStride();
		vertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		
		// Input attribute bindings describe shader attribute locations and memory layouts
		vertexInputAttributs.resize(layout.GetElements().size());
		uint32_t location = 0;
		for (const auto &element : layout)
		{
			vertexInputAttributs[location].binding = 0;
			vertexInputAttributs[location].location = location;
			vertexInputAttributs[location].format = Utils::ShaderDataTypeToVulkanFormat(element.Component);
			vertexInputAttributs[location].offset = element.Offset;
			location++;
		}

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexAttributeDescriptionCount = (uint32_t)vertexInputAttributs.size();
		
		
		if (layout.GetElements().empty())
		{
			vertexInputInfo.pVertexAttributeDescriptions = VK_NULL_HANDLE;
			vertexInputInfo.vertexBindingDescriptionCount = 0;
			vertexInputInfo.pVertexBindingDescriptions = VK_NULL_HANDLE;
		}
		else
		{
			vertexInputInfo.pVertexAttributeDescriptions = vertexInputAttributs.data();
			vertexInputInfo.vertexBindingDescriptionCount = 1;
			vertexInputInfo.pVertexBindingDescriptions = &vertexInputBinding;
		}
		return vertexInputInfo;
	}
	VkPipelineDepthStencilStateCreateInfo VulkanPipeline::createDepthStencilInfo() const
	{
		// Depth and stencil state containing depth and stencil compare and test operations
		// We only use depth tests and want depth tests and writes to be enabled and compare with less or equal
		VkPipelineDepthStencilStateCreateInfo depthStencilState = {};
		depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilState.depthTestEnable = m_Specification.DepthTest ? VK_TRUE : VK_FALSE;
		depthStencilState.depthWriteEnable = m_Specification.DepthWrite ? VK_TRUE : VK_FALSE;
		depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		depthStencilState.depthBoundsTestEnable = VK_FALSE;
		depthStencilState.back.failOp = VK_STENCIL_OP_KEEP;
		depthStencilState.back.passOp = VK_STENCIL_OP_KEEP;
		depthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;
		depthStencilState.stencilTestEnable = VK_FALSE;
		depthStencilState.front = depthStencilState.back;
		return depthStencilState;
	}
	std::vector<VkPipelineColorBlendAttachmentState> VulkanPipeline::createColorBlendAttachments() const
	{
		Ref<Framebuffer> framebuffer = m_Specification.RenderPass->GetSpecification().TargetFramebuffer;
		const size_t colorAttachmentCount = framebuffer->GetSpecification().SwapChainTarget ? 1 : framebuffer->GetNumColorAttachments();
		std::vector<VkPipelineColorBlendAttachmentState> blendAttachmentStates(colorAttachmentCount);
		if (framebuffer->GetSpecification().SwapChainTarget)
		{
			blendAttachmentStates[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			blendAttachmentStates[0].blendEnable = VK_TRUE;
			blendAttachmentStates[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			blendAttachmentStates[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			blendAttachmentStates[0].colorBlendOp = VK_BLEND_OP_ADD;
			blendAttachmentStates[0].alphaBlendOp = VK_BLEND_OP_ADD;
			blendAttachmentStates[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			blendAttachmentStates[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		}
		else
		{
			//for (size_t i = 0; i < colorAttachmentCount; i++)
			//{
			//	if (!framebuffer->GetSpecification().Blend)
			//		break;
			//
			//	blendAttachmentStates[i].colorWriteMask = 0xf;
			//	if (!framebuffer->GetSpecification().Blend)
			//		break;
			//
			//	const auto& attachmentSpec = framebuffer->GetSpecification().Attachments.Attachments[i];
			//	FramebufferBlendMode blendMode = framebuffer->GetSpecification().BlendMode == FramebufferBlendMode::None
			//		? attachmentSpec.BlendMode
			//		: framebuffer->GetSpecification().BlendMode;
			//
			//	blendAttachmentStates[i].blendEnable = attachmentSpec.Blend ? VK_TRUE : VK_FALSE;
			//	if (blendMode == FramebufferBlendMode::SrcAlphaOneMinusSrcAlpha)
			//	{
			//		blendAttachmentStates[i].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			//		blendAttachmentStates[i].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			//		blendAttachmentStates[i].srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			//		blendAttachmentStates[i].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			//	}
			//	else if (blendMode == FramebufferBlendMode::OneZero)
			//	{
			//		blendAttachmentStates[i].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
			//		blendAttachmentStates[i].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
			//		blendAttachmentStates[i].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			//		blendAttachmentStates[i].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			//	}
			//	else if (blendMode == FramebufferBlendMode::Zero_SrcColor)
			//	{
			//		blendAttachmentStates[i].srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
			//		blendAttachmentStates[i].dstColorBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
			//		blendAttachmentStates[i].srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			//		blendAttachmentStates[i].dstAlphaBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
			//	}
			//	else
			//	{
			//		XYZ_ASSERT(false, "");
			//	}
			//	blendAttachmentStates[i].colorBlendOp = VK_BLEND_OP_ADD;
			//	blendAttachmentStates[i].alphaBlendOp = VK_BLEND_OP_ADD;
			//}
		}
		return blendAttachmentStates;
	}
	VkPipelineColorBlendStateCreateInfo VulkanPipeline::createColorBlendInfo(std::vector<VkPipelineColorBlendAttachmentState>& colorBlendAttachments) const
	{
		colorBlendAttachments = createColorBlendAttachments();
		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.attachmentCount = static_cast<uint32_t>(colorBlendAttachments.size());
		colorBlending.pAttachments = colorBlendAttachments.data();
		//colorBlending.blendConstants[0] = 0.0f; // Optional
		//colorBlending.blendConstants[1] = 0.0f; // Optional
		//colorBlending.blendConstants[2] = 0.0f; // Optional
		//colorBlending.blendConstants[3] = 0.0f; // Optional
		return colorBlending;
	}
	VkPipelineDynamicStateCreateInfo VulkanPipeline::createDynamicStateInfo(std::vector<VkDynamicState>& dynamicStateEnables) const
	{
		// Enable dynamic states
		// Most states are baked into the pipeline, but there are still a few dynamic states that can be changed within a command buffer
		// To be able to change these we need do specify which dynamic states will be changed using this pipeline. Their actual states are set later on in the command buffer.
		// For this example we will set the viewport and scissor using dynamic states
		dynamicStateEnables.push_back(VK_DYNAMIC_STATE_VIEWPORT);
		dynamicStateEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);
		if (m_Specification.Topology == PrimitiveTopology::Lines
			|| m_Specification.Topology == PrimitiveTopology::LineStrip
			|| m_Specification.Wireframe)
			dynamicStateEnables.push_back(VK_DYNAMIC_STATE_LINE_WIDTH);

		VkPipelineDynamicStateCreateInfo dynamicState = {};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.pDynamicStates = dynamicStateEnables.data();
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());
		return dynamicState;
	}

	void VulkanPipeline::destroy(VkPipelineLayout pipelineLayout, VkPipeline vulkanPipeline)
	{
		if (pipelineLayout != VK_NULL_HANDLE && vulkanPipeline != VK_NULL_HANDLE)
		{
			const VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
			VK_CHECK_RESULT(vkDeviceWaitIdle(device));
			vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
			vkDestroyPipeline(device, vulkanPipeline, nullptr);
		}
	}

	void VulkanPipeline::createPipelineLayoutInfo()
	{
		Ref<VulkanShader> vulkanShader(m_Specification.Shader);
		const auto descriptorSetLayouts = vulkanShader->GetDescriptorSetLayouts();
		const auto& pushConstantRanges = vulkanShader->GetPushConstantRanges();

		// TODO: should come from shader
		std::vector<VkPushConstantRange> vulkanPushConstantRanges(pushConstantRanges.size());
		for (uint32_t i = 0; i < pushConstantRanges.size(); i++)
		{
			const auto& pushConstantRange = pushConstantRanges[i];
			auto& vulkanPushConstantRange = vulkanPushConstantRanges[i];

			vulkanPushConstantRange.stageFlags = pushConstantRange.ShaderStage;
			vulkanPushConstantRange.offset = pushConstantRange.Offset;
			vulkanPushConstantRange.size = pushConstantRange.Size;
		}


		const VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.pNext = nullptr;
		pipelineLayoutCreateInfo.setLayoutCount = (uint32_t)descriptorSetLayouts.size();
		pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutCreateInfo.pushConstantRangeCount = (uint32_t)vulkanPushConstantRanges.size();
		pipelineLayoutCreateInfo.pPushConstantRanges = vulkanPushConstantRanges.data();
		VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &m_PipelineLayout));
	}
}