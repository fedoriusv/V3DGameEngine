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
    class RenderPipeline
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

        /**
        * @brief PipelineGraphicInfo struct
        */
        struct PipelineGraphicInfo
        {
            PipelineGraphicInfo() noexcept
                //    : _tracker(nullptr)
            {
            }

            GraphicsPipelineStateDesc       _pipelineDesc;
            RenderPassDesc                  _renderpassDesc;
            //ShaderProgramDescription        _programDesc;
            //ObjectTracker<Pipeline>*        _tracker;
            std::string                     _name;
        };

        /**
        * @brief PipelineComputeInfo struct
        */
        struct PipelineComputeInfo
        {
            PipelineComputeInfo() noexcept
                //    : _tracker(nullptr)
            {
            }

            //ShaderProgramDescription        _programDesc;
            //ObjectTracker<Pipeline>*        _tracker;
            std::string                     _name;
        };

        PipelineType getType() const;

    protected:

        explicit RenderPipeline(PipelineType type) noexcept;
        virtual ~RenderPipeline();

        virtual bool create(const PipelineGraphicInfo* pipelineInfo) = 0;
        virtual bool create(const PipelineComputeInfo* pipelineInfo) = 0;
        virtual void destroy() = 0;

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
