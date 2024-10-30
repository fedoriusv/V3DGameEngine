#pragma once

#include "Render.h"
#include "PipelineState.h"
#include "RenderTargetState.h"

namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief RenderPipeline base class. Render side
    */
    class RenderPipeline : public RenderObject<RenderPipeline>
    {
    public:

        [[nodiscard]] static const RenderPassDesc createCompatibilityRenderPassDescription(const RenderPassDesc& renderpassDesc);

        /**
        * @brief Pipeline PipelineType enum
        */
        enum class PipelineType : u32
        {
            PipelineType_Graphic,
            PipelineType_Compute
        };

        PipelineType getType() const;

    protected:

        explicit RenderPipeline(PipelineType type) noexcept;
        virtual ~RenderPipeline();

    private:

        RenderPipeline() = delete;
        RenderPipeline(const RenderPipeline&) = delete;

        PipelineType m_pipelineType;
    };

    inline RenderPipeline::PipelineType RenderPipeline::getType() const
    {
        return m_pipelineType;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
