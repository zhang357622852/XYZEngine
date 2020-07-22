#pragma once
#include "XYZ/Renderer/APIContext.h"
#include "XYZ/Core/Window.h"
#include "XYZ/Event/EventHandler.h"
#include "XYZ/Event/Event.h"
#include "XYZ/Event/EventSystem.h"

#include <GLFW/glfw3.h>


namespace XYZ {
/**	@class WindowsWindow
 *	Enables window management on Windows
 *
 */
	class WindowsWindow : public Window
	{
	public:
		WindowsWindow(const WindowProperties& props);
		virtual ~WindowsWindow();

		virtual void Update() override;
		virtual void SetVSync(bool enabled) override;
		virtual bool IsClosed() override;
		virtual void SetCursor(WindowCursor cursor) override;
		

		inline void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }
		inline virtual void* GetNativeWindow() const { return m_Window; }

		inline virtual uint32_t GetWidth() const override { return m_Data.Width; }
		inline virtual uint32_t GetHeight() const override { return m_Data.Height; }

	private:
		void Destroy();

		GLFWwindow* m_Window;
		GLFWcursor* m_Cursor = nullptr;
		WindowCursor m_Current = WindowCursor::XYZ_ARROW_CURSOR;

		std::unique_ptr<APIContext> m_Context;

		struct WindowData
		{
			std::string Title;
			uint32_t Width = 0;
			uint32_t Height = 0;
			bool VSync = false;

			EventCallbackFn EventCallback;
			WindowsWindow* This = nullptr;
		};

		WindowData m_Data;
		
	};
}