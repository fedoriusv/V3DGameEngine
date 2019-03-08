#pragma once

#include "Common.h"
#include "CameraHelper.h"

#include "Event/InputEventHandler.h"

namespace v3d
{
namespace scene
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * CameraFPSHelper
    */
    class CameraFPSHelper final : public CameraHelper
    {
    public:

        CameraFPSHelper() = delete;
        CameraFPSHelper(const CameraFPSHelper&) = delete;

        explicit CameraFPSHelper(Camera* camera, const core::Vector3D& position) noexcept;
        ~CameraFPSHelper();

        void setRotation(const core::Vector3D& rotation);
        const core::Vector3D& getRotation() const;

        void update(f32 deltaTime);

        void rotateHandlerCallback(v3d::event::InputEventHandler* handler, const event::MouseInputEvent* event, bool mouseCapture);
        void moveHandlerCallback(v3d::event::InputEventHandler* handler, const event::KeyboardInputEvent* event);

    private:

        void move(const core::Vector3D& direction);
        void rotate(f32 angle, const core::Vector3D& point);
        bool isPointOut(const core::Vector3D& point);

        f32 m_moveSpeed;

        const f32 k_rotationSpeed = 8.8f;
        const f32 k_movementSpeed = 1000.0f;
        const f32 k_accelerationSpeed = 10.0f;

        struct Move
        {
            bool _forward;
            bool _back;
            bool _left;
            bool _right;
        };

        Move    m_direction;
        bool    m_rotate;

        bool isDirectionChange() const;
        bool isRotationChange() const;

        core::Vector3D m_angle;

        core::Vector3D m_minBorder;
        core::Vector3D m_maxBorder;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d