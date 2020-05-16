#pragma once

#include "Common.h"
#include "Renderer/RenderPass.h"
#include "Renderer/Framebuffer.h"

#ifdef D3D_RENDER
#include "D3DConfiguration.h"
#include "D3DRenderResource.h"

namespace v3d
{
namespace renderer
{
namespace dx3d
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class D3DDescriptorHeap;
    class D3DDescriptorHeapManager;

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * D3DRenderState final class. DirectX Render side
    */
    class D3DRenderState final : public RenderPass, public D3DRenderResource
    {
    public:

        explicit D3DRenderState(const RenderPassDescription& desc) noexcept;
        ~D3DRenderState() = default;

        bool create() override;
        void destroy() override;

        const RenderPassDescription& getDescription() const;

    private:

        RenderPassDescription m_renderStateInfo;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * D3DRenderTarget final class. DirectX Render side
    */
    class D3DRenderTarget final : public Framebuffer, public D3DRenderResource
    {
    public:

        explicit D3DRenderTarget(ID3D12Device* device, D3DDescriptorHeapManager* manager, const std::vector<Image*>& images) noexcept;
        ~D3DRenderTarget();

        bool create(const RenderPass* pass) override;
        void destroy() override;

        const std::vector<D3D12_CPU_DESCRIPTOR_HANDLE>& getColorDescHandles() const;
        const D3D12_CPU_DESCRIPTOR_HANDLE& getDepthStensilDescHandles() const;

        const RenderPassDescription& getDescription() const;
        const std::vector<Image*>& getImages() const;

    private:

        ID3D12Device* const m_device;
        D3DDescriptorHeapManager* const m_heapManager;
        const D3DRenderState* m_renderState;

        std::vector<Image*> m_images;

        D3DDescriptorHeap* m_colorDescriptorHeap;
        std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_colorRenderTargets;

        D3DDescriptorHeap* m_depthStencilDescriptorHeap;
        D3D12_CPU_DESCRIPTOR_HANDLE m_depthStensilRenderTarget;
    };

/////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace dx3d
} //namespace renderer
} //namespace v3d
#endif //D3D_RENDER
