#ifndef SCENEVIEW_H
#define SCENEVIEW_H

#include <Camera.h>
#include <Scene.h>

namespace OM3D
{

    class SceneView
    {
    public:
        SceneView(const Scene *scene = nullptr);

        Camera &camera();
        const Camera &camera() const;

        void render() const;

    private:
        const Scene *_scene = nullptr;
        Camera _camera;
    };

} // namespace OM3D

#endif // SCENEVIEW_H
