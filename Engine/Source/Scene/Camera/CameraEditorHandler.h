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

        explicit CameraEditorHandler(std::unique_ptr<Camera> camera) noexcept;
        ~CameraEditorHandler();

        CameraEditorHandler() = delete;
        CameraEditorHandler(const CameraEditorHandler&) = delete;

        void setMoveSpeed(f32 speed);
        f32 getMoveSpeed() const;

        void setRotationSpeed(f32 speed);
        f32 getRotationSpeed() const;

        void update(f32 deltaTime) override;

        void handleInputEventCallback(const v3d::event::InputEventHandler* handler, const event::InputEvent* event);

    private:

        bool m_freeFlyMode;
        bool m_orbitingMode;

        const math::TVector2D<f32>  m_distanceLimits;

        math::TVector2D<f32>        m_deltaRotation;
        f32                         m_deltaDistance;
        math::Vector4D              m_direction;

        f32                         m_rotationSpeed;
        f32                         m_moveSpeed;
        f32                         m_accelerationSpeed;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d