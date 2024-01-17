#include "perlin_noise.h"

namespace evn_util {
	PerlinNoise::PerlinNoise(uint16_t cell_dimension)
		: m_dimensions(cell_dimension), m_mesh_dimension(256)
	{
		// initCorners();
	}

	PerlinNoise::~PerlinNoise()
	{

	}
	float PerlinNoise::perlin(float x, float y)
	{
		glm::vec2 input {x, y};
		// get the corner indices
		int x0{ (int)(x) };
		int y0{ (int)(y) };
		int x1{ x0 + 1 };
		int y1{ y0 + 1 };

		// get distance to first corner
		float sx{ x - (float)x0 };
		float sy{ y - (float)y0 };

		// get the top corners
		// top left
		float d0{ dotGradient(x0, y0, x, y) };
		// top right
		float d1{ dotGradient(x1, y0, x, y) };
		// interpolate
		float u{ interp(d0, d1, sx) };

		// bottom corners
		float d2{ dotGradient(x0, y1, x, y) };
		float d3{ dotGradient(x1, y1, x, y) };
		// interpolate
		float v{ interp(d2, d3, sx) };
		

		// return the interpolated value in the y direction
		return interp(u, v, sy);
	}
	float PerlinNoise::octavePerlin(float x, float y, int octaves, float persistence)
	{
		float val{ 0.0f };
		float freq{ 1 };
		float amp { 1 };

		for (int i = 0; i < octaves; i++) {
			val += perlin(x * freq / m_dimensions, y * freq / m_dimensions) * amp;
			freq *= 2;
			amp *= persistence;
		}

		return val;
	}
	void PerlinNoise::initCorners()
	{
		// resize the matrice to the dimensions
		int dim{ m_dimensions + 1 };

		m_corner_matrice.resize(dim);
		
		for (int i{ 0 }; i < dim; i++) 
			m_corner_matrice[i].resize(dim);

		for (int y{ 0 }; y < dim; y++) {
			for (int x{ 0 }; x < dim; x++) {
				// choose a random gradients to set to this corner
				m_corner_matrice[y][x] = randomGradient(x * m_dimensions, y * m_dimensions);
			}
		}
		
	}
	float PerlinNoise::ease(float a, float b, float c) const
	{
		return  (float)((a - b) * (3.0 - c * 2.0) * c * c + a);
	}

	glm::vec2 PerlinNoise::randomGradient(int ix, int iy) {
		// No precomputed gradients mean this works for any number of grid coordinates
		const unsigned w = 8 * sizeof(unsigned);
		const unsigned s = w / 2;
		unsigned a = ix, b = iy;
		a *= 3284157443;

		b ^= a << s | a >> w - s;
		b *= 1911520717;

		a ^= b << s | b >> w - s;
		a *= 2048419325;
		float random = a * (3.14159265 / ~(~0u >> 1)); // in [0, 2*Pi]

		// Create the vector from the angle
		glm::vec2 v;
		v.x = sin(random);
		v.y = cos(random);

		return v;
	}
	float PerlinNoise::dotGradient(int x0, int y0, float x, float y)
	{
		// get the random gradient vector
		glm::vec2 gradient {randomGradient(x0, y0)};
		float dx = x - (float)x0;
		float dy = y - (float)y0;
		return dx * gradient.x + dy * gradient.y;
	}
}