//#ifndef _V3D_FPSCAMERA_H_
//#define _V3D_FPSCAMERA_H_
//
//#include "Camera.h"
//#include "event/KeyCodes.h"
//
//namespace v3d
//{
//namespace scene
//{
//    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//    struct SMoveKeys
//    {
//        v3d::EKeyCode _left;
//        v3d::EKeyCode _right;
//        v3d::EKeyCode _forward;
//        v3d::EKeyCode _back;
//        v3d::EKeyCode _acceleration;
//    };
//
//    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//    class CFPSCamera : public CCamera
//    {
//    public:
//
//        CFPSCamera();
//        ~CFPSCamera();
//
//        void            update(s32 dt) override;
//
//        void            setSpeed(f32 speed);
//        f32             getSpeed() const;
//
//        void            setMoveKeys(const SMoveKeys& keys);
//
//    private:
//
//        void            move(const core::Vector3D& direction);
//        bool            isPointOut(const core::Vector3D& point);
//
//        void            rotateByMouse();
//        void            rotate(f32 angle, const core::Vector3D& point);
//
//        SMoveKeys       m_keys;
//        f32             m_speed;
//
//        core::Vector3D  m_max;
//        core::Vector3D  m_min;
//    };
//
//    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
//}
//}
//
//#endif //_V3D_FPSCAMERA_H_
