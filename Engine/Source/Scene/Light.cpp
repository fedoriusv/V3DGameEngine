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

    /*
    Classic Game Attenuation Model(Constant + Linear + Quadratic)
        |                       Range | Kc | Kl | Kq   |
        | -------------------- - | -- - | ---- | ---- - |
        | Short(~7 units)        | 1.0  | 0.7  | 1.8    |
        | Medium(~13 units)      | 1.0  | 0.35 | 0.44   |
        | Long(~20 units)        | 1.0  | 0.22 | 0.20   |
        | Very long(~50 units)   | 1.0  | 0.09 | 0.032  |
    */

////////////////////////////////////////////////////////////////////////////////////////////////////////////

Light::Light(renderer::Device* device, Type type) noexcept
    : m_header(type)
    , m_device(device)
    , m_color({ 1.f, 1.f, 1.f, 1.f })
    , m_intensity()
    , m_temperature(0)
    , m_attenuation({ 1.f, 0.35f, 0.44, 1.f })
    , m_shadowCaster(true)
    , m_shadowMask(0b00111111)
{
}

Light::Light(renderer::Device* device, const LightHeader& header) noexcept
    : m_header(header)
    , m_device(device)
    , m_color({ 1.f, 1.f, 1.f, 1.f })
    , m_intensity()
    , m_temperature(0)
    , m_attenuation({ 1.f, 0.35f, 0.44, 1.f })
    , m_shadowCaster(true)
    , m_shadowMask(0b00111111)
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
{
    LOG_DEBUG("PointLight::PointLight constructor %llx", this);
}

PointLight::PointLight(renderer::Device* device, const LightHeader& header) noexcept
    : ComponentBase<PointLight, Light>(device, header)
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
    , m_outerAngle(30.0f)
    , m_innerAngle(10.0f)
{
    LOG_DEBUG("SpotLight::SpotLight constructor %llx", this);
}

SpotLight::SpotLight(renderer::Device* device, const LightHeader& header) noexcept
    : ComponentBase<SpotLight, Light>(device, header)
    , m_outerAngle(30.0f)
    , m_innerAngle(10.0f)
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

} //namespace scene
} //namespace v3d
