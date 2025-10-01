#include "Transform.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace scene
{

Transform::Transform() noexcept
    : m_position(0.f, 0.f, 0.f)
    , m_rotation(0.f, 0.f, 0.f)
    , m_scale(1.f, 1.f, 1.f)

    , m_transformFlag(0/*TransformState::TransformState_All*/)
{
}

Transform::~Transform()
{
}

void Transform::setPosition(const math::Vector3D& position)
{
    m_position = position;
    m_transformFlag |= TransformState::TransformState_Translation;
}

void Transform::setRotation(const math::Vector3D& rotation)
{
    m_rotation = rotation;
    m_transformFlag |= TransformState::TransformState_Rotation;
}

void Transform::setScale(const math::Vector3D& scale)
{
    m_scale = scale;
    m_transformFlag |= TransformState::TransformState_Scale;
}

void Transform::setMatrix(const math::Matrix4D& transform)
{
    m_modelMatrix = transform;

    m_position = m_modelMatrix.getTranslation();
    m_rotation = m_modelMatrix.getRotation();
    m_scale = m_modelMatrix.getScale();
    m_transformFlag &= ~TransformState::TransformState_All;
}

void Transform::claculateTransform() const
{
    if (m_transformFlag & TransformState::TransformState_Scale)
    {
        m_modelMatrix.setScale(m_scale);
        m_transformFlag &= ~TransformState::TransformState_Scale;
    }

    if (m_transformFlag & TransformState::TransformState_Rotation)
    {
        m_modelMatrix.setRotation(m_rotation);
        m_transformFlag &= ~TransformState::TransformState_Rotation;
    }

    if (m_transformFlag & TransformState::TransformState_Translation)
    {
        m_modelMatrix.setTranslation(m_position);
        m_transformFlag &= ~TransformState::TransformState_Translation;
    }
}

} //namespace scene
} //namespace v3d
