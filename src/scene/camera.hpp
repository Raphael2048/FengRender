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
        // 这里fov是垂直方向的角度, 不是水平方向
        float far_;
        float fov_;
        // 宽度/高度
        float aspect_;

    };
}