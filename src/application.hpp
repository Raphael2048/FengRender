#pragma once

#include "renderer.hpp"
#include "window.hpp"
#include "scene/scene.hpp"

namespace feng
{
    class Application
    {
    public:
        Application();

        void Init();
        virtual void OnInit(); 
        void Run();
        float Update();

    protected:
        std::unique_ptr<Scene> Root;

    private:
        std::unique_ptr<Window> window_;
        std::unique_ptr<Renderer> renderer_;
        __int64 last_time;
        float seconds_per_count;
    };

}