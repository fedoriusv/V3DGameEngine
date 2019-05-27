#pragma once

#include "Common.h"

#ifdef D3D_RENDER
#if (D3D_VERSION_MAJOR == 12)

#   include <d3d12.h>
#   include <dxgi1_4.h>

namespace v3d
{
namespace renderer
{
namespace d3d12
{

/////////////////////////////////////////////////////////////////////////////////////////////////////

struct CD3DX12_DEFAULT {};

struct CD3DX12_CPU_DESCRIPTOR_HANDLE : public D3D12_CPU_DESCRIPTOR_HANDLE
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE() = default;
    explicit CD3DX12_CPU_DESCRIPTOR_HANDLE(const D3D12_CPU_DESCRIPTOR_HANDLE& o) 
        : D3D12_CPU_DESCRIPTOR_HANDLE(o)
    {
    }

    CD3DX12_CPU_DESCRIPTOR_HANDLE(CD3DX12_DEFAULT)
    { 
        ptr = 0;
    }

    CD3DX12_CPU_DESCRIPTOR_HANDLE(_In_ const D3D12_CPU_DESCRIPTOR_HANDLE& other, INT offsetScaledByIncrementSize)
    {
        InitOffsetted(other, offsetScaledByIncrementSize);
    }

    CD3DX12_CPU_DESCRIPTOR_HANDLE(_In_ const D3D12_CPU_DESCRIPTOR_HANDLE& other, INT offsetInDescriptors, UINT descriptorIncrementSize)
    {
        InitOffsetted(other, offsetInDescriptors, descriptorIncrementSize);
    }

    CD3DX12_CPU_DESCRIPTOR_HANDLE& Offset(INT offsetInDescriptors, UINT descriptorIncrementSize)
    {
        ptr += INT64(offsetInDescriptors) * UINT64(descriptorIncrementSize);
        return *this;
    }

    CD3DX12_CPU_DESCRIPTOR_HANDLE& Offset(INT offsetScaledByIncrementSize)
    {
        ptr += offsetScaledByIncrementSize;
        return *this;
    }

    bool operator==(_In_ const D3D12_CPU_DESCRIPTOR_HANDLE& other) const
    {
        return (ptr == other.ptr);
    }

    bool operator!=(_In_ const D3D12_CPU_DESCRIPTOR_HANDLE & other) const
    {
        return (ptr != other.ptr);
    }

    CD3DX12_CPU_DESCRIPTOR_HANDLE& operator=(const D3D12_CPU_DESCRIPTOR_HANDLE & other)
    {
        ptr = other.ptr;
        return *this;
    }

    inline void InitOffsetted(_In_ const D3D12_CPU_DESCRIPTOR_HANDLE & base, INT offsetScaledByIncrementSize)
    {
        InitOffsetted(*this, base, offsetScaledByIncrementSize);
    }

    inline void InitOffsetted(_In_ const D3D12_CPU_DESCRIPTOR_HANDLE & base, INT offsetInDescriptors, UINT descriptorIncrementSize)
    {
        InitOffsetted(*this, base, offsetInDescriptors, descriptorIncrementSize);
    }

    static inline void InitOffsetted(_Out_ D3D12_CPU_DESCRIPTOR_HANDLE & handle, _In_ const D3D12_CPU_DESCRIPTOR_HANDLE & base, INT offsetScaledByIncrementSize)
    {
        handle.ptr = base.ptr + offsetScaledByIncrementSize;
    }

    static inline void InitOffsetted(_Out_ D3D12_CPU_DESCRIPTOR_HANDLE & handle, _In_ const D3D12_CPU_DESCRIPTOR_HANDLE & base, INT offsetInDescriptors, UINT descriptorIncrementSize)
    {
        handle.ptr = static_cast<SIZE_T>(base.ptr + INT64(offsetInDescriptors) * UINT64(descriptorIncrementSize));
    }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace vk
} //namespace renderer
} //namespace v3d12


#else
#   error "DirectX version not supported"
#endif //(D3D_VERSION_MAJOR == 12)

#endif //D3D_RENDER
