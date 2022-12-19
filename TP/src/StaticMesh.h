#ifndef STATICMESH_H
#define STATICMESH_H

#include <TypedBuffer.h>
#include <Vertex.h>
#include <graphics.h>
#include <vector>

namespace OM3D
{

    struct MeshData
    {
        std::vector<Vertex> vertices;
        std::vector<u32> indices;
    };

    class StaticMesh : NonCopyable
    {
    public:
        StaticMesh() = default;
        StaticMesh(StaticMesh &&) = default;
        StaticMesh &operator=(StaticMesh &&) = default;

        StaticMesh(const MeshData &data);

        void setup() const;
        void draw() const;
        void draw_instanced(size_t instances) const;
		
		TypedBuffer<u32>* get_indices() { return &_index_buffer; }
		TypedBuffer<Vertex>* get_vertices() { return &_vertex_buffer; }

        inline const glm::vec3& get_center() const {
            return _center;
        }

        inline const float get_radius() const {
            return _radius;
        }

    private:
        TypedBuffer<Vertex> _vertex_buffer;
        TypedBuffer<u32> _index_buffer;

        glm::vec3 _center;
        float _radius;
    };

} // namespace OM3D

#endif // STATICMESH_H
