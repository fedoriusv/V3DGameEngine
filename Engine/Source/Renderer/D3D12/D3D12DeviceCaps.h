#pragma once

#include "Renderer/DeviceCaps.h"
#include "Utils/Singleton.h"

#ifdef D3D_RENDER

namespace v3d
{
namespace renderer
{
namespace dx3d
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * D3DDeviceCaps struct. Vulkan Render side
    */
    struct D3DDeviceCaps : DeviceCaps, utils::Singleton<D3DDeviceCaps>
    {
        D3DDeviceCaps() = default;

        D3DDeviceCaps(const D3DDeviceCaps&) = delete;
        D3DDeviceCaps& operator=(const D3DDeviceCaps&) = delete;

    private:

        friend class D3DGraphicContext;

        void initialize();
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace dx3d
} //namespace renderer
} //namespace v3d
#endif //VULKAN_RENDER
