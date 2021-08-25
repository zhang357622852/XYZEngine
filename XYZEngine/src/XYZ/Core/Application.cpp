#include "stdafx.h"
#include "Application.h"


#include "XYZ/Debug/Timer.h"
#include "XYZ/Renderer/Renderer.h"
#include "XYZ/Asset/AssetManager.h"

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <imgui.h>

namespace XYZ {
	Application* Application::s_Application = nullptr;

	ThreadPool Application::s_ThreadPool(12);

	Application::Application()
	{
		AssetManager::Init();
		Renderer::Init();
		s_Application = this;
		m_Running = true;

		m_Window = Window::Create();
		m_Window->RegisterCallback(Hook(&Application::OnEvent, this));	
		m_Window->SetVSync(0);

		TCHAR NPath[MAX_PATH];
		GetCurrentDirectory(MAX_PATH, NPath);
		std::wstring tmp(&NPath[0]);
		m_ApplicationDir = std::string(tmp.begin(), tmp.end());

		m_ImGuiLayer = new ImGuiLayer();
		m_LayerStack.PushOverlay(m_ImGuiLayer);	
	}

	Application::~Application()
	{
		Renderer::Shutdown();
		AssetManager::Shutdown();
	}

	void Application::Run()
	{
		float performance = 0.0f;
		while (m_Running)
		{		
			float time = (float)glfwGetTime();
			float timestep = time - m_LastFrameTime;
			m_LastFrameTime = time;
			{
				Stopwatch watch;				
				Renderer::WaitAndRender();

				for (Layer* layer : m_LayerStack)	
					layer->OnUpdate(timestep);	

				
				m_ImGuiLayer->Begin();
				if (ImGui::Begin("Stats"))
				{
					ImGui::Text("Performance: %f ms", performance);
				}
				ImGui::End();

				for (Layer* layer : m_LayerStack)
					layer->OnImGuiRender();
		
				m_ImGuiLayer->End();

				performance = watch.Stop();
			}
			m_Window->Update();
		}
	}

	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
	}

	void Application::PushOverlay(Layer* overlayer)
	{
		m_LayerStack.PushOverlay(overlayer);		
	}

	void Application::PopLayer(Layer* layer)
	{
		m_LayerStack.PopLayer(layer);
	}

	void Application::Stop()
	{
		m_Running = false;
	}

	bool Application::onWindowResized(WindowResizeEvent& event)
	{
		Renderer::SetViewPort(0, 0, event.GetWidth(), event.GetHeight());
		return false;
	}

	bool Application::onWindowClosed(WindowCloseEvent& event)
	{
		m_Running = false;
		return false;
	}


	bool Application::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<WindowResizeEvent>(Hook(&Application::onWindowResized, this));
		dispatcher.Dispatch<WindowCloseEvent>(Hook(&Application::onWindowClosed, this));

		for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
		{
			(*it)->OnEvent(event);
			if (event.Handled)
				return true;
		}
		return false;
	}
}