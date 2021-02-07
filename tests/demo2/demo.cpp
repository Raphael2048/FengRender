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
        Root->SetCamera(new Camera(Vector3{10, 0, 20}, Vector3{0, 0, 0}, 10.0f, 100.0f, 60.0f, 1280.0f / 720.0f));

        Root->SetDirectionalLight(new DirectionalLight(Vector3(1, -5, -2), Color(3, 2, 2)));
        // Root->SetSkyLight(new SkyLight(L"resources\\textures\\scubemap_street.dds", 1));

        //Root->AddPointLight(new PointLight(Vector3(0, 5, -5), Color(1, 2, 3), 200));

        auto sphere = AssimpMeshLoader::LoadModel("resources\\models\\sphere.fbx");

        auto cornell = AssimpMeshLoader::LoadModel("resources\\models\\cornell.fbx");

        auto material = std::make_shared<StaticMaterial>(
            L"resources\\textures\\rusted_iron\\albedo.dds",
            L"resources\\textures\\rusted_iron\\normal.dds",
            L"resources\\textures\\rusted_iron\\roughness.dds",
            L"resources\\textures\\rusted_iron\\metallic.dds");

        auto material_rock = std::make_shared<StaticMaterial>(
            L"resources\\textures\\rock\\albedo.dds",
            L"resources\\textures\\rock\\normal.dds",
            L"resources\\textures\\rock\\roughness.dds",
            L"resources\\textures\\rock\\metallic.dds");

        auto material_pica = std::make_shared<StaticMaterial>(
            L"resources\\textures\\pica\\albedo.dds",
            L"resources\\textures\\pica\\normal.dds",
            L"resources\\textures\\pica\\roughness.dds",
            L"resources\\textures\\pica\\metallic.dds");

        auto material_pure = std::make_shared<StaticMaterial>(
            L"resources\\textures\\pure\\albedo.dds",
            L"resources\\textures\\pure\\normal.dds",
            L"resources\\textures\\pure\\roughness.dds",
            L"resources\\textures\\pure\\metallic.dds");

        auto material_red = std::make_shared<StaticMaterial>(
            L"resources\\textures\\red\\albedo.dds",
            L"resources\\textures\\red\\normal.dds",
            L"resources\\textures\\red\\roughness.dds",
            L"resources\\textures\\red\\metallic.dds");

        auto material_green = std::make_shared<StaticMaterial>(
            L"resources\\textures\\green\\albedo.dds",
            L"resources\\textures\\green\\normal.dds",
            L"resources\\textures\\green\\roughness.dds",
            L"resources\\textures\\green\\metallic.dds");

        Root->AddStaticMesh(new StaticMesh(
            Vector3::Zero,
            Vector3::Zero,
            Vector3::One,
            cornell[0],
            material_pica
        ));

        Root->AddStaticMesh(new StaticMesh(
            Vector3::Zero,
            Vector3::Zero,
            Vector3::One,
            cornell[1],
            material_pure
        ));
        Root->AddStaticMesh(new StaticMesh(
            Vector3::Zero,
            Vector3::Zero,
            Vector3::One,
            cornell[2],
            material_pica
        ));
        Root->AddStaticMesh(new StaticMesh(
            Vector3::Zero,
            Vector3::Zero,
            Vector3::One,
            cornell[3],
            material_pica
        ));
        Root->AddStaticMesh(new StaticMesh(
            Vector3::Zero,
            Vector3::Zero,
            Vector3::One,
            cornell[4],
            material_green
        ));

        Root->AddStaticMesh(new StaticMesh(
            Vector3(0, 0, -5),
            Vector3(20, 10, 0),
            Vector3(3, 3, 3),
            sphere[0],
            material));
    
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