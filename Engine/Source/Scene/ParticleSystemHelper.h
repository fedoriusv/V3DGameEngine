#pragma once

#include "Common.h"
#include "Renderer/CommandList.h"

namespace v3d
{
namespace scene
{
    class ParticleSystem;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    class ParticleSystemHelper final
    {
    public:

        ParticleSystemHelper() = delete;
        ParticleSystemHelper(const ParticleSystemHelper&) = delete;

        ParticleSystemHelper(renderer::CommandList& list);
        ~ParticleSystemHelper();

        void add(u32 particleCount, const math::Vector3D& pos, const math::Vector3D& minVel, const math::Vector3D& maxVel);
        void update(f32 dt);
        void draw();

        const renderer::VertexInputAttributeDescription& getVertexInputAttribDesc() const;

        static ParticleSystemHelper* createParticleSystemHelper(renderer::CommandList& list);

    private:

        renderer::CommandList& m_cmdList;

        std::vector<std::pair<ParticleSystem*, renderer::VertexStreamBuffer*>> m_particleSystems;
        renderer::VertexInputAttributeDescription m_attributeDesc;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d
