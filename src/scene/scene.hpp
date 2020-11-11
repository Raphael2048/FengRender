#pragma once

#include "scene/camera.hpp"
#include "scene/static_mesh.hpp"
#include "util/octree.hpp"
namespace feng
{
    class Scene : public Uncopyable
    {
    public:

        void SetCamera(Camera *camera) { Camera.reset(camera); }
        void AddStaticMesh(StaticMesh *mesh) { StaticMeshes.emplace_back(mesh); }
        void Init()
        {
            // 先算出BoundingBox
            Update(0);
            StaticMeshesOctree.reset(
                new Octree<StaticMeshProxy>(
                    Vector3(0, 0, 0),
                    1000.0f
                )
            );
            StaticMeshesVisibity.resize(StaticMeshes.size());
            for (auto it = StaticMeshes.cbegin(); it != StaticMeshes.cend(); ++it)
            {
                StaticMeshProxy proxy;
                proxy.pointer = (*it).get();
                proxy.id = std::distance(StaticMeshes.cbegin(), it);
                StaticMeshesOctree->AddElement(proxy);
            }
        }
        void Update(float delta)
        {
            Camera->Update(delta);

            for (auto &node : StaticMeshes)
            {
                node->Update(delta);
            }
        }

        std::unique_ptr<Camera> Camera;
        std::vector<std::unique_ptr<StaticMesh>> StaticMeshes;

        struct StaticMeshProxy
        {
            StaticMesh* pointer;
            uint32_t id;
            static const int MaxNodeDepth = 5;
            static const int MaxElementsPerLeaf = 5;
            Box GetBoundingBox() const { return pointer->GetBoundingBox(); }
        };
        std::unique_ptr<Octree<StaticMeshProxy>> StaticMeshesOctree;
        std::vector<bool> StaticMeshesVisibity;
    };

} // namespace feng