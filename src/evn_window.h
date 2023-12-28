#pragma once

#define GLFW_INCLUDE_VULKAN
#include<GLFW/glfw3.h>
#include<string>


namespace evn {
	class Window {
	public:
		Window();
		Window(const uint32_t width, const uint32_t height, const std::string& name);
		~Window();

		inline bool shouldClose() const { return glfwWindowShouldClose(p_window); }

	private: // methods
		void init_window();
	private:
		bool m_resized;
		uint32_t m_width;
		uint32_t m_height;
		std::string m_name;
		GLFWwindow* p_window;
	};
}