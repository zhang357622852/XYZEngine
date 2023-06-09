#pragma once
#include "XYZ/Core/Ref/Ref.h"
#include "Framebuffer.h"

namespace XYZ {

	struct RenderPassSpecification
	{
		Ref<Framebuffer> TargetFramebuffer;
	};

	class XYZ_API RenderPass : public RefCount
	{
	public:
		virtual ~RenderPass() = default;

		virtual RenderPassSpecification& GetSpecification() = 0;
		virtual const RenderPassSpecification& GetSpecification() const = 0;

		static Ref<RenderPass> Create(const RenderPassSpecification & spec);

	};
}