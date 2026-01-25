#include "Light.h"
#include "Scene/Geometry/Mesh.h"
#include "Renderer/Device.h"
#include "Renderer/Buffer.h"
#include "Utils/Logger.h"
#include "Stream/Stream.h"

namespace v3d
{
namespace scene
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////

Light::Light(renderer::Device* device, Type type) noexcept
    : m_header(type)
    , m_device(device)
    , m_color({ 1.f, 1.f, 1.f, 1.f })
    , m_intensity()
    , m_temperature(0)
    , m_attenuation({ 1.f, 1.f, 1.f, 0.f})
    , m_shadowCaster(true)
{
}

Light::Light(renderer::Device* device, const LightHeader& header) noexcept
    : m_header(header)
    , m_device(device)
    , m_color({ 1.f, 1.f, 1.f, 1.f })
    , m_intensity()
    , m_temperature(0)
    , m_attenuation({ 1.f, 1.f, 1.f, 0.f })
    , m_shadowCaster(true)
{
}

Light::~Light()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

DirectionalLight::DirectionalLight(renderer::Device* device) noexcept
    : ComponentBase<DirectionalLight, Light>(device, Type::DirectionalLight)
{
    LOG_DEBUG("DirectionalLight::DirectionalLight constructor %llx", this);
}

DirectionalLight::DirectionalLight(renderer::Device* device, const LightHeader& header) noexcept
    : ComponentBase<DirectionalLight, Light>(device, header)
{
    ASSERT(header.getResourceSubType<Type>() == Type::DirectionalLight, "must be DirectionalLight");
    LOG_DEBUG("DirectionalLight::DirectionalLight constructor %llx", this);
}

DirectionalLight::~DirectionalLight()
{
    LOG_DEBUG("DirectionalLight::DirectionalLight destructor %llx", this);
}

bool DirectionalLight::load(const stream::Stream* stream, u32 offset)
{
    if (m_loaded)
    {
        LOG_WARNING("DirectionalLight::load: the light %llx is already loaded", this);
        return true;
    }

    ASSERT(stream, "nullptr");
    stream->seekBeg(offset);
    ASSERT(offset == m_header._offset, "wrong offset");

    stream->read<color::ColorRGBAF>(m_color);
    stream->read<f32>(m_intensity);
    stream->read<f32>(m_temperature);
    stream->read<math::float4>(m_attenuation);
    stream->read<bool>(m_shadowCaster);

    m_loaded = true;
    return true;
}

bool DirectionalLight::save(stream::Stream* stream, u32 offset) const
{
    ASSERT(false, "not impl");
    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

PointLight::PointLight(renderer::Device* device) noexcept
    : ComponentBase<PointLight, Light>(device, Type::PointLight)
    , m_radius(1.f)
{
    LOG_DEBUG("PointLight::PointLight constructor %llx", this);
}

PointLight::PointLight(renderer::Device* device, const LightHeader& header) noexcept
    : ComponentBase<PointLight, Light>(device, header)
    , m_radius(1.f)
{
    ASSERT(header.getResourceSubType<Type>() == Type::PointLight, "must be PointLight");
    LOG_DEBUG("PointLight::PointLight constructor %llx", this);
}

PointLight::~PointLight()
{
    LOG_DEBUG("PointLight::PointLight destructor %llx", this);
}

bool PointLight::load(const stream::Stream* stream, u32 offset)
{
    if (m_loaded)
    {
        LOG_WARNING("PointLight::load: the light %llx is already loaded", this);
        return true;
    }

    ASSERT(stream, "nullptr");
    stream->seekBeg(offset);
    ASSERT(offset == m_header._offset, "wrong offset");

    stream->read<color::ColorRGBAF>(m_color);
    stream->read<f32>(m_intensity);
    stream->read<f32>(m_temperature);
    stream->read<math::float4>(m_attenuation);
    stream->read<bool>(m_shadowCaster);
    stream->read<f32>(m_radius);

    m_loaded = true;
    return true;
}

bool PointLight::save(stream::Stream* stream, u32 offset) const
{
    ASSERT(false, "not impl");
    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

SpotLight::SpotLight(renderer::Device* device) noexcept
    : ComponentBase<SpotLight, Light>(device, Type::SpotLight)
    , m_innerAngle(0.0f)
    , m_outerAngle(0.0f)
{
    LOG_DEBUG("SpotLight::SpotLight constructor %llx", this);
}

SpotLight::SpotLight(renderer::Device* device, const LightHeader& header) noexcept
    : ComponentBase<SpotLight, Light>(device, header)
    , m_innerAngle(0.0f)
    , m_outerAngle(0.0f)
{
    ASSERT(header.getResourceSubType<Type>() == Type::SpotLight, "must be SpotLight");
    LOG_DEBUG("SpotLight::SpotLight constructor %llx", this);
}

SpotLight::~SpotLight()
{
    LOG_DEBUG("SpotLight::SpotLight destructor %llx", this);
}

bool SpotLight::load(const stream::Stream* stream, u32 offset)
{
    if (m_loaded)
    {
        LOG_WARNING("SpotLight::load: the light %llx is already loaded", this);
        return true;
    }

    ASSERT(stream, "nullptr");
    stream->seekBeg(offset);
    ASSERT(offset == m_header._offset, "wrong offset");

    stream->read<color::ColorRGBAF>(m_color);
    stream->read<f32>(m_intensity);
    stream->read<f32>(m_temperature);
    stream->read<math::float4>(m_attenuation);
    stream->read<bool>(m_shadowCaster);
    stream->read<f32>(m_innerAngle);
    stream->read<f32>(m_outerAngle);

    m_loaded = true;
    return true;
}

bool SpotLight::save(stream::Stream* stream, u32 offset) const
{
    ASSERT(false, "not impl");
    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

DirectionalLight* LightHelper::createDirectionLight(renderer::Device* device, const std::string& name)
{
    DirectionalLight* light = V3D_NEW(DirectionalLight, memory::MemoryLabel::MemoryObject)(device);
    light->m_header.setName(name);

    return light;
}

PointLight* LightHelper::createPointLight(renderer::Device* device, f32 radius, const std::string& name)
{
    PointLight* light = V3D_NEW(PointLight, memory::MemoryLabel::MemoryObject)(device);
    light->m_header.setName(name);
    light->setRadius(radius);

    return light;
}

SpotLight* LightHelper::createSpotLight(renderer::Device* device, const std::string& name)
{
    SpotLight* light = V3D_NEW(SpotLight, memory::MemoryLabel::MemoryObject)(device);
    light->m_header.setName(name);

    return light;
}

} //namespace scene
} //namespace v3d
