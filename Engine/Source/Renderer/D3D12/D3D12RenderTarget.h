#pragma once

#include "Common.h"
#include "Renderer/Framebuffer.h"

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
    * D3DRenderTarget final class. DirectX Render side
    */
    class D3DRenderTarget final : public Framebuffer
    {
    public:

        explicit D3DRenderTarget();
        ~D3DRenderTarget();

        bool create(const RenderPass* pass) override;
        void destroy() override;

    };

/////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace dx3d
} //namespace renderer
} //namespace v3d
#endif //D3D_RENDER
