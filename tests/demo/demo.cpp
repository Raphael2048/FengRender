#include "feng.hpp"
#include "scene/camera.hpp"
#include "util/model_loader.hpp"
#include <cstdio>
#include <algorithm>
#include <cmath>
using namespace feng;
class Demo : public Application
{
    protected:
    void OnInit() override
    {
        Camera* camera = new Camera(
            Vector3{0, 0, 20}, Vector3{0, 0, 0}, 1.0f, 1000.0f, 60.0f, 1280.0f/720.0f
        );
        Root->SetCamera(camera);

        auto m = AssimpMeshLoader::LoadModel("resources\\models\\cylinder.fbx");

        StaticMesh* mesh1 = new StaticMesh(
            Vector3(0, 0, 0),
            Vector3(0, 0, 0),
            Vector3::One,
            m[0]
        );

        Root->AddStaticMesh(mesh1);
        StaticMesh* mesh2 = new StaticMesh(
            Vector3(10, 0, 0),
            Vector3(0, 0, 0),
            Vector3::One,
            m[0]
        );

        Root->AddStaticMesh(mesh2);
    }

    virtual void OnMouseWheel(short value) override
    {
        auto& camera = Root->Camera;
        camera->SetPosition(camera->GetPosition() + camera->MatrixWorld.Forward() * (value / 100.0f));
    }

    virtual void OnMouseMove(WPARAM param, int x, int y) override
    {
        static int last_x = x;
        static int last_y = y;
        if ((param & MK_RBUTTON) != 0)
        {
            auto& camera = Root->Camera;
            float dx = (x - last_x) / 10.0f;
            float dy = (y - last_y) / 10.0f;
            Vector3 rotation = camera->GetRotation();
            rotation.y = std::clamp(fmod(rotation.y - dx + 360.0f, 360.0f), 0.0f, 360.0f);
            rotation.x = std::clamp(rotation.x - dy, -90.0f, 90.0f);
            camera->SetRotation(rotation);
        }
        last_x = x;
        last_y = y;
    }
};

int main()
{
    Demo ddd;
    ddd.Run();
}