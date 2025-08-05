#include "Light.h"
#include "Scene/Geometry/Mesh.h"
#include "Renderer/Device.h"
#include "Renderer/Buffer.h"
#include "Utils/Logger.h"

namespace v3d
{
namespace scene
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////

Light::Light(Type type) noexcept
    : m_color({ 1.f, 1.f, 1.f })
    , m_type(type)
    , m_intensity()
    , m_temperature(0)
    , m_volume(nullptr)
{
}

Light::~Light()
{
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
{
    LOG_DEBUG("DirectionalLight::DirectionalLight constructor %llx", this);
}

DirectionalLight::~DirectionalLight()
{
    LOG_DEBUG("DirectionalLight::DirectionalLight destructor %llx", this);
}

const math::Vector3D DirectionalLight::getDirection() const
{
    return math::Vector3D(m_transform.getTransform()[8], m_transform.getTransform()[9], m_transform.getTransform()[10]).normalize();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

PointLight::PointLight() noexcept
    : Light(Type::PointLight)
    , m_attenuation(1.f)
{
    LOG_DEBUG("PointLight::PointLight constructor %llx", this);
}

PointLight::~PointLight()
{
    LOG_DEBUG("PointLight::PointLight destructor %llx", this);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

SpotLight::SpotLight() noexcept
    : Light(Type::SpotLight)
{
    LOG_DEBUG("SpotLight::SpotLight constructor %llx", this);
}

SpotLight::~SpotLight()
{
    LOG_DEBUG("SpotLight::SpotLight destructor %llx", this);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

PointLight* LightHelper::createPointLight(renderer::Device* device, renderer::CmdListRender* cmdList, f32 radius, const std::string& name)
{
    PointLight* light = V3D_NEW(PointLight, memory::MemoryLabel::MemoryObject)();
    light->m_volume = scene::MeshHelper::createSphere(device, cmdList, radius, 64, 64, name);
    light->m_name = name;

    return light;
}

SpotLight* LightHelper::createSpotLight(renderer::Device* device, renderer::CmdListRender* cmdList, const std::string& name)
{
    return nullptr;
}

} //namespace scene
} //namespace v3d
