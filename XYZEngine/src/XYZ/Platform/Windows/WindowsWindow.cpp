#include "stdafx.h"
#include "WindowsWindow.h"

#include "XYZ/Renderer/Renderer.h"
#include "XYZ/Core/Application.h"

#include <stb_image.h>


#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

namespace XYZ {
	static bool GLFWInitialized = false;

	std::unique_ptr<Window> Window::Create(const Ref<APIContext>& context, const WindowProperties& props)
	{
		return std::make_unique<WindowsWindow>(props, context);
	}
	

	WindowsWindow::WindowsWindow(const WindowProperties& props, const Ref<APIContext>& context)
	{
		if (!GLFWInitialized)
		{
			const int success = glfwInit();

			XYZ_ASSERT(success, "Could not initialize GLFW!");
			GLFWInitialized = true;
		}

		if (RendererAPI::GetType() == RendererAPI::Type::Vulkan)
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);


		m_Data.This = this;
		GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);


		m_Data.Width = props.Width;
		m_Data.Height = props.Height;
		
		if (props.Flags & WindowFlags::MAXIMIZED)
		{
			glfwWindowHint(GLFW_MAXIMIZED, true);
			m_Window = glfwCreateWindow(mode->width, mode->width, props.Title.c_str(), NULL, NULL);	
			m_Data.Width = mode->width;
			m_Data.Height = mode->height;
		}
		else if (props.Flags & WindowFlags::FULLSCREEN)
		{
			m_Window = glfwCreateWindow(mode->width, mode->width, props.Title.c_str(), glfwGetPrimaryMonitor(), NULL);
			m_Data.Width = mode->width;
			m_Data.Height = mode->height;
		}
		else
		{
			m_Window = glfwCreateWindow(props.Width, props.Height, props.Title.c_str(), NULL, NULL);
		}
		m_Context = context;
		m_Context->Init(m_Window);
		m_Context->CreateSwapChain(&m_Data.Width, &m_Data.Height, m_Data.VSync);
		glfwSetWindowUserPointer(m_Window, &m_Data);
		
		
		
		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			data.Width = width;
			data.Height = height;
			WindowResizeEvent e(width, height);
			data.This->Execute(e);
		});

		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
		{
			const WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			WindowCloseEvent e;
			data.This->Execute(e);
		});

		glfwSetCharCallback(m_Window, [](GLFWwindow* window, unsigned int key)
		{
			const WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			KeyTypedEvent e(key);
			data.This->Execute(e);
		});

		glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			const WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			switch (action)
			{
			case GLFW_PRESS:
			{
				KeyPressedEvent e(key, mods);
				data.This->Execute(e);
				break;
			}
			case GLFW_RELEASE:
			{
				KeyReleasedEvent e(key);
				data.This->Execute(e);
				break;
			}

			}
		});

		glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods)
		{
			const WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			switch (action)
			{
			case GLFW_PRESS:
			{
				MouseButtonPressEvent e(button);
				data.This->Execute(e);
				break;
			}
			case GLFW_RELEASE:
			{
				MouseButtonReleaseEvent e(button);
				data.This->Execute(e);
				break;
			}

			}
		});

		glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset)
		{
			const WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			MouseScrollEvent e((float)xOffset, (float)yOffset);
			data.This->Execute(e);
		});

		glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos)
		{
			const WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			MouseMovedEvent e((int)xPos, (int)yPos);
			data.This->Execute(e);
		});


		m_Cursors[XYZ_ARROW_CURSOR] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
		m_Cursors[XYZ_IBEAM_CURSOR] = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
		m_Cursors[XYZ_CROSSHAIR_CURSOR] = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
		m_Cursors[XYZ_HAND_CURSOR] = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
		m_Cursors[XYZ_HRESIZE_CURSOR] = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
		m_Cursors[XYZ_VRESIZE_CURSOR] = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);

		{
			int width, height;
			glfwGetWindowSize(m_Window, &width, &height);
			m_Data.Width = width;
			m_Data.Height = height;
		}

		// Set icon
		{
			GLFWimage icon;
			int channels;
			icon.pixels = stbi_load("Resources/Editor/XYZLogoSquare.png", &icon.width, &icon.height, &channels, 4);
			glfwSetWindowIcon(m_Window, 1, &icon);
			stbi_image_free(icon.pixels);
		}
	}

	WindowsWindow::~WindowsWindow()
	{
		destroy();
	}

	void WindowsWindow::BeginFrame()
	{		
		m_Context->BeginFrame();
	}

	void WindowsWindow::SwapBuffers()
	{
		m_Context->SwapBuffers();
	}

	void WindowsWindow::ProcessEvents()
	{
		glfwPollEvents();
	}

	void WindowsWindow::SetVSync(bool enable)
	{
		m_Data.VSync = enable;
		#ifdef RENDER_THREAD_ENABLED
		Renderer::GetPool().PushJob([enable]() {
			glfwSwapInterval(enable);
		});
		#else
		glfwSwapInterval(enable);
		#endif
	}

	bool WindowsWindow::IsClosed() const
	{
		return glfwWindowShouldClose(m_Window);
	}

	bool WindowsWindow::IsVSync() const
	{
		return m_Data.VSync;
	}

	void WindowsWindow::SetStandardCursor(uint8_t cursor)
	{
		XYZ_ASSERT(cursor < NUM_CURSORS, "Invalid cursor");
		if (cursor != m_CurrentCursor)
		{
			m_CurrentCursor = cursor;
			glfwSetCursor(m_Window, m_Cursors[cursor]);
		}
	}

	void WindowsWindow::SetCustomCursor(void* cursor)
	{
		m_CurrentCursor = NUM_CURSORS; // Set current cursor to invalid
		GLFWcursor* cur = static_cast<GLFWcursor*>(cursor);
		glfwSetCursor(m_Window, cur);
	}

	void* WindowsWindow::CreateCustomCursor(uint8_t* pixels, uint32_t width, uint32_t height, int32_t xOffset, int32_t yOffset)
	{
		GLFWimage image;
		image.pixels = pixels;
		image.width = width;
		image.height = height;
		return glfwCreateCursor(&image, xOffset, yOffset);
	}

	void* WindowsWindow::GetWindow() const
	{
		return m_Window;
	}

	void* WindowsWindow::GetNativeWindow() const
	{
		return glfwGetWin32Window(m_Window);
	}


	void WindowsWindow::destroy() const
	{
		glfwDestroyWindow(m_Window);
		glfwTerminate();
	}
}