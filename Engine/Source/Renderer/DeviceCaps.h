#pragma once

#include "Common.h"
#include "TextureProperties.h"
#include "BufferProperties.h"


namespace v3d
{
namespace renderer
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief VendorID enums
    */
    enum VendorID
    {
        VendorID_Empty              = 0x0000,
        VendorID_ATI                = 0x1002,
        VendorID_ImgTech            = 0x1010,
        VendorID_ImgTech_Legacy     = 0x104a,
        VendorID_NVIDIA             = 0x10de,
        VendorID_ARM                = 0x13b5,
        VendorID_Microsoft          = 0x1414,
        VendorID_VMWare             = 0x15ad,
        VendorID_Qualcomm           = 0x5143,
        VendorID_Intel              = 0x8086,
        VendorID_VBox               = 0x80ee,
        VendorID_Mesa               = 0x10005,
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
    * @brief DeviceCaps struct. Game/Render side.
    */
    struct DeviceCaps
    {
        enum TilingType : u32
        {
            TilingType_Optimal = 0,
            TilingType_Linear = 1,

            TilingType_Count
        };

        u32 maxColorAttachments = k_maxColorAttachments;
        u32 maxVertexInputAttributes = k_maxVertexInputAttributes;
        u32 maxVertexInputBindings = k_maxVertexInputBindings;

        VendorID vendorID = VendorID_Empty;

        bool supportMultiview = false;

        struct ImageFormatSupport
        {
            bool _supportAttachment;
            bool _supportSampled;
            bool _supportMip;
        };

        const ImageFormatSupport& getImageFormatSupportInfo(Format format, TilingType type)
        {
            return m_imageFormatSupport[format][type];
        }

        DeviceCaps() = default;
        virtual ~DeviceCaps() = default;

    protected:

        ImageFormatSupport m_imageFormatSupport[Format::Format_Count][TilingType::TilingType_Count] = {};
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
