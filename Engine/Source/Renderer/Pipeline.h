#pragma once

#include "Common.h"
#include "Utils/Observable.h"
#include "PipelineStateProperties.h"
#include "ShaderProperties.h"
#include "ObjectTracker.h"

namespace v3d
{
namespace renderer
{
    struct ShaderHeader;

    class Context;
    class Shader;
    class PipelineManager;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * Pipeline base class. Render side
    */
    class Pipeline : public RenderObject<Pipeline>, public utils::Observable
    {
    public:
        enum class PipelineType : u32
        {
            PipelineType_Graphic,
            PipelineType_Compute
        };

        struct PipelineGraphicInfo
        {
            GraphicsPipelineStateDescription _pipelineDesc;
            RenderPassDescription            _renderpassDesc;
            ShaderProgramDescription         _programDesc;

            ObjectTracker<Pipeline>*         _tracker;
        };

        explicit Pipeline(PipelineType type) noexcept;
        virtual ~Pipeline();

        virtual bool create(const PipelineGraphicInfo* pipelineInfo) = 0;
        virtual void destroy() = 0;

        PipelineType getType() const;

    protected:

        bool createShader(const Shader* shader);
        virtual bool compileShader(const ShaderHeader* header, const void* source, u32 size) = 0;

    private:

        u64          m_key;
        PipelineType m_pipelineType;

        friend PipelineManager;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * PipelineManager class
    */
    class PipelineManager final : utils::Observer
    {
    public:

        PipelineManager() = delete;
        PipelineManager(const PipelineManager&) = delete;

        explicit PipelineManager(Context* context) noexcept;
        ~PipelineManager();

        Pipeline* acquireGraphicPipeline(const Pipeline::PipelineGraphicInfo& pipelineInfo);
        bool removePipeline(Pipeline* pipeline);
        void clear();

        void handleNotify(utils::Observable* ob) override;

    private:

        Context*                    m_context;
        std::map<u64, Pipeline*>    m_pipelineGraphicList;
        //std::unordered_map<u64, Pipeline*> m_pipelineGraphicList; TODO
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
