#pragma once
#include <stdint.h>
#include <vector>
#include <glm/glm.hpp>
#include <random>

namespace evn_util {
	class PerlinNoise {
	public:
		PerlinNoise(uint16_t cell_dimensions);
		~PerlinNoise();
		float perlin(float x, float y);
		float octavePerlin(float x, float y, int octaves, float persistence=0.5);
		static inline float linear(float start, float end, float coef) { return coef * (end - start) + start; }
		static inline float poly(float coef) { return 3 * coef * coef - 2 * coef * coef * coef; }
		static inline float interp(float start, float end, float coef) { return linear(start, end, poly(coef)); }
	private: // methods
		void initCorners();
		glm::vec2 randomGradient(int x, int y);
		float dotGradient(int x0, int x1, float x, float y);
		float ease(float a, float b, float c) const ;

		
	private:
		uint16_t m_dimensions;
		uint32_t m_mesh_dimension;
		std::vector<std::vector<glm::vec2>> m_corner_matrice;
	};
}
