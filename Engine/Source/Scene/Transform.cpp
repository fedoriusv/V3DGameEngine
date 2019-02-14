#include "Transform.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace scene
{

Transform::Transform() noexcept
    : m_position(core::Vector3D(0.f))
    , m_rotation(core::Vector3D(0.f))
    , m_scale(core::Vector3D(1.f))

    , m_transformFlag(0/*TransformState::TransformState_All*/)
{
    LOG_DEBUG("Transform constructor %xll", this);
}

Transform::~Transform()
{
    LOG_DEBUG("Camera destructor %xll", this);
}

void Transform::setPosition(const core::Vector3D& position)
{
    m_position = position;
    m_transformFlag |= TransformState::TransformState_Translation;
}

void Transform::setRotation(const core::Vector3D& rotation)
{
    m_rotation = rotation;
    m_transformFlag |= TransformState::TransformState_Rotation;
}

void Transform::setScale(const core::Vector3D& scale)
{
    m_scale = scale;

    bool hasScale = (scale.x != 1.f || scale.y != 1.f || scale.z != 1.f);
    if (hasScale)
    {
        m_transformFlag |= TransformState::TransformState_Scale;
    }
}

void Transform::setTransform(const core::Matrix4D& transform)
{
    m_modelMatrix = transform;

    m_position = m_modelMatrix.getTranslation();
    m_rotation = m_modelMatrix.getRotation();
    m_scale = m_modelMatrix.getScale();
    m_transformFlag &= ~TransformState::TransformState_All;
}

const core::Vector3D& Transform::getPosition() const
{
    return m_position;
}

const core::Vector3D& Transform::getRotation() const
{
    ASSERT(m_transformFlag & ~TransformState::TransformState_Scale && m_transformFlag & ~TransformState::TransformState_Rotation, "not updated");
    return m_rotation;
}

const core::Vector3D& Transform::getScale() const
{
    ASSERT(m_transformFlag & ~TransformState::TransformState_Scale && m_transformFlag & ~TransformState::TransformState_Rotation, "not updated");
    return m_scale;
}

const core::Matrix4D& Transform::getTransform() const
{
    Transform::updateTransform();
    return m_modelMatrix;
}

void Transform::updateTransform() const
{
    if (m_transformFlag & TransformState::TransformState_Rotation)
    {
        m_modelMatrix.setRotation(m_rotation);
        m_transformFlag &= ~TransformState::TransformState_Rotation;

        m_modelMatrix.postScale(m_scale);
        m_transformFlag &= ~TransformState::TransformState_Scale;
    }

    if (m_transformFlag & TransformState::TransformState_Scale)
    {
        m_modelMatrix.setScale(m_scale);
        m_transformFlag &= ~TransformState::TransformState_Scale;
    }

    if (m_transformFlag & TransformState::TransformState_Translation)
    {
        m_modelMatrix.setTranslation(m_position);
        m_transformFlag &= ~TransformState::TransformState_Translation;
    }
}

} //namespace scene
} //namespace v3d
