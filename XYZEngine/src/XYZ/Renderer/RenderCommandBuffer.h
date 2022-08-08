#pragma once
#include "XYZ/Core/Ref/Ref.h"
#include "Pipeline.h"

namespace XYZ {

	class SecondaryRenderCommandBuffer;
	class RenderCommandBuffer : public RefCount
	{
	public:
		virtual ~RenderCommandBuffer() {}

		virtual void Begin() = 0;
		virtual void End() = 0;
		
		virtual void RT_Begin(){};
		virtual void RT_End() {}

		virtual void* CommandBufferHandle(uint32_t index) = 0;

		virtual void Submit() = 0;

		virtual void CreateTimestampQueries(uint32_t count) = 0;

		virtual float GetExecutionGPUTime(uint32_t frameIndex, uint32_t queryIndex = 0) const = 0;
		virtual const PipelineStatistics& GetPipelineStatistics(uint32_t frameIndex) const = 0;

		virtual uint32_t BeginTimestampQuery() = 0;
		virtual void EndTimestampQuery(uint32_t queryID) = 0;

		virtual Ref<SecondaryRenderCommandBuffer> CreateSecondaryCommandBuffer() = 0;

		static Ref<RenderCommandBuffer> Create(uint32_t count = 0, const std::string& debugName = "");
	};


	class SecondaryRenderCommandBuffer : public RefCount
	{
	public:
		virtual void Begin(Ref<Framebuffer> frameBuffer) = 0;
		virtual void End() = 0;

		virtual void RT_Begin(Ref<Framebuffer> frameBuffer) {};
		virtual void RT_End() {}

		virtual Ref<RenderCommandBuffer> GetPrimaryCommandBuffer() const = 0;

		virtual void* CommandBufferHandle(uint32_t index) const = 0;
	};

}