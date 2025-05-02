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

        explicit CameraArcballHandler(Camera* camera, f32 distance) noexcept;
        explicit CameraArcballHandler(Camera* camera, f32 distance, f32 minDistance, f32 maxDistance) noexcept;
        ~CameraArcballHandler();

        void setRotation(const math::Vector3D& rotation);
        const math::Vector3D& getRotation() const;

        void update(f32 deltaTime) override;

        void handleMouseCallback(v3d::event::InputEventHandler* handler, const event::MouseInputEvent* event);
        void handleTouchCallback(v3d::event::InputEventHandler* handler, const event::TouchInputEvent* event);

    private:

        const math::Vector2D m_distanceLimits;

        const f32   k_rotationSpeed = 0.8f;
        const f32   k_zoomSpeed = 1.0f;


        static constexpr f32 k_signZ = -1.0f;

    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d
