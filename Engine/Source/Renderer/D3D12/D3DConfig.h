#pragma once

#ifdef D3D_RENDER
#   define D3D_VERSION_MAJOR 12
#   define D3D_VERSION_MINOR 0

#   if defined(NTDDI_VERSION) && (NTDDI_VERSION < 0x0A00000A/*NTDDI_WIN10_FE*/)
#       error "Windows SDK must be 10.0.20348 or later. Make sure you have a proper version"
#   endif

#   define D3D_DEBUG 0 //Detailed logs
#   define D3D_DEBUG_LAYERS 1
#   if D3D_DEBUG_LAYERS
#       define D3D_DEBUG_LIVE_REPORT 1
#       define D3D_DEBUG_LAYERS_CALLBACK 1
#   endif
#   define D3D_DEBUG_MARKERS 1
#   define D3D_DEBUG_PIX_MARKERS 0
#   define D3D_DRED_ENABLE 0
#   define D3D_AFTERMATH_ENABLE 0

#   define D3D_MAKE_VERSION(major, minor) (((major) << 22) | ((minor) << 12))

#   define D3D_VERSION_12_0 D3D_MAKE_VERSION(12, 0)
#   define D3D_VERSION_12_1 D3D_MAKE_VERSION(12, 1)
#   define D3D_VERSION_12_2 D3D_MAKE_VERSION(12, 2)

#   define D3D_CURRENT_VERSION D3D_MAKE_VERSION(D3D_VERSION_MAJOR, D3D_VERSION_MINOR)

#endif //D3D_RENDER