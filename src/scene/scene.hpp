#pragma once

#include "scene/camera.hpp"
#include "scene/static_mesh.hpp"
namespace feng
{
    class Scene : public Uncopyable
    {
    public:
        void SetCamera(Camera* camera) { Camera.reset(camera); }
        void AddStaticMesh(StaticMesh* mesh) { StaticMeshes.emplace_back(mesh);  }
        void Update(float delta);
        std::unique_ptr<Camera> Camera;
        std::vector<std::unique_ptr<StaticMesh>> StaticMeshes;
    };

} // namespace feng