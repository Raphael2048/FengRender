#pragma once

#include "util/defines.hpp"
#include "util/math.hpp"

namespace feng
{
    class Node
    {
    public:
        Node(const Vector3& position, const Vector3& rotation, const Vector3& scale) :
            position_(position), rotation_(rotation), scale_(scale){}
        Node &SetRotation(const Vector3 &r)
        {
            rotation_ = r;
            dirty_ = true;
            return *this;
        }
        Vector3 GetRotation() const
        {
            return rotation_;
        }

        Node &SetPosition(const Vector3 &p)
        {
            position_ = p;
            dirty_ = true;
            return *this;
        }

        Vector3 GetPosition() const
        {
            return position_;
        }

        Node &SetScale(const Vector3 &s)
        {
            scale_ = s;
            dirty_ = true;
            return *this;
        }

        Vector3 GetScale() const
        {
            return scale_;
        }

        virtual void Update(float deltatime) { }

        Matrix MatrixView;
        Matrix MatrixInvView;
    protected:


        Vector3 position_ = {0, 0, 0};
        Vector3 rotation_ = {0, 0, 0};
        // Roll, Pitch, Yaw
        Vector3 scale_ = {1, 1, 1};
        Matrix transform_;
        bool dirty_ = true;
    };
} // namespace feng