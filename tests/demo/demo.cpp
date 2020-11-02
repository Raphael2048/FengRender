#include "feng.hpp"
#include "scene/camera.hpp"
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

        std::array<Vertex, 8> vertices =
        {
            Vertex({Vector3(-1.0f, -1.0f, -1.0f), Vector2(0.0f, 1.0f)}),
            Vertex({Vector3(-1.0f, +1.0f, -1.0f), Vector2(1.0f, 1.0f)}),
            Vertex({Vector3(+1.0f, +1.0f, -1.0f), Vector2(1.0f, 1.0f)}),
            Vertex({Vector3(+1.0f, -1.0f, -1.0f), Vector2(1.0f, 1.0f)}),
            Vertex({Vector3(-1.0f, -1.0f, +1.0f), Vector2(1.0f, 1.0f)}),
            Vertex({Vector3(-1.0f, +1.0f, +1.0f), Vector2(1.0f, 1.0f)}),
            Vertex({Vector3(+1.0f, +1.0f, +1.0f), Vector2(1.0f, 1.0f)}),
            Vertex({Vector3(+1.0f, -1.0f, +1.0f), Vector2(1.0f, 1.0f)}),
        };

        std::array<std::uint32_t, 36> indices =
        {
            // front face
            0, 1, 2,
            0, 2, 3,

            // back face
            4, 6, 5,
            4, 7, 6,

            // left face
            4, 5, 1,
            4, 1, 0,

            // right face
            3, 2, 6,
            3, 6, 7,

            // top face
            1, 5, 6,
            1, 6, 2,

            // bottom face
            4, 0, 3,
            4, 3, 7
        };


        StaticMesh* mesh = new StaticMesh(
            Vector3(0, 0, 0),
            Vector3(0, 0, 0),
            Vector3::One,
            vertices.data(), vertices.size(),
            indices.data(), indices.size()
        );

        Root->AddStaticMesh(mesh);
    }

    virtual void OnMouseWheel(short value) override
    {
        auto& camera = Root->Camera;
        camera->SetPosition(camera->GetPosition() + camera->MatrixView.Forward() * (value / 100.0f));
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