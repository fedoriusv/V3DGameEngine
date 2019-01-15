#include "Formats.h"

namespace v3d
{
namespace renderer
{

    const u32 k_formatList[Format::Format_Count][2] = 
    {
        /*                                             Components, Bytes*/
        /* Format_Undefined = 0*/                           { 0, 0  },

        /* Format_R4G4_UNorm_Pack8 = 1*/                    { 2, 8  },
        /* Format_R4G4B4A4_UNorm_Pack16 = 2*/               { 4, 16 },
        /* Format_B4G4R4A4_UNorm_Pack16 = 3*/               { 4, 16 },
        /* Format_R5G6B5_UNorm_Pack16 = 4*/                 { 3, 16 },
        /* Format_B5G6R5_UNorm_Pack16 = 5*/                 { 3, 16 },
        /* Format_R5G5B5A1_UNorm_Pack16 = 6*/               { 4, 16 },
        /* Format_B5G5R5A1_UNorm_Pack16 = 7*/               { 4, 16 },
        /* Format_A1R5G5B5_UNorm_Pack16 = 8*/               { 4, 16 },

        /* Format_R8_UNorm = 9*/                            { 1, 8 },
        /* Format_R8_SNorm = 10*/                           { 1, 8 },
        /* Format_R8_UScaled = 11*/                         { 1, 8 },
        /* Format_R8_SScaled = 12*/                         { 1, 8 },
        /* Format_R8_UInt = 13*/                            { 1, 8 },
        /* Format_R8_SInt = 14*/                            { 1, 8 },
        /* Format_R8_SRGB = 15*/                            { 1, 8 },

        /* Format_R8G8_UNorm = 16*/                         { 2, 16 },
        /* Format_R8G8_SNorm = 17*/                         { 2, 16 },
        /* Format_R8G8_UScaled = 18*/                       { 2, 16 },
        /* Format_R8G8_SScaled = 19*/                       { 2, 16 },
        /* Format_R8G8_UInt = 20*/                          { 2, 16 },
        /* Format_R8G8_SInt = 21*/                          { 2, 16 },
        /* Format_R8G8_SRGB = 22*/                          { 2, 16 },

        /* Format_R8G8B8_UNorm = 23*/                       { 3, 24 },
        /* Format_R8G8B8_SNorm = 24*/                       { 3, 24 },
        /* Format_R8G8B8_UScaled = 25*/                     { 3, 24 },
        /* Format_R8G8B8_SScaled = 26*/                     { 3, 24 },
        /* Format_R8G8B8_UInt = 27*/                        { 3, 24 },
        /* Format_R8G8B8_SInt = 28*/                        { 3, 24 },
        /* Format_R8G8B8_SRGB = 29*/                        { 3, 24 },
        /* Format_B8G8R8_UNorm = 30*/                       { 3, 24 },
        /* Format_B8G8R8_SNorm = 31*/                       { 3, 24 },
        /* Format_B8G8R8_UScaled = 32*/                     { 3, 24 },
        /* Format_B8G8R8_SScaled = 33*/                     { 3, 24 },
        /* Format_B8G8R8_UInt = 34*/                        { 3, 24 },
        /* Format_B8G8R8_SInt = 35*/                        { 3, 24 },
        /* Format_B8G8R8_SRGB = 36*/                        { 3, 24 },

        /* Format_R8G8B8A8_UNorm = 37*/                     { 4, 32 },
        /* Format_R8G8B8A8_SNorm = 38*/                     { 4, 32 },
        /* Format_R8G8B8A8_UScaled = 39*/                   { 4, 32 },
        /* Format_R8G8B8A8_SScaled = 40*/                   { 4, 32 },
        /* Format_R8G8B8A8_UInt = 41*/                      { 4, 32 },
        /* Format_R8G8B8A8_SInt = 42*/                      { 4, 32 },
        /* Format_R8G8B8A8_SRGB = 43*/                      { 4, 32 },
        /* Format_B8G8R8A8_UNorm = 44*/                     { 4, 32 },
        /* Format_B8G8R8A8_SNorm = 45*/                     { 4, 32 },
        /* Format_B8G8R8A8_UScaled = 46*/                   { 4, 32 },
        /* Format_B8G8R8A8_SScaled = 47*/                   { 4, 32 },
        /* Format_B8G8R8A8_UInt = 48*/                      { 4, 32 },
        /* Format_B8G8R8A8_SInt = 49*/                      { 4, 32 },
        /* Format_B8G8R8A8_SRGB = 50*/                      { 4, 32 },
        /* Format_A8B8G8R8_UNorm_Pack32 = 51*/              { 4, 32 },
        /* Format_A8B8G8R8_SNorm_Pack32 = 52*/              { 4, 32 },
        /* Format_A8B8G8R8_UScaled_Pack32 = 53*/            { 4, 32 },
        /* Format_A8B8G8R8_SScaled_Pack32 = 54*/            { 4, 32 },
        /* Format_A8B8G8R8_UInt_Pack32 = 55*/               { 4, 32 },
        /* Format_A8B8G8R8_SInt_Pack32 = 56*/               { 4, 32 },
        /* Format_A8B8G8R8_SRGB_Pack32 = 57*/               { 4, 32 },

        /* Format_A2R10G10B10_UNorm_Pack32 = 58*/           { 4, 32 },
        /* Format_A2R10G10B10_SNorm_Pack32 = 59*/           { 4, 32 },
        /* Format_A2R10G10B10_UScaled_Pack32 = 60*/         { 4, 32 },
        /* Format_A2R10G10B10_SScaled_Pack32 = 61*/         { 4, 32 },
        /* Format_A2R10G10B10_UInt_Pack32 = 62*/            { 4, 32 },
        /* Format_A2R10G10B10_SInt_Pack32 = 63*/            { 4, 32 },
        /* Format_A2B10G10R10_UNorm_Pack32 = 64*/           { 4, 32 },
        /* Format_A2B10G10R10_SNorm_Pack32 = 65*/           { 4, 32 },
        /* Format_A2B10G10R10_UScaled_Pack32 = 66*/         { 4, 32 },
        /* Format_A2B10G10R10_SScaled_Pack32 = 67*/         { 4, 32 },
        /* Format_A2B10G10R10_UInt_Pack32 = 68*/            { 4, 32 },
        /* Format_A2B10G10R10_SInt_Pack32 = 69*/            { 4, 32 },

        /* Format_R16_UNorm = 70*/                          { 1, 16 },
        /* Format_R16_SNorm = 71*/                          { 1, 16 },
        /* Format_R16_UScaled = 72*/                        { 1, 16 },
        /* Format_R16_SScaled = 73*/                        { 1, 16 },
        /* Format_R16_UInt = 74*/                           { 1, 16 },
        /* Format_R16_SInt = 75*/                           { 1, 16 },
        /* Format_R16_SFloat = 76*/                         { 1, 16 },

        /* Format_R16G16_UNorm = 77*/                       { 2, 32 },
        /* Format_R16G16_SNorm = 78*/                       { 2, 32 },
        /* Format_R16G16_UScaled = 79*/                     { 2, 32 },
        /* Format_R16G16_SScaled = 80*/                     { 2, 32 },
        /* Format_R16G16_UInt = 81*/                        { 2, 32 },
        /* Format_R16G16_SInt = 82*/                        { 2, 32 },
        /* Format_R16G16_SFloat = 83*/                      { 2, 32 },

        /* Format_R16G16B16_UNorm = 84*/                    { 3, 48 },
        /* Format_R16G16B16_SNorm = 85*/                    { 3, 48 },
        /* Format_R16G16B16_UScaled = 86*/                  { 3, 48 },
        /* Format_R16G16B16_SScaled = 87*/                  { 3, 48 },
        /* Format_R16G16B16_UInt = 88*/                     { 3, 48 },
        /* Format_R16G16B16_SInt = 89*/                     { 3, 48 },
        /* Format_R16G16B16_SFloat = 90*/                   { 3, 48 },

        /* Format_R16G16B16A16_UNorm = 91*/                 { 4, 64 },
        /* Format_R16G16B16A16_SNorm = 92*/                 { 4, 64 },
        /* Format_R16G16B16A16_UScaled = 93*/               { 4, 64 },
        /* Format_R16G16B16A16_SScaled = 94*/               { 4, 64 },
        /* Format_R16G16B16A16_UInt = 95*/                  { 4, 64 },
        /* Format_R16G16B16A16_SInt = 96*/                  { 4, 64 },
        /* Format_R16G16B16A16_SFloat = 97*/                { 4, 64 },

        /* Format_R32_UInt = 98*/                           { 1, 32 },
        /* Format_R32_SInt = 99*/                           { 1, 32 },
        /* Format_R32_SFloat = 100*/                        { 1, 32 },

        /* Format_R32G32_UInt = 101*/                       { 2, 64 },
        /* Format_R32G32_SInt = 102*/                       { 2, 64 },
        /* Format_R32G32_SFloat = 103*/                     { 2, 64 },

        /* Format_R32G32B32_UInt = 104*/                    { 3, 96 },
        /* Format_R32G32B32_SInt = 105*/                    { 3, 96 },
        /* Format_R32G32B32_SFloat = 106*/                  { 3, 96 },

        /* Format_R32G32B32A32_UInt = 107*/                 { 4, 128 },
        /* Format_R32G32B32A32_SInt = 108*/                 { 4, 128 },
        /* Format_R32G32B32A32_SFloat = 109*/               { 4, 128 },

        /* Format_R64_UInt = 110*/                          { 1, 64 },
        /* Format_R64_SInt = 111*/                          { 1, 64 },
        /* Format_R64_SFloat = 112*/                        { 1, 64 },

        /* Format_R64G64_UInt = 113*/                       { 2, 128 },
        /* Format_R64G64_SInt = 114*/                       { 2, 128 },
        /* Format_R64G64_SFloat = 115*/                     { 2, 128 },

        /* Format_R64G64B64_UInt = 116*/                    { 3, 192 },
        /* Format_R64G64B64_SInt = 117*/                    { 3, 192 },
        /* Format_R64G64B64_SFloat = 118*/                  { 3, 192 },

        /* Format_R64G64B64A64_UInt = 119*/                 { 4, 256 },
        /* Format_R64G64B64A64_SInt = 120*/                 { 4, 256 },
        /* Format_R64G64B64A64_SFloat = 121*/               { 4, 256 },

        /* Format_B10G11R11_UFloat_Pack32 = 122*/           { 3, 32 },
        /* Format_E5B9G9R9_UFloat_Pack32 = 123*/            { 3, 32 },

        /* Format_D16_UNorm = 124*/                         { 1, 16 },
        /* Format_X8_D24_UNorm_Pack32 = 125*/               { 1, 24 },
        /* Format_D32_SFloat = 126*/                        { 1, 32 },
        /* Format_S8_UInt = 127*/                           { 1, 8  },
        /* Format_D16_UNorm_S8_UInt = 128*/                 { 2, 24 },
        /* Format_D24_UNorm_S8_UInt = 129*/                 { 2, 32 },
        /* Format_D32_SFloat_S8_UInt = 130*/                { 2, 40 },

        /* Format_BC1_RGB_UNorm_Block = 131*/               { 0, 0 },
        /* Format_BC1_RGB_SRGB_Block = 132*/                { 0, 0 },
        /* Format_BC1_RGBA_UNorm_Block = 133*/              { 0, 0 },
        /* Format_BC1_RGBA_SRGB_Block = 134*/               { 0, 0 },
        /* Format_BC2_UNorm_Block = 135*/                   { 0, 0 },
        /* Format_BC2_SRGB_Block = 136*/                    { 0, 0 },
        /* Format_BC3_UNorm_Block = 137*/                   { 0, 0 },
        /* Format_BC3_SRGB_Block = 138*/                    { 0, 0 },
        /* Format_BC4_UNorm_Block = 139*/                   { 0, 0 },
        /* Format_BC4_SNorm_Block = 140*/                   { 0, 0 },
        /* Format_BC5_UNorm_Block = 141*/                   { 0, 0 },
        /* Format_BC5_SNorm_Block = 142*/                   { 0, 0 },
        /* Format_BC6H_UFloat_Block = 143*/                 { 0, 0 },
        /* Format_BC6H_SFloat_Block = 144*/                 { 0, 0 },
        /* Format_BC7_UNorm_Block = 145*/                   { 0, 0 },
        /* Format_BC7_SRGB_Block = 146*/                    { 0, 0 },
        /* Format_ETC2_R8G8B8_UNorm_Block = 147*/           { 0, 0 },
        /* Format_ETC2_R8G8B8_SRGB_Block = 148*/            { 0, 0 },
        /* Format_ETC2_R8G8B8A1_UNorm_Block = 149*/         { 0, 0 },
        /* Format_ETC2_R8G8B8A1_SRGB_Block = 150*/          { 0, 0 },
        /* Format_ETC2_R8G8B8A8_UNorm_Block = 151*/         { 0, 0 },
        /* Format_ETC2_R8G8B8A8_SRGB_Block = 152*/          { 0, 0 },
        /* Format_EAC_R11_UNorm_Block = 153*/               { 0, 0 },
        /* Format_EAC_R11_SNorm_Block = 154*/               { 0, 0 },
        /* Format_EAC_R11G11_UNorm_Block = 155*/            { 0, 0 },
        /* Format_EAC_R11G11_SNorm_Block = 156*/            { 0, 0 },
        /* Format_ASTC_4x4_UNorm_Block = 157*/              { 0, 0 },
        /* Format_ASTC_4x4_SRGB_Block = 158*/               { 0, 0 },
        /* Format_ASTC_5x4_UNorm_Block = 159*/              { 0, 0 },
        /* Format_ASTC_5x4_SRGB_Block = 160*/               { 0, 0 },
        /* Format_ASTC_5x5_UNorm_Block = 161*/              { 0, 0 },
        /* Format_ASTC_5x5_SRGB_Block = 162*/               { 0, 0 },
        /* Format_ASTC_6x5_UNorm_Block = 163*/              { 0, 0 },
        /* Format_ASTC_6x5_SRGB_Block = 164*/               { 0, 0 },
        /* Format_ASTC_6x6_UNorm_Block = 165*/              { 0, 0 },
        /* Format_ASTC_6x6_SRGB_Block = 166*/               { 0, 0 },
        /* Format_ASTC_8x5_UNorm_Block = 167*/              { 0, 0 },
        /* Format_ASTC_8x5_SRGB_Block = 168*/               { 0, 0 },
        /* Format_ASTC_8x6_UNorm_Block = 169*/              { 0, 0 },
        /* Format_ASTC_8x6_SRGB_Block = 170*/               { 0, 0 },
        /* Format_ASTC_8x8_UNorm_Block = 171*/              { 0, 0 },
        /* Format_ASTC_8x8_SRGB_Block = 172*/               { 0, 0 },
        /* Format_ASTC_10x5_UNorm_Block = 173*/             { 0, 0 },
        /* Format_ASTC_10x5_SRGB_Block = 174*/              { 0, 0 },
        /* Format_ASTC_10x6_UNorm_Block = 175*/             { 0, 0 },
        /* Format_ASTC_10x6_SRGB_Block = 176*/              { 0, 0 },
        /* Format_ASTC_10x8_UNorm_Block = 177*/             { 0, 0 },
        /* Format_ASTC_10x8_SRGB_Block = 178*/              { 0, 0 },
        /* Format_ASTC_10x10_UNorm_Block = 179*/            { 0, 0 },
        /* Format_ASTC_10x10_SRGB_Block = 180*/             { 0, 0 },
        /* Format_ASTC_12x10_UNorm_Block = 181*/            { 0, 0 },
        /* Format_ASTC_12x10_SRGB_Block = 182*/             { 0, 0 },
        /* Format_ASTC_12x12_UNorm_Block = 183*/            { 0, 0 },
        /* Format_ASTC_12x12_SRGB_Block = 184*/             { 0, 0 }
    };

    u32 getFormatSize(Format format)
    {
        return k_formatList[format][1];
    }

    u32 getCountComponent(Format format)
    {
        return k_formatList[format][0];
    }

} //namespace renderer
} //namespace v3d
