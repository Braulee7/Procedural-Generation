#pragma once

#include <memory>
#include "util/perlin_noise.h"
#include "evn_mesh.h"
namespace evn {
    class Terrain {
    public:
        Terrain(Device& device, int x_offset, int y_offset);
        ~Terrain();
        void update(VkCommandBuffer& command_buffer);
    public:
        const static int MESH_WIDTH = 241;
        const static int MESH_HEIGHT = 241;
    private:
        void initMesh();
        void calculateNormals(Data& mesh_data);
        glm::vec3 surfaceNormalFromIndices(uint32_t a, uint32_t b, uint32_t c, Data& mesh_data);
        glm::vec3 getColorFromHeight(float& height);
    private:
        evn_util::PerlinNoise m_perlin_noise;

        // mesh variables
        Device &r_device;
        std::unique_ptr<Mesh> m_mesh;
        int m_xoffset;
        int m_yoffset;
    };
}