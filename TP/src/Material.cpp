#include "Material.h"

#include <algorithm>
#include <glad/glad.h>

namespace OM3D
{

    Material::Material()
    {}

    void Material::set_program(std::shared_ptr<Program> prog)
    {
        _program = std::move(prog);
    }

    void Material::set_blend_mode(BlendMode blend)
    {
        _blend_mode = blend;
    }

    void Material::set_depth_test_mode(DepthTestMode depth)
    {
        _depth_test_mode = depth;
    }

    void Material::set_texture(u32 slot, std::shared_ptr<Texture> tex)
    {
        if (const auto it =
                std::find_if(_textures.begin(), _textures.end(),
                             [&](const auto &t) { return t.second == tex; });
            it != _textures.end())
        {
            it->second = std::move(tex);
        }
        else
        {
            _textures.emplace_back(slot, std::move(tex));
        }
    }

    void Material::bind() const
    {
        switch (_blend_mode)
        {
        case BlendMode::None:
            glDisable(GL_BLEND);
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
            glFrontFace(GL_CCW);
            break;

        case BlendMode::Alpha:
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDisable(GL_CULL_FACE);
            break;
        }

        switch (_depth_test_mode)
        {
        case DepthTestMode::None:
            glDisable(GL_DEPTH_TEST);
            break;

        case DepthTestMode::Equal:
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_EQUAL);
            break;

        case DepthTestMode::Standard:
            glEnable(GL_DEPTH_TEST);
            // We are using reverse-Z
            glDepthFunc(GL_GEQUAL);
            break;

        case DepthTestMode::Reversed:
            glEnable(GL_DEPTH_TEST);
            // We are using reverse-Z
            glDepthFunc(GL_LEQUAL);
            break;
        }

        for (const auto &texture : _textures)
        {
            texture.second->bind(texture.first);
        }
        _program->bind();
    }

    std::shared_ptr<Material> Material::empty_material(std::vector<std::string> debugDefines)
    {
        static std::weak_ptr<Material> weak_material;
        auto material = weak_material.lock();

        if (!material)
        {
            material = std::make_shared<Material>();
            material->_program = Program::from_files("lit.frag", "basic.vert", debugDefines);
            weak_material = material;
        }
        
        return material;
    }

    Material Material::textured_material(std::vector<std::string> debugDefines)
    {
        std::vector<std::string> defines = { "TEXTURED" };
        defines.insert(defines.end(), debugDefines.begin(), debugDefines.end());

        Material material;
        material._program = Program::from_files("lit.frag", "basic.vert", defines);
        
        return material;
    }

    Material Material::textured_normal_mapped_material(std::vector<std::string> debugDefines)
    {
        std::vector<std::string> defines = { "TEXTURED", "NORMAL_MAPPED" };
        defines.insert(defines.end(), debugDefines.begin(), debugDefines.end());
        
        Material material;
        material._program = Program::from_files("lit.frag", "basic.vert", defines);

        return material;
    }

} // namespace OM3D
