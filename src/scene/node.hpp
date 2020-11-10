#pragma once

#include "util/defines.hpp"
#include "util/math.hpp"
#include "dx12/dx12_defines.hpp"
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

        virtual Node &SetScale(const Vector3 &s)
        {
            scale_ = s;
            dirty_ = true;
            return *this;
        }

        Vector3 GetScale() const
        {
            return scale_;
        }

        bool IsCBDirty()
        {
            if (cb_dirty_ > 0)
            {
                --cb_dirty_;
                return true;
            }
            return false;
        }

        const Box& GetBoundingBox()
        {
            if(box_dirty_)
            {
                box_dirty_ = false;
                RefreshBoundingBox();
            }
            return box_;
        }

        virtual void RefreshBoundingBox() {}

        virtual void Update([[maybe_unused]]float deltatime) { }

        Matrix MatrixWorld;
        Matrix MatrixInvWorld;
    protected:


        Vector3 position_ = {0, 0, 0};
        // Pitch, Yaw, Roll 
        // order: yaw->pitch->roll / Y->X->Z) 
        Vector3 rotation_ = {0, 0, 0};
        Vector3 scale_ = {1, 1, 1};

        Box box_;
        // for transform
        bool dirty_ : 1 = true;
        // for bounding box
        bool box_dirty_ : 1 = true;
        // for constant buffer
        uint8_t cb_dirty_ : 3 = BACK_BUFFER_SIZE;
    };
} // namespace feng