#include "evn_window.h"
#include "evn_device.h"
#include "evn_swapchain.h"

int main(void)
{
	// testing window
	evn::Window window(1280, 720, "evolution engine");
	evn::Device device(window);
	evn::Swapchain swapchain(device, window.getExtent());

	while (!window.shouldClose()) {
		glfwPollEvents();
	}
	return 0;
}