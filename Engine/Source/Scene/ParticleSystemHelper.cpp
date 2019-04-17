#include "ParticleSystemHelper.h"
#include "ParticleSystem.h"

#include "Renderer/Object/StreamBuffer.h"

namespace v3d
{
namespace scene
{

ParticleSystemHelper::ParticleSystemHelper(renderer::CommandList & list)
    : m_cmdList(list)
{
    m_attributeDesc = renderer::VertexInputAttribDescription(
        { renderer::VertexInputAttribDescription::InputBinding(0, renderer::VertexInputAttribDescription::InputRate::InputRate_Vertex, sizeof(ParticleSystem::Particle)) },
        { 
            renderer::VertexInputAttribDescription::InputAttribute(0, 0, renderer::Format::Format_R32G32B32A32_SFloat, offsetof(ParticleSystem::Particle, _position)),
            renderer::VertexInputAttribDescription::InputAttribute(0, 0, renderer::Format::Format_R32G32B32A32_SFloat, offsetof(ParticleSystem::Particle, _color)),
            renderer::VertexInputAttribDescription::InputAttribute(0, 0, renderer::Format::Format_R32_SFloat, offsetof(ParticleSystem::Particle, _alpha)),
            renderer::VertexInputAttribDescription::InputAttribute(0, 0, renderer::Format::Format_R32_SFloat, offsetof(ParticleSystem::Particle, _size)),
            renderer::VertexInputAttribDescription::InputAttribute(0, 0, renderer::Format::Format_R32_SFloat, offsetof(ParticleSystem::Particle, _rotation)),
            renderer::VertexInputAttribDescription::InputAttribute(0, 0, renderer::Format::Format_R32_SInt, offsetof(ParticleSystem::Particle, _type)),
        }
    );

}

const renderer::VertexInputAttribDescription& ParticleSystemHelper::getVertexInputAttribDesc() const
{
    return m_attributeDesc;
}

void ParticleSystemHelper::add(u32 particleCount, const core::Vector3D& pos, const core::Vector3D& minVel, const core::Vector3D& maxVel)
{
    ParticleSystem* particle = new ParticleSystem(particleCount, pos, minVel, maxVel);
    
    u32 bufferSize = particleCount * sizeof(ParticleSystem::Particle);
    renderer::VertexStreamBuffer* vertexBuffer = m_cmdList.createObject<renderer::VertexStreamBuffer>(renderer::StreamBuffer_Write | renderer::StreamBuffer_Shared, bufferSize, reinterpret_cast<u8*>(particle->getData()));

    m_particleSystems.push_back(std::make_pair(particle, vertexBuffer));
}

void ParticleSystemHelper::update(f32 dt)
{
    for (auto& particle : m_particleSystems)
    {
        particle.first->update(dt);
    }
}

void ParticleSystemHelper::draw()
{
    for (auto& particle : m_particleSystems)
    {
        m_cmdList.draw(renderer::StreamBufferDescription(particle.second, 0), 0, particle.first->getCount(), 1);
    }
}

ParticleSystemHelper * ParticleSystemHelper::createParticleSystemHelper(renderer::CommandList & list)
{
    return new ParticleSystemHelper(list);
}

} //namespace scene
} //namespace v3d
