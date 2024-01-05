#include "evn_window.h"

// default window size
const uint32_t WIDTH = 1280;
const uint32_t HEIGHT = 720;

namespace evn {
	Window::Window()
		: m_resized(false), m_width(WIDTH), m_height(HEIGHT), m_name(""), p_window(nullptr)
	{
		initWindow();
	}

	Window::Window(const uint32_t width, const uint32_t height, const std::string& name)
		: m_resized(false), m_width(width), m_height(height), m_name(name), p_window(nullptr)
	{
		initWindow();
	}

	Window::~Window()
	{
		glfwDestroyWindow(p_window);
		glfwTerminate();
	}

	void Window::initWindow()
	{
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		p_window = glfwCreateWindow(m_width, m_height, m_name.c_str(), nullptr, nullptr);
	}
}