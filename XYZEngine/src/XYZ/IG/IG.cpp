#include "stdafx.h"
#include "IG.h"
#include "IGSerializer.h"

#include "XYZ/Core/Input.h"
#include "XYZ/Renderer/Renderer2D.h"
#include "XYZ/Renderer/Renderer.h"
#include "XYZ/Event/ApplicationEvent.h"


#include <glm/gtx/transform.hpp>

namespace XYZ {
	static IGContext* s_Context;
	static size_t s_PoolHandle = 0;
	static glm::vec2 s_MousePosition = glm::vec2(0.0f);


	void IG::Init()
	{
		s_Context = new IGContext();
		auto [width, height] = Input::GetWindowSize();
		s_Context->Dockspace.SetRootSize({ width, height });
	}

	void IG::Shutdown()
	{
		IGSerializer::Serialize("IG.ig", *s_Context);
		delete s_Context;
	}

	void IG::LoadLayout(const char* filepath)
	{
		IGSerializer::Deserialize(filepath, *s_Context);
	}

	void IG::BeginFrame(const glm::mat4& viewProjectionMatrix)
	{
		XYZ_ASSERT(s_Context, "IG is not initialized");
		glm::mat4 viewMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f));
		viewMatrix = glm::inverse(viewMatrix);

	
		Renderer2D::BeginScene(viewProjectionMatrix * viewMatrix);	
		Renderer2D::SetMaterial(s_Context->RenderData.DefaultMaterial);

		s_Context->RenderData.RebuildMesh(s_Context->Allocator);
	}

	void IG::EndFrame()
	{
		for (auto& it : s_Context->RenderData.Mesh.Quads)
		{
			Renderer2D::SubmitQuadNotCentered(it.Position, it.Size, it.TexCoord, it.TextureID, it.Color);
		}
		for (auto& it : s_Context->RenderData.Mesh.Lines)
		{
			Renderer2D::SubmitLine(it.P0, it.P1, it.Color);
		}
		s_Context->Dockspace.SubmitToRenderer(&s_Context->RenderData);

		Renderer2D::Flush();
		Renderer2D::FlushLines();

		if (s_Context->RenderData.Scissors.size())
		{
			s_Context->RenderData.ScissorBuffer->Update(s_Context->RenderData.Scissors.data(),s_Context->RenderData. Scissors.size() * sizeof(IGScissor));
			s_Context->RenderData.ScissorBuffer->BindRange(0, s_Context->RenderData.Scissors.size() * sizeof(IGScissor), 0);
			Renderer2D::SetMaterial(s_Context->RenderData.ScissorMaterial);
			for (auto& it : s_Context->RenderData.ScrollableMesh.Quads)
			{
				Renderer2D::SubmitQuadNotCentered(it.Position, it.Size, it.TexCoord, it.TextureID, it.Color, it.ScissorIndex);
			}
			for (auto& it : s_Context->RenderData.ScrollableMesh.Lines)
			{
				Renderer2D::SubmitLine(it.P0, it.P1, it.Color);
			}
		
			Renderer2D::Flush();
			Renderer2D::FlushLines();
		}
		Renderer2D::EndScene();
		Renderer::WaitAndRender();
	}

	void IG::OnEvent(Event& event)
	{
		if (event.GetEventType() == EventType::MouseButtonPressed)
		{
			s_Context->Input.OnMouseButtonPress((MouseButtonPressEvent&)event, *s_Context);
		}
		else if (event.GetEventType() == EventType::MouseButtonReleased)
		{
			s_Context->Input.OnMouseButtonRelease((MouseButtonReleaseEvent&)event, *s_Context);
		}
		else if (event.GetEventType() == EventType::MouseMoved)
		{
			s_Context->Input.OnMouseMove((MouseMovedEvent&)event, *s_Context);
		}
		else if (event.GetEventType() == EventType::MouseScroll)
		{
			s_Context->Input.OnMouseScroll((MouseScrollEvent&)event, *s_Context);
		}
		else if (event.GetEventType() == EventType::KeyTyped)
		{
			s_Context->Input.OnKeyType((KeyTypedEvent&)event, *s_Context);
		}
		else if (event.GetEventType() == EventType::KeyPressed)
		{
			s_Context->Input.OnKeyPress((KeyPressedEvent&)event, *s_Context);
		}
		else if (event.GetEventType() == EventType::WindowResized)
		{
			WindowResizeEvent& e = (WindowResizeEvent&)event;
			s_Context->Dockspace.SetRootSize({(float)e.GetWidth(), (float)e.GetHeight() });
		}

		if (event.Handled)
		{
			RebuildUI();
		}
	}

	void IG::BeginUI(size_t handle)
	{
		s_PoolHandle = handle;
	}

	void IG::EndUI()
	{
	}

	void IG::RebuildUI()
	{
		s_Context->RenderData.Rebuild = true;
		s_Context->RenderData.RebuildTwice = true;		
	}


	std::pair<size_t, size_t> IG::AllocateUI(const std::initializer_list<IGHierarchyElement>& hierarchy)
	{
		auto result = s_Context->Allocator.CreatePool(hierarchy);
		s_Context->RenderData.Rebuild = true;
		s_Context->RenderData.RebuildMesh(s_Context->Allocator);
		return result;
	}

	std::pair<size_t, size_t> IG::AllocateUI(const std::vector<IGHierarchyElement>& hierarchy)
	{
		auto result = s_Context->Allocator.CreatePool(hierarchy);
		s_Context->RenderData.Rebuild = true;
		s_Context->RenderData.RebuildMesh(s_Context->Allocator);
		return result;
	}

	size_t IG::ReallocateUI(size_t handle, const std::vector<IGHierarchyElement>& hierarchy)
	{
		s_Context->Allocator.GetPools()[handle].Rebuild(hierarchy);
		s_Context->RenderData.Rebuild = true;
		s_Context->RenderData.RebuildMesh(s_Context->Allocator);
		return s_Context->Allocator.GetPools()[handle].Size();
	}

	IGContext& IG::GetContext()
	{
		return *s_Context;
	}
}