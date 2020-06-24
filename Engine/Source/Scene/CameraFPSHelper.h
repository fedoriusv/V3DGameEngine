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

        void update(f32 deltaTime) override;

        void rotateHandlerCallback(const v3d::event::InputEventHandler* handler, const event::MouseInputEvent* event, bool mouseCapture);
        void moveHandlerCallback(const v3d::event::InputEventHandler* handler, const event::KeyboardInputEvent* event);

    private:

#if 0
        void move(const core::Vector3D& direction);
        void rotate(f32 angle, const core::Vector3D& point);
        bool isPointOut(const core::Vector3D& point);
#endif
        f32 m_moveSpeed;

        const f32 k_rotationSpeed = 0.8f;
        const f32 k_movementSpeed = 1000.0f;
        const f32 k_accelerationSpeed = 10.0f;

        static constexpr f32 k_constrainPitch = 89.0f;

        struct Move
        {
            bool _forward;
            bool _back;
            bool _left;
            bool _right;
        };
        Move    m_direction;

        bool isDirectionChange() const;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d