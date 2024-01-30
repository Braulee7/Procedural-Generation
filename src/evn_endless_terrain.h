#pragma once

#include <list>
#include <map>
#include <memory>
#include "evn_terrain.h"
#include "evn_camera.h"
namespace evn {
    class EndlessTerrain {
    public:
        EndlessTerrain(Device& device, Camera& camera);
        void update(VkCommandBuffer& command_buffer);
    private:
        void updateVisibleChunks(glm::vec2 viewer_pos);
    private:
        Device& r_device;
        Camera& r_camera;
        std::list<Terrain> m_visible_chunks;
        std::map<glm::vec2, std::shared_ptr<Terrain>> m_chunks;
        const float m_render_dist = 450;
        int m_chunk_size;
        int m_no_visible_chunks;
    };
}