#pragma once

#include "Common.h"

#ifdef D3D_RENDER
#   include "D3D12Configuration.h"

namespace v3d
{
namespace renderer
{
namespace d3d12
{

/////////////////////////////////////////////////////////////////////////////////////////////////////

    std::string StringError(HRESULT error);

/////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d12
#endif //D3D_RENDER