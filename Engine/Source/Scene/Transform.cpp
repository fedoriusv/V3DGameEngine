#include "Transform.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace scene
{

Transform::Transform() noexcept
    : m_position(math::Vector3D(0.f))
    , m_rotation(math::Vector3D(0.f))
    , m_scale(math::Vector3D(1.f))

    , m_transformFlag(0/*TransformState::TransformState_All*/)
{
    LOG_DEBUG("Transform constructor %llx", this);
}

Transform::~Transform()
{
    LOG_DEBUG("Transform destructor %llx", this);
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

    bool hasScale = (scale.m_x != 1.f || scale.m_y != 1.f || scale.m_z != 1.f);
    if (hasScale)
    {
        m_transformFlag |= TransformState::TransformState_Scale;
    }
}

void Transform::setTransform(const math::Matrix4D& transform)
{
    m_modelMatrix = transform;

    m_position = m_modelMatrix.getTranslation();
    m_rotation = m_modelMatrix.getRotation();
    m_scale = m_modelMatrix.getScale();
    m_transformFlag &= ~TransformState::TransformState_All;
}

const math::Vector3D& Transform::getPosition() const
{
    return m_position;
}

const math::Vector3D& Transform::getRotation() const
{
    //ASSERT(!(m_transformFlag & TransformState::TransformState_Scale) && !(m_transformFlag & TransformState::TransformState_Rotation), "not updated");
    return m_rotation;
}

const math::Vector3D& Transform::getScale() const
{
    //ASSERT(!(m_transformFlag & TransformState::TransformState_Scale) && !(m_transformFlag & TransformState::TransformState_Rotation), "not updated");
    return m_scale;
}

const math::Matrix4D& Transform::getTransform() const
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
