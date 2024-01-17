/*
#include "evn_app.h"

int main(void)
{
	evn::App app("evn Engine");
	app.run();
	return 0;
}

*/
#include <time.h>
#include <iostream>
#include "util/perlin_noise.h"

int main(void)
{
	evn_util::PerlinNoise perlin_noise(16, time(0));

	std::cout << "P3\n" << 256 << ' ' << 256 << "\n255\n";

	

	for (int i = 0; i < 256; i++) {
		for (int j = 0; j < 256; j++) {
			float x { (float)j };
			float y { (float)i };
			float val{ 0.0f };
			float freq{ 1 };
			float amp { 1 };
			for (int k = 0; k < 8; k++) {
				val += perlin_noise.perlin(x * freq / 16.0f, y * freq / 16.0f) * amp;
				freq *= 2;
				amp *= 0.5;
			}
			
			int color = (int)(((val + 1.0f) * 0.5f) * 255);
			std::cout << color << ' ' << color << ' ' << color << "\n";
		}
	}

	return 0;
}