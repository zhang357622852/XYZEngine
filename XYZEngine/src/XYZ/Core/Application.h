#pragma once
#include "Window.h"
#include "LayerStack.h"
#include "ThreadPool.h"

#include "XYZ/ImGui/ImGuiLayer.h"

namespace XYZ {

	struct ApplicationSpecification
	{
		bool EnableImGui = false;
	};
	
	class Application
	{
	public:
		Application(const ApplicationSpecification& specification);
		virtual ~Application();

		void Run();
		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlayer);
		void PopLayer(Layer* layer);
		void Stop();


		bool OnEvent(Event& event);

		Window&			   GetWindow() const		 { return *m_Window; }
		ThreadPool&		   GetThreadPool()			 { return m_ThreadPool; }
		ImGuiLayer*		   GetImGuiLayer()	const	 { return m_ImGuiLayer; }
		const std::string& GetApplicationDir() const { return m_ApplicationDir; }
		const ApplicationSpecification& GetSpecification() const { return m_Specification;}
		inline static Application& Get() { return *s_Application; }

		static Application* CreateApplication();

	private:
		bool onWindowResized(WindowResizeEvent& event);
		bool onWindowClosed(WindowCloseEvent& event);
		void updateTimestep();
		void onImGuiRender();

	private:
		LayerStack m_LayerStack;
		ImGuiLayer* m_ImGuiLayer;
		std::unique_ptr<Window> m_Window;


		bool       				 m_Running;
		bool	   				 m_Minimized;
		float      				 m_LastFrameTime;
		Timestep   				 m_Timestep;
		ThreadPool 				 m_ThreadPool;
		ApplicationSpecification m_Specification;
		std::string				 m_ApplicationDir;

		static Application* s_Application;
	};

}