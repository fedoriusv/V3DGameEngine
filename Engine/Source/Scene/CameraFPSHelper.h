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

        void rotateHandlerCallback(v3d::event::InputEventHandler* handler, const event::MouseInputEvent* event);
        void moveHandlerCallback(v3d::event::InputEventHandler* handler, const event::KeyboardInputEvent* event);

    private:

        f32 m_moveSpeed;

        const f32 k_rotationSpeed = 0.8f;
        const f32 k_movementSpeed = 50.0f;
        const f32 k_accelerationSpeed = 2.0f;

        struct Move
        {
            bool _forward;
            bool _back;
            bool _left;
            bool _right;
        };

        Move    m_direction;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d