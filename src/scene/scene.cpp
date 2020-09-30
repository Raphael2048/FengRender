#include "scene/scene.hpp"

namespace feng {
    void Scene::Update(float deltatime)
    {
        Camera->Update(deltatime);

        for(auto& node: StaticMeshes)
        {
            node->Update(deltatime);
        }
    }
}