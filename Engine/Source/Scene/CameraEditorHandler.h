#pragma once

#include "Common.h"
#include "CameraHandler.h"

#include "Events/Input/InputEventHandler.h"

namespace v3d
{
namespace scene
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief CameraEditorHandler class
    */
    class CameraEditorHandler final : public CameraHandler
    {
    public:

        explicit CameraEditorHandler(Camera* camera, const math::Vector3D& position) noexcept;
        ~CameraEditorHandler();

        CameraEditorHandler() = delete;
        CameraEditorHandler(const CameraEditorHandler&) = delete;

        void setRotation(const math::Vector3D& rotation);
        const math::Vector3D& getRotation() const;

        void setMoveSpeed(f32 speed);
        f32 getMoveSpeed() const;

        void setRotationSpeed(f32 speed);
        f32 getRotationSpeed() const;

        void update(f32 deltaTime) override;

        void handleInputEventCallback(const v3d::event::InputEventHandler* handler, const event::InputEvent* event);

    private:

        static constexpr f32 k_constrainPitch = 89.0f;

        enum CameraMode
        {
            None,
            FPSMode,
            ArcballMode,
        };
        CameraMode m_cameraMode;

        struct Move
        {
            bool _forward;
            bool _back;
            bool _left;
            bool _right;
        };
        Move m_direction;
        f32 m_moveSpeed;
        f32 m_accelerationSpeed;
        f32 m_rotationSpeed;

        bool isDirectionChange() const;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d