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
        void calculateNormals(Data& mesh_data);
        glm::vec3 surfaceNormalFromIndices(uint32_t a, uint32_t b, uint32_t c, Data& mesh_data);
        glm::vec3 getColorFromHeight(float& height);
    private:
        evn_util::PerlinNoise m_perlin_noise;

        // mesh variables
        Device &r_device;
        std::unique_ptr<Mesh> m_mesh;
        const int MESH_WIDTH = 241;
        const int MESH_HEIGHT = 241;
    };
}