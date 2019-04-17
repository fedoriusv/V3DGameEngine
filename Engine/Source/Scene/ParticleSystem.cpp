#include "ParticleSystem.h"

namespace v3d
{
namespace scene
{

inline f32 rnd(f32 range)
{
    return range * f32(rand() / f64(RAND_MAX));
}

ParticleSystem::ParticleSystem(u32 particleCount, const core::Vector3D & pos, const core::Vector3D & minvel, const core::Vector3D & maxvel)
    : m_position(pos)
    , m_minVel(minvel)
    , m_maxVel(maxvel)
{
    m_particles.resize(particleCount);
    for (auto& particle : m_particles)
    {
        init(&particle, m_position);
    }
}

ParticleSystem::~ParticleSystem()
{
}

void ParticleSystem::init(Particle *particle, const core::Vector3D& emitterPos)
{
    particle->_vel = core::Vector4D(0.0f, m_minVel.y + rnd(m_maxVel.y - m_minVel.y), 0.0f, 0.0f);
    particle->_alpha = rnd(0.75f);
    particle->_size = 1.0f + rnd(0.5f);
    particle->_size *= 0.5f;
    particle->_color = core::Vector4D(1.0f);
    particle->_type = ParticleType_Flame;
    particle->_rotation = rnd(2.0f * core::k_pi);
    particle->_rotationSpeed = rnd(2.0f) - rnd(2.0f);

    // Get random sphere point
    f32 theta = rnd(2 * core::k_pi);
    f32 phi = rnd(core::k_pi) - core::k_pi / 2;
    f32 r = rnd(m_radius);

    particle->_position.x = r * cos(theta) * cos(phi);
    particle->_position.y = r * sin(phi);
    particle->_position.z = r * sin(theta) * cos(phi);

    particle->_position += core::Vector4D(emitterPos, 0.0f);

    particle->_position.w = rnd(16.0f);
}

void ParticleSystem::transition(Particle *particle)
{
    switch (particle->_type)
    {
    case ParticleType_Flame:
        // Flame particles have a chance of turning into smoke
        if (rnd(1.0f) < 0.015f)
        {
            particle->_alpha = 0.0f;
            particle->_color = core::Vector4D(0.15f + rnd(0.25f));
            particle->_position.x = m_position.x + (particle->_position.x - m_position.x) * 0.5f;
            particle->_position.z = m_position.z + (particle->_position.z - m_position.z) * 0.5f;
            particle->_position.w = rnd(16.0f);
            particle->_vel = core::Vector4D(rnd(1.0f) - rnd(1.0f), (m_minVel.y * 2) + rnd(m_maxVel.y - m_minVel.y), rnd(1.0f) - rnd(1.0f), 0.0f);
            particle->_size = 1.0f + rnd(0.5f);
            particle->_rotationSpeed = rnd(1.0f) - rnd(1.0f);
            particle->_type = ParticleType_Smoke;
        }
        else
        {
            init(particle, m_position);
        }
        break;

    case ParticleType_Smoke:
        // Respawn at end of life
        init(particle, m_position);
        break;
    }
}

void ParticleSystem::update(f32 dt)
{
    f32 particleTimer = dt * 0.45f;
    for (auto& particle : m_particles)
    {
        switch (particle._type)
        {
        case ParticleType_Flame:
            particle._position.y -= particle._vel.y * particleTimer * 3.5f;
            particle._alpha += particleTimer * 2.5f;
            particle._size -= particleTimer * 0.5f;
            break;

        case ParticleType_Smoke:
            particle._position -= particle._vel * dt * 1.0f;
            particle._alpha += particleTimer * 1.25f;
            particle._size += particleTimer * 0.125f;
            particle._color -= particleTimer * 0.05f;
            break;
        }
        particle._rotation += particleTimer * particle._rotationSpeed;
        // Transition particle state
        if (particle._alpha > 2.0f)
        {
            transition(&particle);
        }
    }
}

u32 ParticleSystem::getCount() const
{
    return static_cast<u32>(m_particles.size());
}

void* ParticleSystem::getData()
{
    return m_particles.data();
}

} //namespace scene
} //namespace v3d
