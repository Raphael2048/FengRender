#include "application.hpp"

namespace feng
{

    Application::Application()
    {
        window_ = std::make_unique<Window>(GetModuleHandleA(nullptr), "Feng Render, FPS:", 1280, 720);
        renderer_ = std::make_unique<Renderer>(*window_);
        Root.reset(new Scene());

        QueryPerformanceCounter((LARGE_INTEGER*)&last_time);
        __int64 count_per_sec;
        QueryPerformanceFrequency((LARGE_INTEGER*)&count_per_sec);
        seconds_per_count = 1.0f / count_per_sec;
    }

    void Application::Init()
    {
        // add camera, lights, static_meshes etc.
        OnInit();

        // build octree needed
        Root->Init();

        // Init renderer resources
        renderer_->Init(*Root);

        window_->SetRenderLoop([&](){
            float deltatime = Update();
            Root->Update(deltatime);
            renderer_->Draw(*Root);
        });

        window_->SetMouseWheelCallback([&](short value){
            OnMouseWheel(value);
        });

        window_->SetMouseMoveCallback([&](WPARAM state, int x, int y){
            OnMouseMove(state, x, y);
        });
    }

    void Application::OnInit()
    {
        Camera* camera = new Camera(
            Vector3{0, 0, 10}, Vector3{0, 0, 0}, 10.0f, 1000.0f, 60.0f, 1.777f
        );
        Root->SetCamera(camera);
    }

    float Application::Update()
    {
        __int64 now_time;
        QueryPerformanceCounter((LARGE_INTEGER*)&now_time);

        float deltatime = (now_time - last_time) * seconds_per_count;
        last_time = now_time;


        static float accum_time = 0;
        static int accum_frame = 0;
        accum_time += deltatime;
            accum_frame++;
        if (accum_time > 1.0f)
        {
            int fps = static_cast<int>(accum_frame / accum_time);
            std::wstring text = L"Feng Render, FPS:" + std::to_wstring(fps);
            SetWindowTextW(window_->GetWindowHandle(), text.c_str());
            accum_frame = 0;
            accum_time = 0;
        }

        return deltatime;
    }

    void Application::Run()
    {
        Init();
        window_->StartRenderLoop();
    }
}