#pragma once

#include "Common.h"
#include "ParticleSystem.h"

namespace v3d
{
namespace scene
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    class ParticleSystem final
    {
    public:

        enum ParticleType : u32
        {
            ParticleType_Flame,
            ParticleType_Smoke
        };

        struct Particle
        {
            math::Vector4D _position;
            math::Vector4D _color;
            f32            _alpha;
            f32            _size;
            f32            _rotation;
            ParticleType   _type;

            math::Vector4D _vel;
            f32            _rotationSpeed;
        };

        ParticleSystem() = delete;
        ParticleSystem(const ParticleSystem&) = delete;

        ParticleSystem(u32 particleCount, const math::Vector3D& pos, const math::Vector3D& minvel, const math::Vector3D& maxvel);
        ~ParticleSystem();

        void init(Particle *particle, const math::Vector3D& emitterPos);
        void transition(Particle *particle);
        void update(f32 dt);

        u32 getCount() const;
        void* getData();

    private:

        std::vector<Particle> m_particles;

        math::Vector3D        m_position;
        math::Vector3D        m_minVel;
        math::Vector3D        m_maxVel;

        f32                   m_radius = 1.0f;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d
