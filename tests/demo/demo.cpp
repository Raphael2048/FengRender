#include "feng.hpp"
#include "scene/camera.hpp"
#include <cstdio>

using namespace feng;
class Demo : public Application
{   
    void OnInit() override
    {
        Camera* camera = new Camera(
            Vector3{0, 0, -20}, Vector3{10, 0, 0}, 10.0f, 1000.0f, 60.0f, 1280.0f/720.0f
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
};

int main()
{
    Demo ddd;
    ddd.Run();
}