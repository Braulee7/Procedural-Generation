#include "evn_endless_terrain.h"

namespace evn {
    EndlessTerrain::EndlessTerrain(Device& device, Camera& camera)
        : r_device(device), r_camera(camera), 
        m_chunk_size(Terrain::MESH_HEIGHT - 1), 
        m_no_visible_chunks((int)(m_render_dist / m_chunk_size))
    {
    }

    void EndlessTerrain::update(VkCommandBuffer& command_buffer)
    {
        glm::vec2 viewer_pos {r_camera.m_pos.x, r_camera.m_pos.z};
        updateVisibleChunks(viewer_pos);
        // render visible chunks
        for (auto& chunk : m_visible_chunks) 
            chunk->update(command_buffer);
    }

    void EndlessTerrain::updateVisibleChunks(glm::vec2 viewer_pos)
    {
        // clear visible chunks
        m_visible_chunks.clear();

        int curr_x{ (int)(viewer_pos.x / m_chunk_size) };
        int curr_y{ (int)(viewer_pos.y / m_chunk_size) };

        for (int y_offset = -m_no_visible_chunks; y_offset <= m_no_visible_chunks; y_offset++) {
            for (int x_offset = -m_no_visible_chunks; x_offset <= m_no_visible_chunks; x_offset++) {
                glm::vec2 viewed_chunk_coord {curr_x + x_offset, curr_y + y_offset};
                
                if (m_chunks.find(viewed_chunk_coord) != m_chunks.end()) {
                    m_visible_chunks.insert(m_chunks[viewed_chunk_coord]);
                } else {
                    m_chunks[viewed_chunk_coord] = std::make_shared<Terrain>(r_device,
                    viewed_chunk_coord.x * (m_chunk_size), viewed_chunk_coord.y * (m_chunk_size));

                    m_visible_chunks.insert(m_chunks[viewed_chunk_coord]);
                }
            }
        }
    }
    
    bool CompareVec2::operator()(const glm::vec2& op1, const glm::vec2& op2) const
    {
        if (op1.x < op2.x) return true;
        if (op1.x > op2.x) return false;
        return (op1.y < op2.y);
    }

}



