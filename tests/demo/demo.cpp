#include "feng.hpp"
#include <cstdio>
#include <algorithm>
#include <cmath>
using namespace feng;
class Demo : public Application
{
protected:
    void OnInit() override
    {
        Camera *camera = new Camera(
            Vector3{0, 0, 20}, Vector3{0, 180, 0}, 1.0f, 1000.0f, 60.0f, 1280.0f / 720.0f);
        Root->SetCamera(camera);

        auto m = AssimpMeshLoader::LoadModel("resources\\models\\cube.fbx");

        auto material = std::make_shared<StaticMaterial>(
            L"resources\\textures\\rusted_iron\\albedo.dds",
            L"resources\\textures\\rusted_iron\\normal.dds",
            L"resources\\textures\\rusted_iron\\roughness.dds",
            L"resources\\textures\\rusted_iron\\metallic.dds");

        Root->AddStaticMesh(new StaticMesh(
            Vector3(0, 0, 0),
            Vector3(0, 0, 0),
            Vector3::One,
            m[0],
            material));

        Root->AddStaticMesh(new StaticMesh(
            Vector3(10, 0, 0),
            Vector3(0, 0, 0),
            Vector3::One,
            m[0],
            material));

        for (int i = 0; i < 5; i++)
        {
            for (int j = 0; j < 5; j++)
            {
                for (int k = 0; k < 5; k++)
                {
                    Root->AddStaticMesh(new StaticMesh(
                        Vector3(i * 20, j * 20, k *20),
                        Vector3(0, 0, 0),
                        Vector3(1.5, 1.5, 1.5),
                        m[0],
                        material));
                }
            }
        }
    }

    virtual void OnMouseWheel(short value) override
    {
        auto &camera = Root->Camera;
        camera->SetPosition(camera->GetPosition() + camera->MatrixWorld.Forward() * (value / 100.0f));
    }

    virtual void OnMouseMove(WPARAM param, int x, int y) override
    {
        static int last_x = x;
        static int last_y = y;
        if ((param & MK_RBUTTON) != 0)
        {
            auto &camera = Root->Camera;
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