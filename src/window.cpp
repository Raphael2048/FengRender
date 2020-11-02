#include <algorithm>
#include "window.hpp"
#include "windowsx.h"
#include "util/defines.hpp"

namespace feng
{


	Window::Window(HINSTANCE instance, int show_cmd, std::string const & name, std::uint32_t width, std::uint32_t height)
		: m_title(name), m_instance(instance)
	{
		WNDCLASSEX wc;
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = &Window::WindowProc;
		wc.cbClsExtra = NULL;
		wc.cbWndExtra = NULL;
		wc.hInstance = instance;
		wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
		wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)(COLOR_WINDOW);
		wc.lpszMenuName = nullptr;
		wc.lpszClassName = name.c_str();
		wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);

		m_window_width = width;
		m_window_height = height;

		if (!RegisterClassEx(&wc))
		{
			FMSG("Failed to register extended window class: ");
		}

		auto window_style = WS_OVERLAPPEDWINDOW;

		if (/*!allow_resizing*/ true)
		{
			window_style &= ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
		}

		RECT client_rect;
		client_rect.left = 0;
		client_rect.right = width;
		client_rect.top = 0;
		client_rect.bottom = height;
		AdjustWindowRectEx(&client_rect, window_style, FALSE, wc.style);

		m_handle = CreateWindowEx(
			wc.style,
			name.c_str(),
			name.c_str(),
			window_style,
			CW_USEDEFAULT, CW_USEDEFAULT,
			client_rect.right - client_rect.left, client_rect.bottom - client_rect.top,
			nullptr,
			nullptr,
			instance,
			nullptr
		);

		if (!m_handle)
		{
			FMSG("Failed to create window." + GetLastError())
		}

		SetWindowLongPtr(m_handle, GWLP_USERDATA, (LONG_PTR)this);

		ShowWindow(m_handle, show_cmd);
		UpdateWindow(m_handle);

		m_running = true;
	}

	Window::Window(HINSTANCE instance, std::string const& name, std::uint32_t width, std::uint32_t height, bool show)
		: Window(instance, show ? SW_SHOWNORMAL : SW_HIDE, name, width, height)
	{

	}

	Window::~Window()
	{
		Stop();
		UnregisterClassA(m_title.c_str(), m_instance);
	}

	void Window::PollEvents()
	{
		MSG msg;
		if (PeekMessage(&msg, m_handle, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				m_running = false;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	void Window::Show()
	{
		ShowWindow(m_handle, SW_SHOW);
	}

	void Window::Stop()
	{
		DestroyWindow(m_handle);
	}

	void Window::SetRenderLoop(std::function<void()> render_func)
	{
		m_render_func = std::move(render_func);
	}

	void Window::StartRenderLoop()
	{
		while (IsRunning())
		{
			PollEvents();
		}

		UnregisterClassA(m_title.c_str(), m_instance);
	}

	void Window::SetKeyCallback(KeyCallback callback)
	{
		m_key_callback = std::move(callback);
	}

	void Window::SetMouseCallback(MouseCallback callback)
	{
		m_mouse_callback = std::move(callback);
	}

	void Window::SetMouseWheelCallback(MouseWheelCallback callback)
	{
		m_mouse_wheel_callback = std::move(callback);
	}

	void Window::SetMouseMoveCallback(MouseMoveCallback callback)
	{
		m_mouse_move_callback = std::move(callback);
	}

	void Window::SetResizeCallback(ResizeCallback callback)
	{
		m_resize_callback = std::move(callback);
	}

	bool Window::IsRunning() const
	{
		return m_running;
	}

	std::int32_t Window::GetWidth() const
	{
		RECT r;
		GetClientRect(m_handle, &r);
		return static_cast<std::int32_t>(r.right - r.left);
	}

	std::int32_t Window::GetHeight() const
	{
		RECT r;
		GetClientRect(m_handle, &r);
		return static_cast<std::int32_t>(r.bottom - r.top);
	}

	std::string Window::GetTitle() const
	{
		return m_title;
	}

	HWND Window::GetWindowHandle() const
	{
		return m_handle;
	}

	bool Window::IsFullscreen() const
	{
		RECT a, b;
		GetWindowRect(m_handle, &a);
		GetWindowRect(GetDesktopWindow(), &b);
		return (a.left == b.left  &&
			a.top == b.top   &&
			a.right == b.right &&
			a.bottom == b.bottom);
	}

	LRESULT CALLBACK Window::WindowProc(HWND handle, UINT msg, WPARAM w_param, LPARAM l_param)
	{
		// extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
		// if (ImGui_ImplWin32_WndProcHandler(handle, msg, w_param, l_param))
			// return true;

		auto window = (Window*)GetWindowLongPtr(handle, GWLP_USERDATA);
		if (window) return window->WindowProc_Impl(handle, msg, w_param, l_param);

		return DefWindowProc(handle, msg, w_param, l_param);
	}

	LRESULT CALLBACK Window::WindowProc_Impl(HWND handle, UINT msg, WPARAM w_param, LPARAM l_param)
	{
		switch (msg)
		{
		case WM_PAINT:
			if (m_render_func)
			{
				m_render_func();
			}
			return 0;
		case WM_DESTROY:
			m_running = false;
			return 0;
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
			if (m_mouse_callback)
			{
				m_mouse_callback((int)w_param, msg, (int)l_param);
			}
			return 0;
		case WM_MOUSEMOVE:
			if (m_mouse_move_callback)
			{
				m_mouse_move_callback(w_param, GET_X_LPARAM(l_param), GET_Y_LPARAM(l_param));
			}
			return 0;
		case WM_KEYDOWN:
		case WM_KEYUP:
			if (m_key_callback)
			{
				m_key_callback((int)w_param, msg, (int)l_param);
			}

			return 0;

		case WM_MOUSEWHEEL:
			if (m_mouse_wheel_callback)
			{
				m_mouse_wheel_callback(GET_WHEEL_DELTA_WPARAM(w_param));
			}

			return 0;

		case WM_SIZE:
			if (w_param != SIZE_MINIMIZED)
			{
				if (RECT rect; m_resize_callback && GetClientRect(handle, &rect))
				{
					int width = rect.right - rect.left;
					int height = rect.bottom - rect.top;

					bool has_changed = width != m_window_width || height != m_window_height;
					bool is_valid_size = width > 16 && height > 16;

					if (has_changed && is_valid_size)
					{
						m_resize_callback(static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height));

						m_window_width = width;
						m_window_height = height;
					}
				}
			}
			return 0;
		}

		return DefWindowProc(handle, msg, w_param, l_param);
	}

} /* wr */
