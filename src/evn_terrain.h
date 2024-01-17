#include <memory>
#include "util/perlin_noise.h"
#include "evn_mesh.h"
namespace evn {
    class Terrain {
    public:
        Terrain(Device& device);
        ~Terrain();

        void update(VkCommandBuffer& command_buffer);
    private:
        void initMesh();
        glm::vec3 getColorFromHeight(const float height);
    private:
        evn_util::PerlinNoise m_perlin_noise;

        // mesh variables
        Device &r_device;
        std::unique_ptr<Mesh> mesh;
        const uint16_t MESH_WIDTH = 256;
        const uint16_t MESH_HEIGHT = 256;
    };
}