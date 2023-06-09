#pragma once
#include "XYZ/Renderer/Renderer.h"


namespace XYZ {

	struct CompositePassConfiguration
	{
		Ref<RenderPass> Pass;
		Ref<Shader>		Shader;
	};

	class XYZ_API CompositePass
	{
	public:
		void Init(const CompositePassConfiguration& config, const Ref<RenderCommandBuffer>& commandBuffer);

		void Submit(const Ref<RenderCommandBuffer>& commandBuffer, const Ref<Image2D>& lightImage, const Ref<Image2D>& bloomImage, bool clear);

	private:
		void createPipeline();

	private:
		Ref<RenderPass>		  m_RenderPass;
		Ref<Shader>			  m_Shader;
		Ref<Pipeline>		  m_Pipeline;
		Ref<Material>		  m_Material;
		Ref<MaterialInstance> m_MaterialInstance;
	};

}