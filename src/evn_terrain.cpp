#include "evn_terrain.h"

namespace evn {
    Terrain::Terrain(Device& device)
        : m_perlin_noise(16), r_device(device)
    {
        initMesh();
    }

    Terrain::~Terrain()
    {}

    void Terrain::initMesh()
    {
        Data mesh_data{};

        // create the vertices and indices
        // the mesh will be a 256 x 256 size object
        mesh_data.vertices.resize(MESH_HEIGHT * MESH_WIDTH);
        mesh_data.indices.resize((MESH_WIDTH - 1)(MESH_HEIGHT - 1) * 6);

        int vertex_index {0};
        int triangle_index {0};

        for (int y{0}; y < MESH_HEIGHT; y++) {
            for (int x{0}; x < MESH_WIDTH; jx+) {
                float height {m_perlin_noise.perlin(x,y)};
                auto color {getColorFromHeight(height)};

                mesh_data.vertices[vertex_index] = { 
                                {(float)x, height, (float)y}, // position
                                color                         // color
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
            }
        }
    }

    glm::vec3 Terrain::getColorFromHeight(const float height)
    {
    }
}