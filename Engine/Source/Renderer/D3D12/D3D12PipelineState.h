#pragma once

#include "Common.h"
#include "Renderer/Pipeline.h"

#ifdef D3D_RENDER
#   include "D3D12Configuration.h"

namespace v3d
{
namespace renderer
{
namespace dx3d
{

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * D3DGraphicPipelineState final class. DirectX Render side
    */
    class D3DGraphicPipelineState final : public Pipeline
    {
    public:

        explicit D3DGraphicPipelineState(ID3D12Device* device) noexcept;
        ~D3DGraphicPipelineState();

        bool create(const PipelineGraphicInfo* pipelineInfo) override;
        void destroy() override;

        ID3D12PipelineState* getHandle() const;

    private:

        ID3D12Device* m_device;
        ID3D12PipelineState* m_pipelineState;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace dx3d
} //namespace renderer
} //namespace v3d

#endif //D3D_RENDER
