#pragma once
#include "scene/node.hpp"

namespace feng
{
    // 平行光方向指向负Z轴, 和摄影机一样
    // 三级CSM阴影
    class DirectionalLight : public Node
    {
    public:
        friend class DirectionalLightEffect;
        // HDR linear color, unit : (W/m^2) or (lux)
        DirectionalLight(const Vector3 &rotation, const Color &color);

        virtual void Update(float time) override;

        virtual Node &SetScale([[maybe_unused]] const Vector3 &scale) override { return *this; }
        virtual Node &SetPosition([[maybe_unused]] const Vector3 &pos) override { return *this; }

    private:
        Color color_;
        float shadow_distance_ = 200.0f;
    };

    class SpotLight : public Node
    {
    public:
        SpotLight(const Vector3& position, const Vector3& rotation, const Color& color, float radius, float inner_angle, float outer_angle_, bool has_shadow);
        virtual void Update(float time) override;
        virtual void RefreshBoundingBox() override;
        const DirectX::BoundingFrustum GetBoundingFrustum();
        Matrix MatrixProj;
        Matrix MatrixInvProj;
    private:
        DirectX::BoundingFrustum frustum_;
        // 单位 (W/sr) or (cd)
        Color color_;
        float radius_;
        float inner_angle_;
        float outer_angle_;
        bool cast_shadow_;
    };
} // namespace feng