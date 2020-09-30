#include "feng.hpp"
#include "scene/camera.hpp"
#include <cstdio>

using namespace feng;
class Demo : public Application
{   
    void OnInit() override
    {
        Camera* camera = new Camera(
            Vector3{0, 0, 10}, Vector3{0, 0, 0}, 10.0f, 1000.0f, 60.0f, 1.777f
        );
        Root->SetCamera(camera);
    }
};

int main()
{
    Demo ddd;
    ddd.Run();
    getchar();
}