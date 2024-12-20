#pragma once

#include "Common.h"
#include "CameraHelper.h"

#include "Events/InputEventHandler.h"

namespace v3d
{
namespace scene
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief CameraArcballHelper class
    */
    class CameraArcballHelper final : public CameraHelper
    {
    public:

        CameraArcballHelper() = delete;
        CameraArcballHelper(const CameraArcballHelper&) = delete;

        explicit CameraArcballHelper(Camera* camera, f32 distance) noexcept;
        explicit CameraArcballHelper(Camera* camera, f32 distance, f32 minDistance, f32 maxDistance) noexcept;
        ~CameraArcballHelper();

        void setRotation(const math::Vector3D& rotation);
        const math::Vector3D& getRotation() const;

        void update(f32 deltaTime) override;

        void handlerMouseCallback(v3d::event::InputEventHandler* handler, const event::MouseInputEvent* event);
        void handlerTouchCallback(v3d::event::InputEventHandler* handler, const event::TouchInputEvent* event);

    private:

        const math::Vector2D m_distanceLimits;

        const f32   k_rotationSpeed = 0.8f;
        const f32   k_zoomSpeed = 1.0f;


        static constexpr f32 k_signZ = -1.0f;

    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d
