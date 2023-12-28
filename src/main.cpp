#include "evn_window.h"
#include "evn_device.h"

int main(void)
{
	// testing window
	evn::Window window(1280, 720, "evolution engine");
	evn::Device device(window);

	while (!window.shouldClose()) {
		glfwPollEvents();
	}
	return 0;
}