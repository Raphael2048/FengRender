#pragma once
#include "scene/node.hpp"

namespace feng
{

    class Camera : public Node
    {
    public:
        friend class DirectionalLightEffect;
        Camera(const Vector3& position, const Vector3& rotation, float near_distance, float far_distance, float fov, float aspect);

        virtual void Update(float delta) override;
        virtual Node &SetScale(const Vector3 &s) override;
        virtual void RefreshBoundingBox() override;

        const DirectX::BoundingFrustum& GetBoundingFrustrum();

        Matrix MatrixProj;
        Matrix MatrixInvProj;
    private:

        DirectX::BoundingFrustum frustum_;
        float near_;
        float far_;
        float fov_;
        float aspect_;

    };
}