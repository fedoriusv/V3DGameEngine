#pragma once

#include "Common.h"
#include "CameraHelper.h"

#include "Event/InputEventMouse.h"

namespace v3d
{
namespace scene
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * CameraViewTargetHelper
    */
    class CameraViewTargetHelper final : public CameraHelper
    {
    public:

        CameraViewTargetHelper() = delete;
        CameraViewTargetHelper(const CameraViewTargetHelper&) = delete;

        explicit CameraViewTargetHelper(Camera* camera, const core::Vector3D& position) noexcept;
        ~CameraViewTargetHelper();

        void setRotation(const core::Vector3D& rotation);
        const core::Vector3D& getRotation() const;

        virtual void update();

        void rotateHandler(const event::MouseInputEvent* event);

    private:

        core::Vector3D  m_rotate;
        bool            m_needUpdate;

    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d
