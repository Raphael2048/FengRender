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
        DirectionalLight(const Vector3& direction, const Color &color);

        virtual void Update(float time) override;

        virtual Node &SetScale([[maybe_unused]] const Vector3 &scale) override { return *this; }
        virtual Node &SetPosition([[maybe_unused]] const Vector3 &pos) override { return *this; }
        virtual Node &SetRotation([[maybe_unused]] const Vector3 &rotation) override { return *this; }
        DirectionalLight& SetDirection(const Vector3& direction)
        {
            direction_ = direction;
            direction_.Normalize();
            dirty_ = true;
            return *this;
        }

    private:
        Vector3 direction_;
        Color color_;
        float shadow_distance_ = 200.0f;
    };

    class SpotLight : public Node
    {
    public:
        friend class SpotLightEffect;
        SpotLight(const Vector3& position, const Vector3& direction, const Color& color, float radius, float inner_angle, float outer_angle_);
        virtual Node &SetScale([[maybe_unused]] const Vector3 &scale) override { return *this; }
        virtual Node &SetRotation([[maybe_unused]] const Vector3 &rotation) override { return *this; }
        virtual void Update(float time) override;
        virtual void RefreshBoundingBox() override;
        const DirectX::BoundingFrustum GetBoundingFrustum();
        SpotLight& SetDirection(const Vector3& direction)
        {
            direction_ = direction;
            direction_.Normalize();
            dirty_ = true;
            return *this;
        }

        Matrix MatrixProj;
        Matrix MatrixInvProj;
    private:
        DirectX::BoundingFrustum frustum_;
        Vector3 direction_;
        // 单位 (W/sr) or (cd)
        Color color_;
        float radius_;
        float inner_angle_;
        float outer_angle_;
    };
 
    class PointLight : Node
    {
    public:
        friend class PointLightEffect;
        PointLight(const Vector3& position, const Color& color, float radius);
        virtual Node &SetScale([[maybe_unused]] const Vector3 &scale) override { return *this; }
        virtual Node &SetRotation([[maybe_unused]] const Vector3 &rotation) override { return *this; }
        virtual void Update(float time) override;
        virtual void RefreshBoundingBox() override;
    private:
        // 单位 (W/sr) or (cd)
        Color color_;
        float radius_;
    };

    class SkyLight : Node
    {
    public:
        friend class SkyLightEffect;
        SkyLight(const std::wstring& path, float intensity);
        virtual Node &SetScale([[maybe_unused]] const Vector3 &scale) override { return *this; }
        virtual Node &SetPosition([[maybe_unused]] const Vector3 &pos) override { return *this; }
        virtual Node &SetRotation([[maybe_unused]] const Vector3 &rotation) override { return *this; }
        const std::wstring& GetTexturePath()
        {
            return path_;
        }
    private:
        std::wstring path_;
        float intensity_;
    };
} // namespace feng