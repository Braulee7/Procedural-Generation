#include "evn_terrain.h"

namespace evn {
    Terrain::Terrain(Device& device, int x_offset, int y_offset)
        : m_perlin_noise(16), r_device(device), m_xoffset(x_offset),
          m_yoffset(y_offset)
    {
        initMesh();
    }

    Terrain::Terrain(const Terrain& other)
        : m_perlin_noise(other.m_perlin_noise), r_device(other.r_device),
          m_xoffset(other.m_xoffset), m_yoffset(other.m_yoffset)
    {
        initMesh();
    }

    Terrain::~Terrain()
    {}

    void Terrain::update(VkCommandBuffer & command_buffer)
    {
        m_mesh->bind(command_buffer);
        m_mesh->draw(command_buffer);
    }

    void Terrain::initMesh()
    {
        Data mesh_data{};

        // create the vertices and indices
        // the mesh will be a 256 x 256 size object
        mesh_data.vertices.resize((size_t)MESH_HEIGHT * MESH_WIDTH);
        mesh_data.indices.resize((size_t)((MESH_WIDTH - 1) * (MESH_HEIGHT - 1) * 6));
        
        int vertex_index {0};
        int triangle_index {0};

        for (int y{0}; y < MESH_HEIGHT; y++) {
            for (int x{0}; x < MESH_WIDTH; x++) {
                float new_x{ (float)(x + m_xoffset) };
                float new_y{ (float)(y + m_yoffset) };
                float height {(m_perlin_noise.octavePerlin(ABS(new_x),ABS(new_y), 6) )};
                auto color {getColorFromHeight(height)};
                mesh_data.vertices[vertex_index] = { 
                                {new_x, height, new_y}, // position
                                color,                            // color
                                {0, 0, 0}                         // temp normal
                                };

                // give the indices for the triangle
                if (x < MESH_WIDTH - 1 && y < MESH_HEIGHT - 1) {
                    // add two triangles for the square
                    mesh_data.indices[triangle_index] = vertex_index;
                    mesh_data.indices[triangle_index + 1] = vertex_index + MESH_WIDTH + 1;
                    mesh_data.indices[triangle_index + 2] = vertex_index + MESH_WIDTH;
                    triangle_index += 3;

                    mesh_data.indices[triangle_index] = vertex_index + MESH_WIDTH + 1;
                    mesh_data.indices[triangle_index + 1] = vertex_index;
                    mesh_data.indices[triangle_index + 2] = vertex_index + 1;
                    triangle_index += 3;
                }

                vertex_index++;
            }
        }

        calculateNormals(mesh_data);

        m_mesh = std::make_unique<Mesh>(r_device, mesh_data);
    }

    void  Terrain::calculateNormals(Data& mesh_data)
    {
        int triangle_count {(int)(mesh_data.indices.size() / 3)};

        for (int i {0}; i < triangle_count; i++)
        {
            int normal_triangle_index {i * 3};
            uint32_t vertex_a {mesh_data.indices[normal_triangle_index]};
            uint32_t vertex_b {mesh_data.indices[normal_triangle_index + 1]};
            uint32_t vertex_c {mesh_data.indices[normal_triangle_index + 2]};

            glm::vec3 triangle_normal {surfaceNormalFromIndices(vertex_a, vertex_b, vertex_c, mesh_data)};
            mesh_data.vertices[vertex_a].normal += triangle_normal;
            mesh_data.vertices[vertex_b].normal += triangle_normal;
            mesh_data.vertices[vertex_c].normal += triangle_normal;
        }

        for (int i {0}; i < mesh_data.vertices.size(); i++)
            mesh_data.vertices[i].normal = glm::normalize(mesh_data.vertices[i].normal);
    }

    glm::vec3 Terrain::surfaceNormalFromIndices(uint32_t a, uint32_t b, uint32_t c, Data& mesh_data)
    {
        auto point_a {mesh_data.vertices[a].pos};
        auto point_b {mesh_data.vertices[b].pos};
        auto point_c {mesh_data.vertices[c].pos};

        glm::vec3 side_ab {point_a - point_b};
        glm::vec3 side_ac {point_a - point_c};
        auto normal = glm::normalize(glm::cross(side_ab, side_ac));
        return normal;
    }

    glm::vec3 Terrain::getColorFromHeight(float& height)
    {
        int color = (int)(((height + 1.0f) * 0.5f) * 255);
        if (color < 90) {
            height = -0.1;
            return { 0., 0.0, 1. };
        } 
        height =  (height * 20 < 0) ? 0 : height * 20;
        if (color < 150) return { 0., 1.0, 0.0 };
        return { 0.5, 0.5, 0.5 };
    }
}