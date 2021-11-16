#include "stdafx.h"
#include "VulkanRendererAPI.h"

#include "VulkanRenderCommandBuffer.h"
#include "VulkanPipeline.h"
#include "VulkanRenderPass.h"
#include "VulkanFramebuffer.h"
#include "VulkanContext.h"
#include "VulkanVertexBuffer.h"
#include "VulkanIndexBuffer.h"

#include "XYZ/Core/Application.h"

namespace XYZ {
	void VulkanRendererAPI::Init()
	{
	}

	void VulkanRendererAPI::BeginFrame()
	{
		Ref<VulkanContext> context = Renderer::GetAPIContext();
		VulkanSwapChain& swapChain = context->GetSwapChain();
		const VkCommandBuffer commandBuffer = swapChain.GetCurrentCommandBuffer();
		VkCommandBufferBeginInfo cmdBufInfo = {};
		cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cmdBufInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		cmdBufInfo.pNext = nullptr;
		VK_CHECK_RESULT(vkBeginCommandBuffer(commandBuffer, &cmdBufInfo));
	}

	void VulkanRendererAPI::EndFrame()
	{
		Ref<VulkanContext> context = Renderer::GetAPIContext();
		VulkanSwapChain& swapChain = context->GetSwapChain();
		const VkCommandBuffer commandBuffer = swapChain.GetCurrentCommandBuffer();
		VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer));
	}

	void VulkanRendererAPI::TestDraw(const Ref<RenderPass>& renderPass, const Ref<RenderCommandBuffer>& commandBuffer, const Ref<Pipeline>& pipeline, const Ref<VertexBuffer>& vbo, const Ref<IndexBuffer>& ibo)
	{
		Ref<VulkanRenderCommandBuffer> vulkanCommandBuffer = commandBuffer;
		Ref<VulkanPipeline> vulkanPipeline = pipeline;
		Ref<VulkanFramebuffer> framebuffer = renderPass->GetSpecification().TargetFramebuffer;
		Ref<VulkanVertexBuffer> vulkanBuffer = vbo;
		Ref<VulkanIndexBuffer> indexBuffer = ibo;

		Renderer::Submit([vulkanCommandBuffer, vulkanPipeline, renderPass, framebuffer, vulkanBuffer, indexBuffer]() mutable {
			
			Ref<VulkanContext> vulkanContext = Renderer::GetAPIContext();
			const uint32_t frameIndex = vulkanContext->GetSwapChain().GetCurrentBufferIndex();
			uint32_t width = framebuffer->GetSpecification().Width;
			uint32_t height = framebuffer->GetSpecification().Height;

			VkViewport viewport = {};
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;

			VkRect2D scissor = {};
			scissor.offset = { 0, 0 };

			VkRenderPassBeginInfo renderPassBeginInfo = {};
			renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassBeginInfo.pNext = nullptr;
			renderPassBeginInfo.renderPass = framebuffer->GetRenderPass();
			renderPassBeginInfo.renderArea.offset.x = 0;
			renderPassBeginInfo.renderArea.offset.y = 0;
			renderPassBeginInfo.renderArea.extent.width = width;
			renderPassBeginInfo.renderArea.extent.height = height;
			if (framebuffer->GetSpecification().SwapChainTarget)
			{
				const VulkanSwapChain& swapChain = vulkanContext->GetSwapChain();
				width = swapChain.GetWidth();
				height = swapChain.GetHeight();
				renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				renderPassBeginInfo.pNext = nullptr;
				renderPassBeginInfo.renderPass = framebuffer->GetRenderPass();
				renderPassBeginInfo.renderArea.offset = { 0, 0 };
				renderPassBeginInfo.renderArea.extent = swapChain.GetExtent();
				renderPassBeginInfo.framebuffer = swapChain.GetCurrentFramebuffer();

				viewport = { 0.0f, 0.0f };
				viewport.width = (float)width;
				viewport.height = (float)height;

				scissor.extent = swapChain.GetExtent();
			}
			const VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
			renderPassBeginInfo.clearValueCount = 1;
			renderPassBeginInfo.pClearValues = &clearColor;
			const VkCommandBuffer commandBuffer = vulkanCommandBuffer->GetVulkanCommandBuffer(frameIndex);
			vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
			// Update dynamic viewport state
			vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
			// Update dynamic scissor state
					
			vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPipeline->GetVulkanPipeline());

			const VkBuffer vertexBuffers[] = { vulkanBuffer->GetVulkanBuffer() };
			const VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
			vkCmdBindIndexBuffer(commandBuffer, indexBuffer->GetVulkanBuffer(), 0, indexBuffer->GetVulkanIndexType());
			vkCmdDrawIndexed(commandBuffer, indexBuffer->GetCount(), 1, 0, 0, 0);
			vkCmdEndRenderPass(commandBuffer);
		});
	}
}