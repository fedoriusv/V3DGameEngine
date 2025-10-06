#pragma once

#include "Render.h"

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

    inline std::string VendorIDString(u32 vendor)
    {
        switch (vendor)
        {
#define STR(r) case VendorID_ ##r: return #r
            STR(ATI);
            STR(ImgTech);
            STR(ImgTech_Legacy);
            STR(NVIDIA);
            STR(ARM);
            STR(Microsoft);
            STR(VMWare);
            STR(Qualcomm);
            STR(Intel);
            STR(VBox);
            STR(Mesa);
#undef STR
        default:
            return "UNKNOWN";
        }
    }

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

        u32 _maxColorAttachments = k_maxColorAttachments;

        u32 _maxVertexInputAttributes = k_maxVertexInputAttributes;
        u32 _maxVertexInputBindings = k_maxVertexInputBindings;

        u32 _maxDescriptorSets = k_maxDescriptorSetCount;
        u32 _maxDescriptorBindingsPerSet = k_maxDescriptorSlotsCount;

        u32 _constantBufferSize = 1024; //1 KB

        VendorID _vendorID = VendorID_Empty;

        bool _supportMultiview = false;
        bool _supportBlitImage = false;

        struct ImageFormatSupport
        {
            bool _supportAttachment;
            bool _supportSampled;
            bool _supportStorage;
            bool _supportMip;
            bool _supportResolve;
        };

        const ImageFormatSupport& getImageFormatSupportInfo(Format format, TilingType type) const
        {
            return _imageFormatSupport[format][type];
        }

    protected:

        DeviceCaps() = default;
        virtual ~DeviceCaps() = default;

        ImageFormatSupport _imageFormatSupport[Format::Format_Count][TilingType::TilingType_Count] = {};
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace renderer
} //namespace v3d
