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

        [[nodiscard]] static const RenderPassDescription createCompatibilityRenderPassDescription(const RenderPassDescription& renderpassDesc);

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
            [[maybe_unused]] std::string     _name;
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

            ShaderProgramDescription        _programDesc;
            ObjectTracker<Pipeline>*        _tracker;
            [[maybe_unused]] std::string    _name;
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

        Pipeline() = delete;
        Pipeline(const Pipeline&) = delete;

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

        explicit PipelineManager(Context* context) noexcept;
        ~PipelineManager();

        [[nodiscard]] Pipeline* acquireGraphicPipeline(const Pipeline::PipelineGraphicInfo& pipelineInfo);
        [[nodiscard]] Pipeline* acquireComputePipeline(const Pipeline::PipelineComputeInfo& pipelineInfo);

        bool removePipeline(Pipeline* pipeline);
        void clear();

        void handleNotify(const utils::Observable* object, void* msg) override;

    private:

        PipelineManager() = delete;
        PipelineManager(const PipelineManager&) = delete;

        Context* const m_context;
        std::unordered_map<Pipeline::PipelineDescription, Pipeline*, Pipeline::PipelineDescription::Hash, Pipeline::PipelineDescription::Compare> m_pipelineGraphicList;
        std::unordered_map<Pipeline::PipelineDescription, Pipeline*, Pipeline::PipelineDescription::Hash, Pipeline::PipelineDescription::Compare> m_pipelineComputeList;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
