#pragma once

#include "Common.h"
#include "Utils/Observable.h"

#include "Renderer/PipelineStateProperties.h"
#include "Renderer/ShaderProperties.h"
#include "Renderer/ObjectTracker.h"

namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    struct ShaderHeader;

    class Context;
    class Shader;
    class PipelineManager;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief Pipeline base class. Render side
    */
    class Pipeline : public RenderObject<Pipeline>, public utils::Observable
    {
    public:

        static const RenderPassDescription createCompatibilityRenderPassDescription(const RenderPassDescription& renderpassDesc);

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
                : _tracker(nullptr)
            {
            }

            GraphicsPipelineStateDescription _pipelineDesc;
            RenderPassDescription            _renderpassDesc;
            ShaderProgramDescription         _programDesc;
            ObjectTracker<Pipeline>*         _tracker;
        };

        /**
        * @brief PipelineComputeInfo struct
        */
        struct PipelineComputeInfo
        {
            PipelineComputeInfo() noexcept
                : _tracker(nullptr)
            {
            }

            ShaderProgramDescription _programDesc;
            ObjectTracker<Pipeline>* _tracker;
        };

        PipelineType getType() const;

    protected:

        explicit Pipeline(PipelineType type) noexcept;
        virtual ~Pipeline();

        virtual bool create(const PipelineGraphicInfo* pipelineInfo) = 0;
        virtual bool create(const PipelineComputeInfo* pipelineInfo) = 0;

        virtual void destroy() = 0;

        bool createShader(const Shader* shader);
        bool createProgram(const ShaderProgramDescription& desc);
        virtual bool compileShader(const ShaderHeader* header, const void* source, u32 size);
        virtual bool compileShaders(std::vector<std::tuple<const ShaderHeader*, const void*, u32>>& shaders);

    private:

        struct PipelineDescription
        {
            PipelineDescription() noexcept;
            explicit PipelineDescription(const PipelineGraphicInfo& pipelineInfo) noexcept;
            explicit PipelineDescription(const PipelineComputeInfo& pipelineInfo) noexcept;
            ~PipelineDescription() = default;

            struct Hash
            {
                u64 operator()(const PipelineDescription& desc) const;
            };

            struct Compare
            {
                bool operator()(const PipelineDescription& op1, const PipelineDescription& op2) const;
            };

            GraphicsPipelineStateDescription _pipelineDesc;
            RenderPassDescription            _renderpassDesc;
            ShaderProgramDescription         _programDesc;

            PipelineType _pipelineType;
            u64 _hash;
            ;
        };

        PipelineDescription m_desc;
        PipelineType m_pipelineType;

        friend PipelineManager;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief PipelineManager class
    */
    class PipelineManager final : utils::Observer
    {
    public:

        PipelineManager() = delete;
        PipelineManager(const PipelineManager&) = delete;

        explicit PipelineManager(Context* context) noexcept;
        ~PipelineManager();

        Pipeline* acquireGraphicPipeline(const Pipeline::PipelineGraphicInfo& pipelineInfo);
        Pipeline* acquireComputePipeline(const Pipeline::PipelineComputeInfo& pipelineInfo);

        bool removePipeline(Pipeline* pipeline);
        void clear();

        void handleNotify(const utils::Observable* ob) override;

    private:

        Context* const m_context;
        std::unordered_map<Pipeline::PipelineDescription, Pipeline*, Pipeline::PipelineDescription::Hash, Pipeline::PipelineDescription::Compare> m_pipelineGraphicList;
        std::unordered_map<Pipeline::PipelineDescription, Pipeline*, Pipeline::PipelineDescription::Hash, Pipeline::PipelineDescription::Compare> m_pipelineComputeList;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d