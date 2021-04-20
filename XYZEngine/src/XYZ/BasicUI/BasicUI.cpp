#include "stdafx.h"
#include "BasicUI.h"
#include "BasicUIInput.h"
#include "BasicUIHelper.h"

#include "XYZ/Renderer/Renderer2D.h"
#include "XYZ/Renderer/Renderer.h"
#include "XYZ/Core/Application.h"
#include "XYZ/Core/Input.h"

#include <glm/gtx/transform.hpp>

namespace XYZ {

	static bUIContext* s_Context = nullptr;

	void bUI::Init()
	{
		s_Context = new bUIContext();
		auto& app = Application::Get();
		s_Context->ViewportSize = glm::vec2(app.GetWindow().GetWidth(), 
											app.GetWindow().GetHeight());
	}
	void bUI::Shutdown()
	{
		delete s_Context;
	}
	void bUI::Update()
	{
		s_Context->Renderer.Begin();
		s_Context->Data.Update();
		s_Context->Data.BuildMesh(s_Context->Renderer);

		glm::mat4 viewMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f));
		viewMatrix = glm::inverse(viewMatrix);
		Renderer2D::BeginScene(glm::ortho(0.0f, s_Context->ViewportSize.x, s_Context->ViewportSize.y, 0.0f) * viewMatrix);
		Renderer2D::SetMaterial(s_Context->Config.m_Material);
		for (const bUIQuad& quad : s_Context->Renderer.GetMesh().Quads)
			Renderer2D::SubmitQuadNotCentered(quad.Position, quad.Size, quad.TexCoord, quad.TextureID, quad.Color);

		Renderer2D::Flush();
		Renderer2D::EndScene();
		Renderer::WaitAndRender();
	}
	void bUI::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<WindowResizeEvent>(&onWindowResize);
		dispatcher.Dispatch<MouseButtonPressEvent>(&onMouseButtonPress);
		dispatcher.Dispatch<MouseButtonReleaseEvent>(&onMouseButtonRelease);
		dispatcher.Dispatch<MouseMovedEvent>(&onMouseMove);
	}
	void bUI::SetupLayout(const std::string& uiName, const std::string& name, const bUILayout& layout)
	{
		bUIAllocator& allocator = s_Context->Data.GetAllocator(uiName);
		bUIElement* element = allocator.GetElement<bUIElement>(name);
		bUIHelper::ResolvePosition(element->ID, allocator.m_Tree, layout);
	}
	
	bUIConfig& bUI::GetConfig()
	{
		return s_Context->Config;
	}
	const bUIContext& bUI::GetContext()
	{
		return *s_Context;
	}
	bool bUI::onWindowResize(WindowResizeEvent& event)
	{
		glm::vec2 newViewportSize((float)event.GetWidth(), (float)event.GetHeight());
		s_Context->ViewportSize = newViewportSize;
		return false;
	}
	bool bUI::onMouseButtonPress(MouseButtonPressEvent& event)
	{
		return bUIInput::OnMouseButtonPress(event, s_Context->EditData, s_Context->Data);
	}
	bool bUI::onMouseButtonRelease(MouseButtonReleaseEvent& event)
	{
		return bUIInput::OnMouseButtonRelease(event, s_Context->EditData, s_Context->Data);
	}
	bool bUI::onMouseMove(MouseMovedEvent& event)
	{
		return bUIInput::OnMouseMove(event, s_Context->EditData, s_Context->Data);
	}
	bUIContext& bUI::getContext()
	{
		return *s_Context;
	}
}