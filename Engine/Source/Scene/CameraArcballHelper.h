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
    * CameraArcballHelper
    */
    class CameraArcballHelper final : public CameraHelper
    {
    public:

        CameraArcballHelper() = delete;
        CameraArcballHelper(const CameraArcballHelper&) = delete;

        explicit CameraArcballHelper(Camera* camera, f32 distance) noexcept;
        explicit CameraArcballHelper(Camera* camera, f32 distance, f32 minDistance, f32 maxDistance) noexcept;
        ~CameraArcballHelper();

        void setRotation(const core::Vector3D& rotation);
        const core::Vector3D& getRotation() const;

        void update(f32 deltaTime) override;

        void handlerMouseCallback(v3d::event::InputEventHandler* handler, const event::MouseInputEvent* event);
        void handlerTouchCallback(v3d::event::InputEventHandler* handler, const event::TouchInputEvent* event);

    private:

        const core::Vector2D m_distanceLimits;

        const f32   k_rotationSpeed = 0.8f;
        const f32   k_zoomSpeed = 1.0f;


        static constexpr f32 k_signZ = -1.0f;

    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d
