#pragma once

#include <list>
#include <map>
#include <memory>
#include "evn_terrain.h"
#include "evn_camera.h"
namespace evn {
    // Wrapper class for glm::vec2 to compare the
    // two vectors in use with the std::map.find()
    struct CompareVec2 {
        bool operator()(const glm::vec2&, const glm::vec2&) const;
    };


    class EndlessTerrain {
    public:
        EndlessTerrain(Device& device, Camera& camera);
        void update(VkCommandBuffer& command_buffer);
    private:
        void updateVisibleChunks(glm::vec2 viewer_pos);
    private:
        Device& r_device;
        Camera& r_camera;
        std::list<std::shared_ptr<Terrain>> m_visible_chunks;
        std::map<glm::vec2, std::shared_ptr<Terrain>, CompareVec2> m_chunks;
        const float m_render_dist = 450;
        int m_chunk_size;
        int m_no_visible_chunks;
    };
}