#pragma once

#include "Common.h"

#ifdef D3D_RENDER
#if (D3D_VERSION_MAJOR == 12)

#   include <d3d12.h>
#   include <dxgi1_4.h>
#   include "d3dx12.h"

//#ifdef PLATFORM_WINDOWS
//#   include <wrl/client.h>
//using namespace Microsoft::WRL;
//#endif

#if DEBUG
#   define SAFE_DELETE(comp)                \
        if (comp)                           \
        {                                   \
            u32 count = comp->Release();    \
            ASSERT(count == 0, "not free"); \
            comp = nullptr;                 \
        }
#else
#   define SAFE_DELETE(comp)    \
        if (comp)               \
        {                       \
            comp->Release();    \
            comp = nullptr;     \
        }
#endif //DEBUG

namespace v3d
{
namespace renderer
{
namespace dx3d
{

} //namespace dx3d
} //namespace renderer
} //namespace v3d

#else
#   error "DirectX version is not supported"
#endif //(D3D_VERSION_MAJOR == 12)
#endif //D3D_RENDER
