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
    * @brief CameraOrbitController class
    */
    class CameraOrbitController final : public CameraController
    {
    public:

        CameraOrbitController() = delete;
        CameraOrbitController(const CameraOrbitController&) = delete;

        explicit CameraOrbitController(std::unique_ptr<Camera> camera, f32 distance) noexcept;
        explicit CameraOrbitController(std::unique_ptr<Camera> camera, f32 distance, f32 minDistance, f32 maxDistance) noexcept;
        ~CameraOrbitController();

        void update(f32 deltaTime) override;

        void handleInputEventCallback(const event::InputEventHandler* handler, const event::InputEvent* event) override;

    private:

        void handleMouseCallback(const event::InputEventHandler* handler, const event::MouseInputEvent* event);
        void handleTouchCallback(const event::InputEventHandler* handler, const event::TouchInputEvent* event);

        const math::float2 m_distanceLimits;

        math::float2       m_deltaRotation;
        f32                m_distance;
        f32                m_deltaDistance;

        f32                m_rotationSpeed = 0.5f;
        f32                m_zoomSpeed = 5.0f;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d
