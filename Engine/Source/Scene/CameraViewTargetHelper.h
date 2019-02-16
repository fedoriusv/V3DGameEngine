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
        ~CameraArcballHelper();

        void setRotation(const core::Vector3D& rotation);
        const core::Vector3D& getRotation() const;

        virtual void update();

        void handlerCallback(v3d::event::InputEventHandler* handler, const event::MouseInputEvent* event);

    private:

        core::Vector3D  m_rotate;

        const f32       k_rotationSpeed = 0.8f;
        const f32       k_zoomSpeed = 1.0f;

    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d
