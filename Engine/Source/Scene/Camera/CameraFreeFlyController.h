#pragma once

#include "Common.h"
#include "CameraController.h"

#include "Events/Input/InputEventHandler.h"

namespace v3d
{
namespace scene
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief CameraFreeFlyController class
    */
    class CameraFreeFlyController final : public CameraController
    {
    public:

        CameraFreeFlyController() = delete;
        CameraFreeFlyController(const CameraFreeFlyController&) = delete;

        explicit CameraFreeFlyController(std::unique_ptr<Camera> camera) noexcept;
        ~CameraFreeFlyController();

        void setMoveSpeed(f32 speed);
        f32 getMoveSpeed() const;

        void setRotationSpeed(f32 speed);
        f32 getRotationSpeed() const;

        void update(f32 deltaTime) override;

        void handleInputEventCallback(const v3d::event::InputEventHandler* handler, const event::InputEvent* event) override;

    private:

        const math::float2 m_distanceLimits;

        math::float2       m_deltaRotation;
        f32                m_deltaDistance;
        math::Vector4D     m_direction;

        f32                m_rotationSpeed;
        f32                m_moveSpeed;
        f32                m_accelerationSpeed;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d