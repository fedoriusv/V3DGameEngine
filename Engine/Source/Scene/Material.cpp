#include "Material.h"

namespace v3d
{
namespace scene
{

Material::Diffuse::Diffuse()
    : _color(core::Vector4D(0.0f))
{
}

void Material::Diffuse::operator>>(stream::Stream * stream)
{

}

void Material::Diffuse::operator<<(const stream::Stream * stream)
{

}

Material::Ambient::Ambient()
    : _color(core::Vector4D(0.0f))
{
}

void Material::Ambient::operator>>(stream::Stream * stream)
{

}

void Material::Ambient::operator<<(const stream::Stream * stream)
{

}

Material::Material() noexcept
{
}

Material::~Material()
{
}

void Material::init(stream::Stream * stream)
{
}

bool Material::load()
{
    return false;
}

} //namespace scene
} //namespace v3d
