#include "stdafx.h"
#include "RenderCommandBuffer.h"

#include "XYZ/Renderer/RendererAPI.h"
#include "XYZ/API/Vulkan/VulkanRenderCommandBuffer.h"

namespace XYZ {


	Ref<PrimaryRenderCommandBuffer> PrimaryRenderCommandBuffer::Create(uint32_t count, const std::string& debugName)
	{
		switch (RendererAPI::GetType())
		{
		case RendererAPI::Type::None:    XYZ_ASSERT(false, "API is not supported") return nullptr;
		case RendererAPI::Type::Vulkan:  return Ref<VulkanPrimaryRenderCommandBuffer>::Create(count, debugName);
		}
		XYZ_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}

}