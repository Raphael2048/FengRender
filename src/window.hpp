/*!
 * Copyright 2019 Breda University of Applied Sciences and Team Wisp (Viktor Zoutman, Emilio Laiso, Jens Hagen, Meine Zeinstra, Tahar Meijs, Koen Buitenhuis, Niels Brunekreef, Darius Bouma, Florian Schut)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

#include <Windows.h>
#include <functional>

namespace feng
{

	class Window
	{
		using KeyCallback = std::function<void(int key, int action, int mods)>;
		using MouseCallback = std::function<void(int key, int action, int mods)>;
		using ResizeCallback = std::function<void(std::uint32_t width, std::uint32_t height)>;
		using MouseWheelCallback = std::function<void(short value)>;
		using MouseMoveCallback = std::function<void(WPARAM btnState, int x, int y)>;
	public:
		/*!
		* @param instance A handle to the current instance of the application.
		* @param name Window title.
		* @param width Initial window width.
		* @param height Initial window height.
		* @param show Controls whether the window will be shown. Default is true.
		*/
		Window(HINSTANCE instance, std::string const& name, std::uint32_t width, std::uint32_t height, bool show = true);
		Window(HINSTANCE instance, int show_cmd, std::string const& name, std::uint32_t width, std::uint32_t height);
		~Window();

		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;
		Window(Window&&) = delete;
		Window& operator=(Window&&) = delete;

		/*! Handles window events. Should be called every frame */
		void PollEvents();
		/*! Shows the window if it was hidden */
		void Show();
		/*! Requests to close the window */
		void Stop();

		/*! Give the window a function to call on repaint */
		void SetRenderLoop(std::function<void()> render_func);
		/*! Start a loop that runs until the window is closed. */
		void StartRenderLoop();

		/*! Used to set the key callback function */
		void SetKeyCallback(KeyCallback callback);
		/*! Used to set the mouse callback function */
		void SetMouseCallback(MouseCallback callback);
		/*! Used to set the mouse wheel callback function */
		void SetMouseWheelCallback(MouseWheelCallback callback);
		
		void SetMouseMoveCallback(MouseMoveCallback callback);
		/*! Used to set the resize callback function */
		void SetResizeCallback(ResizeCallback callback);

		/*! Returns whether the application is running. (used for the main loop) */
		bool IsRunning() const;
		/* Returns the client width */
		std::int32_t GetWidth() const;
		/* Returns the client height */
		std::int32_t GetHeight() const;
		/* Returns the title of the window. */
		std::string GetTitle() const;
		/*! Returns the native window handle (HWND)*/
		HWND GetWindowHandle() const;
		/*! Checks whether the window is fullscreen */
		bool IsFullscreen() const;

	private:
		/*! WindowProc that calls `WindowProc_Impl` */
		static LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
		/*! Main WindowProc function */
		LRESULT CALLBACK WindowProc_Impl(HWND, UINT, WPARAM, LPARAM);

		KeyCallback m_key_callback;
		MouseCallback m_mouse_callback;
		ResizeCallback m_resize_callback;
		MouseWheelCallback m_mouse_wheel_callback;
		MouseMoveCallback m_mouse_move_callback;

		std::function<void()> m_render_func;

		std::string m_title;

		bool m_running;
		HWND m_handle;
		HINSTANCE m_instance;

		std::int32_t m_window_width = 0;
		std::int32_t m_window_height = 0;
	};

} /* wr */
