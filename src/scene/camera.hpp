#pragma once
#include "scene/node.hpp"

namespace feng
{

    class Camera : public Node
    {
    public:
        Camera(const Vector3& position, const Vector3& rotation, float near, float far, float fov, float aspect);

        virtual void Update(float delta) override;
        Matrix MatrixView;
        Matrix MatrixInvView;
        Matrix MatrixProj;
        Matrix MatrixInvProj;
    private:
        void RefreshMatrix();
        float near_;
        float far_;
        float fov_;
        float aspect_;

    };
}