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
    * @brief CameraArcballHandler class
    */
    class CameraArcballHandler final : public CameraHandler
    {
    public:

        CameraArcballHandler() = delete;
        CameraArcballHandler(const CameraArcballHandler&) = delete;

        explicit CameraArcballHandler(std::unique_ptr<Camera> camera, f32 distance) noexcept;
        explicit CameraArcballHandler(std::unique_ptr<Camera> camera, f32 distance, f32 minDistance, f32 maxDistance) noexcept;
        ~CameraArcballHandler();

        void update(f32 deltaTime) override;

        void handleMouseCallback(v3d::event::InputEventHandler* handler, const event::MouseInputEvent* event);
        void handleTouchCallback(v3d::event::InputEventHandler* handler, const event::TouchInputEvent* event);

    private:

        const math::float2 m_distanceLimits;

        math::Vector3D     m_rotation;
        math::float2       m_deltaRotation;

        f32                m_distance;
        f32                m_deltaDistance;

        f32                m_rotationSpeed = 0.5f;
        f32                m_zoomSpeed = 5.0f;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d
