#include "D3D12Image.h"
#include "Utils/Logger.h"

#ifdef D3D_RENDER
#   include "D3D12GraphicContext.h"
#   include "D3D12CommandList.h"
#   include "D3D12CommandListManager.h"

namespace v3d
{
namespace renderer
{
namespace d3d12
{

 D3DImage::D3DImage(DXGI_FORMAT format, u32 width, u32 height) noexcept
     : Image()
     , m_imageResource(nullptr)
     , m_state(D3D12_RESOURCE_STATE_COMMON)

     , m_format(format)
     , m_size(width, height, 1)

     , m_swapchain(false)
 {
     LOG_DEBUG("D3DImage::D3DImage constructor %llx", this);
 }


 D3DImage::~D3DImage()
 {
     LOG_DEBUG("D3DImage::~D3DImage destructor %llx", this);
     ASSERT(!m_imageResource, "not nullptr");
 }

 bool D3DImage::create()
 {
     ASSERT(false, "not impl");
     return false;
 }

 bool D3DImage::create(ID3D12Resource* resource, CD3DX12_CPU_DESCRIPTOR_HANDLE& handle)
 {
     ASSERT(resource, "nullptr");
     m_imageResource = resource;
     m_handle = handle;
     m_swapchain = true;

     return true;
 }

 void D3DImage::destroy()
 {
     if (m_swapchain)
     {
         m_imageResource = nullptr;
         return;
     }
 }

 void D3DImage::clear(Context* context, const core::Vector4D& color)
 {
     D3DCommandList* commandlist = static_cast<D3DGraphicContext*>(context)->getCurrentCommandList();
     ASSERT(commandlist, "nullptr");

     const FLOAT dxClearColor[] = { color.x, color.y, color.z, color.w };
     const D3D12_RECT dxRect =
     {
         0,
         0,
         static_cast<LONG>(m_size.width),
         static_cast<LONG>(m_size.height)
     };

     static_cast<D3DGraphicsCommandList*>(commandlist)->clearRenderTarget(this, dxClearColor, { dxRect });
 }

 void D3DImage::clear(Context* context, f32 depth, u32 stencil)
 {
     D3DCommandList* commandlist = static_cast<D3DGraphicContext*>(context)->getCurrentCommandList();
     ASSERT(commandlist, "nullptr");

     D3D12_CLEAR_FLAGS flags = D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL;
     const D3D12_RECT dxRect =
     {
         0,
         0,
         static_cast<LONG>(m_size.width),
         static_cast<LONG>(m_size.height)
     };

     static_cast<D3DGraphicsCommandList*>(commandlist)->clearRenderTarget(this, depth, stencil, flags, { dxRect });
 }

 bool D3DImage::upload(Context* context, const core::Dimension3D& size, u32 layers, u32 mips, const void* data)
 {
     ASSERT(false, "not impl");
     return false;
 }

 bool D3DImage::upload(Context* context, const core::Dimension3D& offsets, const core::Dimension3D& size, u32 layers, const void* data)
 {
     ASSERT(false, "not impl");
     return false;
 }

 const core::Dimension3D& D3DImage::getSize() const
 {
     return m_size;
 }

 D3D12_RESOURCE_STATES D3DImage::getState() const
 {
     return m_state;
 }

 D3D12_RESOURCE_STATES D3DImage::setState(D3D12_RESOURCE_STATES state)
 {
     D3D12_RESOURCE_STATES oldState = std::exchange(m_state, state);
     return oldState;
 }

 const CD3DX12_CPU_DESCRIPTOR_HANDLE& D3DImage::getDescriptorHandle() const
 {
     return m_handle;
 }

 ID3D12Resource* D3DImage::getResource() const
 {
     ASSERT(m_imageResource, "nullptr");
     return m_imageResource.Get();
 }

} //namespace d3d12
} //namespace renderer
} //namespace v3d12
#endif //D3D_RENDER
