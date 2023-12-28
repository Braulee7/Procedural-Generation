#include "evn_window.h"

int main(void)
{
	// testing window
	evn::Window window(1280, 720, "evolution engine");
	while (!window.shouldClose()) {
		glfwPollEvents();
	}
	return 0;
}