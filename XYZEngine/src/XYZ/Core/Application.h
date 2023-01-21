#pragma once
#include "Window.h"
#include "LayerStack.h"
#include "ThreadPool.h"

#include "XYZ/ImGui/ImGuiLayer.h"
#include "XYZ/Debug/Timer.h"
#include "XYZ/Core/Core.h"

namespace XYZ {

	struct ApplicationSpecification
	{
		bool EnableImGui = true;
		bool WindowCreate = true;
	};
	
	class XYZ_API Application
	{
	public:
		Application(const ApplicationSpecification& specification = {});
		virtual ~Application();

		void Run();
		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlayer);

		void PopLayer(Layer* layer);
		void Stop();
		bool OnEvent(Event& event);
		Timestep GetTimestep() const { return m_Timestep; }

		Window&							GetWindow() const		  { return *m_Window; }
		ThreadPool&						GetThreadPool()			  { return m_ThreadPool; }
		ImGuiLayer*						GetImGuiLayer()	const	  { return m_ImGuiLayer; }
		PerformanceProfiler&			GetPerformanceProfiler()  { return m_Profiler; }
		
		const std::string&				GetApplicationDir() const { return m_ApplicationDir; }
		const ApplicationSpecification& GetSpecification() const  { return m_Specification;}
		
		static Application& Get();

		static Application* CreateApplication();

	private:
		bool onWindowResized(WindowResizeEvent& event);
		bool onWindowClosed(WindowCloseEvent& event);
		void updateTimestep();
		void onImGuiRender();

		void displayPerformance();
		void displayRenderer();

		void onStop();

		void onRunWindow();
		void onRunWindowless();

	private:
		LayerStack m_LayerStack;
		ImGuiLayer* m_ImGuiLayer;
		std::unique_ptr<Window> m_Window;

		bool       				 m_Running;
		bool	   				 m_Minimized;
		float      				 m_LastFrameTime;
		Stopwatch				 m_Timer;
		Timestep   				 m_Timestep;
		ThreadPool 				 m_ThreadPool;
		ApplicationSpecification m_Specification;
		PerformanceProfiler		 m_Profiler;
		std::string				 m_ApplicationDir;
	};

}