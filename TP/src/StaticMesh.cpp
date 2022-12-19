#include "StaticMesh.h"

#include <glad/glad.h>
#include <glm/gtx/norm.hpp>

namespace OM3D
{
    

    StaticMesh::StaticMesh(const MeshData &data)
        : _vertex_buffer(data.vertices)
        , _index_buffer(data.indices)
    {
        // Ritter's bounding sphere
        const OM3D::Vertex x = data.vertices[0];
        OM3D::Vertex y = data.vertices[1];
        
        float max_dist = glm::length2(x.position - y.position);

        for (const Vertex& v : data.vertices)
        {
            float dst = glm::length2(v.position - x.position);
            if (dst > max_dist) {
                max_dist = dst;
                y = v;
            }
        }

        OM3D::Vertex z = x;
        for (const Vertex& v : data.vertices)
        {
            float dst = glm::length2(v.position - z.position);
            if (dst > max_dist) {
                max_dist = dst;
                z = v;
            }
        }

        this->_center = (z.position - y.position) / 2.0f;
        this->_radius = max_dist / 2.0f;
    }

    void StaticMesh::setup() const {
        _vertex_buffer.bind(BufferUsage::Attribute);
        _index_buffer.bind(BufferUsage::Index);

        // Vertex position
        glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(Vertex), nullptr);
        // Vertex normal
        glVertexAttribPointer(1, 3, GL_FLOAT, false, sizeof(Vertex),
            reinterpret_cast<void*>(3 * sizeof(float)));
        // Vertex uv
        glVertexAttribPointer(2, 2, GL_FLOAT, false, sizeof(Vertex),
            reinterpret_cast<void*>(6 * sizeof(float)));
        // Tangent / bitangent sign
        glVertexAttribPointer(3, 4, GL_FLOAT, false, sizeof(Vertex),
            reinterpret_cast<void*>(8 * sizeof(float)));
        // Vertex color
        glVertexAttribPointer(4, 3, GL_FLOAT, false, sizeof(Vertex),
            reinterpret_cast<void*>(12 * sizeof(float)));

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glEnableVertexAttribArray(3);
        glEnableVertexAttribArray(4);
    }

    void StaticMesh::draw_instanced(size_t instances) const {
        setup();
        glDrawElementsInstanced(GL_TRIANGLES, int(_index_buffer.element_count()), GL_UNSIGNED_INT, 0, (GLsizei)instances);
    }

    void StaticMesh::draw() const
    {
        setup();
        glDrawElements(GL_TRIANGLES, int(_index_buffer.element_count()), GL_UNSIGNED_INT, nullptr);
    }

} // namespace OM3D
