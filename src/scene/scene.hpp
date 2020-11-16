#pragma once

#include "scene/camera.hpp"
#include "scene/static_mesh.hpp"
#include "scene/light.hpp"
#include "util/octree.hpp"
namespace feng
{
    class Scene : public Uncopyable
    {
    public:
        void SetCamera(Camera *camera) { Camera.reset(camera); }
        void SetDirectionalLight(DirectionalLight *light) { DirectionalLight.reset(light); }
        void AddStaticMesh(StaticMesh *mesh) { StaticMeshes.emplace_back(mesh); }
        void AddSpotLight(SpotLight *light) { SpotLights.emplace_back(light); }
        void Init()
        {
            // 先算出BoundingBox
            Update(0);
            Box bounds = StaticMeshes[0]->GetBoundingBox();
            for (auto &mesh : StaticMeshes)
            {
                bounds = bounds.Combine(mesh->GetBoundingBox());
            }

            StaticMeshesOctree.reset(
                new Octree<StaticMeshProxy>(
                    bounds.Center,
                    (std::max)({bounds.Extents.x, bounds.Extents.y, bounds.Extents.z})));
            StaticMeshesVisibity.resize(StaticMeshes.size());
            for (auto it = StaticMeshes.cbegin(); it != StaticMeshes.cend(); ++it)
            {
                StaticMeshProxy proxy;
                proxy.pointer = (*it).get();
                proxy.id = (uint32_t)std::distance(StaticMeshes.cbegin(), it);
                StaticMeshesOctree->AddElement(proxy);
            }
        }
        void Update(float delta)
        {
            Camera->Update(delta);

            if (DirectionalLight)
                DirectionalLight->Update(delta);

            for (auto &light : SpotLights)
            {
                light->Update(delta);
            }

            for (auto &node : StaticMeshes)
            {
                node->Update(delta);
            }
        }

        std::unique_ptr<Camera> Camera;

        std::unique_ptr<DirectionalLight> DirectionalLight;
        std::vector<std::unique_ptr<SpotLight>> SpotLights;

        std::vector<std::unique_ptr<StaticMesh>> StaticMeshes;

        struct StaticMeshProxy
        {
            StaticMesh *pointer;
            uint32_t id;
            static const int MaxNodeDepth = 5;
            static const int MaxElementsPerLeaf = 5;
            Box GetBoundingBox() const { return pointer->GetBoundingBox(); }
        };
        std::unique_ptr<Octree<StaticMeshProxy>> StaticMeshesOctree;
        std::vector<bool> StaticMeshesVisibity;
    };

} // namespace feng