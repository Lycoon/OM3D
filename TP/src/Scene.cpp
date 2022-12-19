#include "Scene.h"

#include <TypedBuffer.h>
#include <shader_structs.h>

#include <algorithm>
#include <glad/glad.h>
#include <glm/gtx/string_cast.hpp>

#include <iostream>

namespace OM3D
{

    Scene::Scene()
    {}

    void Scene::add_object(SceneObject obj)
    {
        _objects.emplace_back(std::move(obj));
    }

    void Scene::add_object(PointLight obj)
    {
        _point_lights.emplace_back(std::move(obj));
    }
    
    void Scene::render(const Camera &camera) const
    {
        // Fill and bind frame data buffer
        TypedBuffer<shader::FrameData> buffer(nullptr, 1);
        {
            auto mapping = buffer.map(AccessType::WriteOnly);
            mapping[0].camera.view_proj = camera.view_proj_matrix();
            mapping[0].point_light_count = u32(_point_lights.size());
            mapping[0].sun_color = glm::vec3(1.0f, 1.0f, 1.0f);
            mapping[0].sun_dir = glm::normalize(_sun_direction);
        }
        buffer.bind(BufferUsage::Uniform, 0);

        // Fill and bind lights buffer
        TypedBuffer<shader::PointLight> light_buffer(
            nullptr, std::max(_point_lights.size(), size_t(1)));
        {
            auto mapping = light_buffer.map(AccessType::WriteOnly);
            for (size_t i = 0; i != _point_lights.size(); ++i)
            {
                const auto &light = _point_lights[i];
                mapping[i] = { light.position(), light.radius(), light.color(),
                               0.0f };
            }
        }
        light_buffer.bind(BufferUsage::Storage, 1);

		// Fill and bind instance buffer
        auto instances = Instances();
        for (const auto& obj : _objects)
        {
            auto mat = obj.get_material().get();
            instances[mat].push_back(&obj);
        }

        for (const auto& e : instances)
        {
            const auto tr_buffer = std::make_shared<TypedBuffer<shader::ModelTransform>>(
                nullptr, 
                std::max(e.second.size(), (size_t)1)
            );
            {
                // Mapping model transforms to SSBO
                auto mapping = tr_buffer->map(AccessType::WriteOnly);
                for (int i = 0; i < e.second.size(); ++i) 
                {
                    const SceneObject* obj = e.second[i];
                    mapping[i] = { obj->transform() };
                }
            }
            tr_buffer->bind(BufferUsage::Storage, 2);

            auto mesh = e.second[0]->get_mesh();
            e.first->bind();
            mesh->draw_instanced(e.second.size());
        }
		
		// Frustum culling
        /*
        OM3D::Frustum frustum = camera.build_frustum();
        for (const SceneObject &obj : _objects)
        {
            const auto transform = obj.transform();
            const glm::vec3 scale = glm::vec3(glm::length(transform[0]),
                                              glm::length(transform[1]),
                                              glm::length(transform[2]));
            const float max_scale = std::max(std::max(scale.x, scale.y), scale.z);
            int inside = 0;
            for (auto normal : {frustum._top_normal, frustum._bottom_normal,
                                frustum._left_normal, frustum._right_normal,
                                frustum._near_normal})
            {
                const auto center = obj.get_mesh()->get_center();
                const auto mesh_pos_world = transform * glm::vec4(center, 1) + glm::vec4(normal, 1) * obj.get_mesh()->get_radius() * max_scale;
                const auto ray = mesh_pos_world - glm::vec4(camera.position(), 1);
                const bool is_in = glm::dot(ray, glm::vec4(normal, 1)) > 0.0f;
                if (is_in)
                    inside++;
            }
            if (inside >= 5)
            {
                obj.render();
            }
        }
		*/
    }

} // namespace OM3D
