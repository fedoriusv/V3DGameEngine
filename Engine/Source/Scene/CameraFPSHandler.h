#pragma once

#include "Common.h"
#include "CameraHandler.h"

#include "Events/InputEventHandler.h"

namespace v3d
{
namespace scene
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief CameraFPSHandler class
    */
    class CameraFPSHandler final : public CameraHandler
    {
    public:

        CameraFPSHandler() = delete;
        CameraFPSHandler(const CameraFPSHandler&) = delete;

        explicit CameraFPSHandler(Camera* camera, const math::Vector3D& position) noexcept;
        ~CameraFPSHandler();

        void setRotation(const math::Vector3D& rotation);
        const math::Vector3D& getRotation() const;

        void update(f32 deltaTime) override;

        void rotateHandleCallback(const v3d::event::InputEventHandler* handler, const event::MouseInputEvent* event, bool mouseCapture);
        void moveHandleCallback(const v3d::event::InputEventHandler* handler, const event::KeyboardInputEvent* event);

    private:

        const f32 k_rotationSpeed = 0.8f;
        const f32 k_movementSpeed = 100.0f;
        const f32 k_accelerationSpeed = 10.0f;

        static constexpr f32 k_constrainPitch = 89.0f;

        f32 m_moveSpeed;

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