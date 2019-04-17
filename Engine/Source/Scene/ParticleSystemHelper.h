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

        void add(u32 particleCount, const core::Vector3D& pos, const core::Vector3D& minVel, const core::Vector3D& maxVel);
        void update(f32 dt);
        void draw();

        const renderer::VertexInputAttribDescription& getVertexInputAttribDesc() const;

        static ParticleSystemHelper* createParticleSystemHelper(renderer::CommandList& list);

    private:

        renderer::CommandList& m_cmdList;

        std::vector<std::pair<ParticleSystem*, renderer::VertexStreamBuffer*>> m_particleSystems;
        renderer::VertexInputAttribDescription m_attributeDesc;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace scene
} //namespace v3d
