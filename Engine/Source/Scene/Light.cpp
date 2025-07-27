#include "Light.h"

namespace v3d
{
namespace scene
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////

Light::Light(Type type) noexcept
    : m_color({ 1.f, 1.f, 1.f })
    , m_intensity()
    , m_temperature(0)
    , m_type(type)
{
}

Light::~Light()
{
}

void Light::setPosition(const math::Vector3D& position)
{
    m_transform.setPosition(position);
}

void Light::setRotation(const math::Vector3D& rotation)
{
    m_transform.setRotation(rotation);
}

void Light::setTransform(const math::Matrix4D& transform)
{
    m_transform.setTransform(transform);
}

bool Light::load(const stream::Stream* stream, u32 offset)
{
    ASSERT(false, "not impl");
    return false;
}

bool Light::save(stream::Stream* stream, u32 offset) const
{
    ASSERT(false, "not impl");
    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

DirectionalLight::DirectionalLight() noexcept
    : Light(Type::DirectionalLight)
    , k_forwardVector({ 0.f, 0.f, 1.f, 1.f })
{
}

DirectionalLight::~DirectionalLight()
{
}

const math::Vector3D DirectionalLight::getDirection() const
{
    return math::Vector3D(m_transform.getTransform()[8], m_transform.getTransform()[9], m_transform.getTransform()[10]).normalize();;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

PointLight::PointLight() noexcept
    : Light(Type::PointLight)
{

}

PointLight::~PointLight()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d
