#ifndef TEXTURE_H
#define TEXTURE_H

#include <ImageFormat.h>
#include <glm/vec2.hpp>
#include <graphics.h>
#include <memory>
#include <vector>

namespace OM3D
{

    struct TextureData
    {
        std::unique_ptr<u8[]> data;
        glm::uvec2 size = {};
        ImageFormat format;

        static Result<TextureData> from_file(const std::string &file_name);
    };

    class Texture
    {
    public:
        Texture() = default;
        Texture(Texture &&) = default;
        Texture &operator=(Texture &&) = default;

        ~Texture();

        Texture(const TextureData &data);
        Texture(const glm::uvec2 &size, ImageFormat format);

        void bind(u32 index) const;
        void bind_as_image(u32 index, AccessType access);

        const glm::uvec2 &size() const;

        static u32 mip_levels(glm::uvec2 size);

    private:
        friend class Framebuffer;

        GLHandle _handle;
        glm::uvec2 _size = {};
        ImageFormat _format;
    };

} // namespace OM3D

#endif // TEXTURE_H
