#include "Formats.h"

namespace v3d
{
namespace renderer
{

    const u32 k_formatList[Format::Format_Count][5] = 
    {
        /*                                             Components, Block Size(bytes), Block Dx Block Dy, Compressed*/
        /* Format_Undefined = 0*/                           { 0,            0,          0,       0,     false },
                                                                                        
        /* Format_R4G4_UNorm_Pack8 = 1*/                    { 2,            1,          1,       1,     false },
        /* Format_R4G4B4A4_UNorm_Pack16 = 2*/               { 4,            1,          1,       1,     false },
        /* Format_B4G4R4A4_UNorm_Pack16 = 3*/               { 4,            1,          1,       1,     false },
        /* Format_R5G6B5_UNorm_Pack16 = 4*/                 { 3,            2,          1,       1,     false },
        /* Format_B5G6R5_UNorm_Pack16 = 5*/                 { 3,            2,          1,       1,     false },
        /* Format_R5G5B5A1_UNorm_Pack16 = 6*/               { 4,            2,          1,       1,     false },
        /* Format_B5G5R5A1_UNorm_Pack16 = 7*/               { 4,            2,          1,       1,     false },
        /* Format_A1R5G5B5_UNorm_Pack16 = 8*/               { 4,            2,          1,       1,     false },

        /* Format_R8_UNorm = 9*/                            { 1,            1,          1,       1,     false },
        /* Format_R8_SNorm = 10*/                           { 1,            1,          1,       1,     false },
        /* Format_R8_UScaled = 11*/                         { 1,            1,          1,       1,     false },
        /* Format_R8_SScaled = 12*/                         { 1,            1,          1,       1,     false },
        /* Format_R8_UInt = 13*/                            { 1,            1,          1,       1,     false },
        /* Format_R8_SInt = 14*/                            { 1,            1,          1,       1,     false },
        /* Format_R8_SRGB = 15*/                            { 1,            1,          1,       1,     false },

        /* Format_R8G8_UNorm = 16*/                         { 2,            2,          1,       1,     false },
        /* Format_R8G8_SNorm = 17*/                         { 2,            2,          1,       1,     false },
        /* Format_R8G8_UScaled = 18*/                       { 2,            2,          1,       1,     false },
        /* Format_R8G8_SScaled = 19*/                       { 2,            2,          1,       1,     false },
        /* Format_R8G8_UInt = 20*/                          { 2,            2,          1,       1,     false },
        /* Format_R8G8_SInt = 21*/                          { 2,            2,          1,       1,     false },
        /* Format_R8G8_SRGB = 22*/                          { 2,            2,          1,       1,     false },

        /* Format_R8G8B8_UNorm = 23*/                       { 3,            3,          1,       1,     false },
        /* Format_R8G8B8_SNorm = 24*/                       { 3,            3,          1,       1,     false },
        /* Format_R8G8B8_UScaled = 25*/                     { 3,            3,          1,       1,     false },
        /* Format_R8G8B8_SScaled = 26*/                     { 3,            3,          1,       1,     false },
        /* Format_R8G8B8_UInt = 27*/                        { 3,            3,          1,       1,     false },
        /* Format_R8G8B8_SInt = 28*/                        { 3,            3,          1,       1,     false },
        /* Format_R8G8B8_SRGB = 29*/                        { 3,            3,          1,       1,     false },
        /* Format_B8G8R8_UNorm = 30*/                       { 3,            3,          1,       1,     false },
        /* Format_B8G8R8_SNorm = 31*/                       { 3,            3,          1,       1,     false },
        /* Format_B8G8R8_UScaled = 32*/                     { 3,            3,          1,       1,     false },
        /* Format_B8G8R8_SScaled = 33*/                     { 3,            3,          1,       1,     false },
        /* Format_B8G8R8_UInt = 34*/                        { 3,            3,          1,       1,     false },
        /* Format_B8G8R8_SInt = 35*/                        { 3,            3,          1,       1,     false },
        /* Format_B8G8R8_SRGB = 36*/                        { 3,            3,          1,       1,     false },

        /* Format_R8G8B8A8_UNorm = 37*/                     { 4,            4,          1,       1,     false },
        /* Format_R8G8B8A8_SNorm = 38*/                     { 4,            4,          1,       1,     false },
        /* Format_R8G8B8A8_UScaled = 39*/                   { 4,            4,          1,       1,     false },
        /* Format_R8G8B8A8_SScaled = 40*/                   { 4,            4,          1,       1,     false },
        /* Format_R8G8B8A8_UInt = 41*/                      { 4,            4,          1,       1,     false },
        /* Format_R8G8B8A8_SInt = 42*/                      { 4,            4,          1,       1,     false },
        /* Format_R8G8B8A8_SRGB = 43*/                      { 4,            4,          1,       1,     false },
        /* Format_B8G8R8A8_UNorm = 44*/                     { 4,            4,          1,       1,     false },
        /* Format_B8G8R8A8_SNorm = 45*/                     { 4,            4,          1,       1,     false },
        /* Format_B8G8R8A8_UScaled = 46*/                   { 4,            4,          1,       1,     false },
        /* Format_B8G8R8A8_SScaled = 47*/                   { 4,            4,          1,       1,     false },
        /* Format_B8G8R8A8_UInt = 48*/                      { 4,            4,          1,       1,     false },
        /* Format_B8G8R8A8_SInt = 49*/                      { 4,            4,          1,       1,     false },
        /* Format_B8G8R8A8_SRGB = 50*/                      { 4,            4,          1,       1,     false },
        /* Format_A8B8G8R8_UNorm_Pack32 = 51*/              { 4,            4,          1,       1,     false },
        /* Format_A8B8G8R8_SNorm_Pack32 = 52*/              { 4,            4,          1,       1,     false },
        /* Format_A8B8G8R8_UScaled_Pack32 = 53*/            { 4,            4,          1,       1,     false },
        /* Format_A8B8G8R8_SScaled_Pack32 = 54*/            { 4,            4,          1,       1,     false },
        /* Format_A8B8G8R8_UInt_Pack32 = 55*/               { 4,            4,          1,       1,     false },
        /* Format_A8B8G8R8_SInt_Pack32 = 56*/               { 4,            4,          1,       1,     false },
        /* Format_A8B8G8R8_SRGB_Pack32 = 57*/               { 4,            4,          1,       1,     false },

        /* Format_A2R10G10B10_UNorm_Pack32 = 58*/           { 4,            4,          1,       1,     false },
        /* Format_A2R10G10B10_SNorm_Pack32 = 59*/           { 4,            4,          1,       1,     false },
        /* Format_A2R10G10B10_UScaled_Pack32 = 60*/         { 4,            4,          1,       1,     false },
        /* Format_A2R10G10B10_SScaled_Pack32 = 61*/         { 4,            4,          1,       1,     false },
        /* Format_A2R10G10B10_UInt_Pack32 = 62*/            { 4,            4,          1,       1,     false },
        /* Format_A2R10G10B10_SInt_Pack32 = 63*/            { 4,            4,          1,       1,     false },
        /* Format_A2B10G10R10_UNorm_Pack32 = 64*/           { 4,            4,          1,       1,     false },
        /* Format_A2B10G10R10_SNorm_Pack32 = 65*/           { 4,            4,          1,       1,     false },
        /* Format_A2B10G10R10_UScaled_Pack32 = 66*/         { 4,            4,          1,       1,     false },
        /* Format_A2B10G10R10_SScaled_Pack32 = 67*/         { 4,            4,          1,       1,     false },
        /* Format_A2B10G10R10_UInt_Pack32 = 68*/            { 4,            4,          1,       1,     false },
        /* Format_A2B10G10R10_SInt_Pack32 = 69*/            { 4,            4,          1,       1,     false },

        /* Format_R16_UNorm = 70*/                          { 1,            2,          1,       1,     false },
        /* Format_R16_SNorm = 71*/                          { 1,            2,          1,       1,     false },
        /* Format_R16_UScaled = 72*/                        { 1,            2,          1,       1,     false },
        /* Format_R16_SScaled = 73*/                        { 1,            2,          1,       1,     false },
        /* Format_R16_UInt = 74*/                           { 1,            2,          1,       1,     false },
        /* Format_R16_SInt = 75*/                           { 1,            2,          1,       1,     false },
        /* Format_R16_SFloat = 76*/                         { 1,            2,          1,       1,     false },

        /* Format_R16G16_UNorm = 77*/                       { 2,            4,          1,       1,     false },
        /* Format_R16G16_SNorm = 78*/                       { 2,            4,          1,       1,     false },
        /* Format_R16G16_UScaled = 79*/                     { 2,            4,          1,       1,     false },
        /* Format_R16G16_SScaled = 80*/                     { 2,            4,          1,       1,     false },
        /* Format_R16G16_UInt = 81*/                        { 2,            4,          1,       1,     false },
        /* Format_R16G16_SInt = 82*/                        { 2,            4,          1,       1,     false },
        /* Format_R16G16_SFloat = 83*/                      { 2,            4,          1,       1,     false },

        /* Format_R16G16B16_UNorm = 84*/                    { 3,            6,          1,       1,     false },
        /* Format_R16G16B16_SNorm = 85*/                    { 3,            6,          1,       1,     false },
        /* Format_R16G16B16_UScaled = 86*/                  { 3,            6,          1,       1,     false },
        /* Format_R16G16B16_SScaled = 87*/                  { 3,            6,          1,       1,     false },
        /* Format_R16G16B16_UInt = 88*/                     { 3,            6,          1,       1,     false },
        /* Format_R16G16B16_SInt = 89*/                     { 3,            6,          1,       1,     false },
        /* Format_R16G16B16_SFloat = 90*/                   { 3,            6,          1,       1,     false },

        /* Format_R16G16B16A16_UNorm = 91*/                 { 4,            8,          1,       1,     false },
        /* Format_R16G16B16A16_SNorm = 92*/                 { 4,            8,          1,       1,     false },
        /* Format_R16G16B16A16_UScaled = 93*/               { 4,            8,          1,       1,     false },
        /* Format_R16G16B16A16_SScaled = 94*/               { 4,            8,          1,       1,     false },
        /* Format_R16G16B16A16_UInt = 95*/                  { 4,            8,          1,       1,     false },
        /* Format_R16G16B16A16_SInt = 96*/                  { 4,            8,          1,       1,     false },
        /* Format_R16G16B16A16_SFloat = 97*/                { 4,            8,          1,       1,     false },

        /* Format_R32_UInt = 98*/                           { 1,            4,          1,       1,     false },
        /* Format_R32_SInt = 99*/                           { 1,            4,          1,       1,     false },
        /* Format_R32_SFloat = 100*/                        { 1,            4,          1,       1,     false },

        /* Format_R32G32_UInt = 101*/                       { 2,            8,          1,       1,     false },
        /* Format_R32G32_SInt = 102*/                       { 2,            8,          1,       1,     false },
        /* Format_R32G32_SFloat = 103*/                     { 2,            8,          1,       1,     false },

        /* Format_R32G32B32_UInt = 104*/                    { 3,            12,         1,       1,     false },
        /* Format_R32G32B32_SInt = 105*/                    { 3,            12,         1,       1,     false },
        /* Format_R32G32B32_SFloat = 106*/                  { 3,            12,         1,       1,     false },

        /* Format_R32G32B32A32_UInt = 107*/                 { 4,            16,         1,       1,     false },
        /* Format_R32G32B32A32_SInt = 108*/                 { 4,            16,         1,       1,     false },
        /* Format_R32G32B32A32_SFloat = 109*/               { 4,            16,         1,       1,     false },

        /* Format_R64_UInt = 110*/                          { 1,            8,          1,       1,     false },
        /* Format_R64_SInt = 111*/                          { 1,            8,          1,       1,     false },
        /* Format_R64_SFloat = 112*/                        { 1,            8,          1,       1,     false },

        /* Format_R64G64_UInt = 113*/                       { 2,            16,         1,       1,     false },
        /* Format_R64G64_SInt = 114*/                       { 2,            16,         1,       1,     false },
        /* Format_R64G64_SFloat = 115*/                     { 2,            16,         1,       1,     false },

        /* Format_R64G64B64_UInt = 116*/                    { 3,            24,         1,       1,     false },
        /* Format_R64G64B64_SInt = 117*/                    { 3,            24,         1,       1,     false },
        /* Format_R64G64B64_SFloat = 118*/                  { 3,            24,         1,       1,     false },

        /* Format_R64G64B64A64_UInt = 119*/                 { 4,            32,         1,       1,     false },
        /* Format_R64G64B64A64_SInt = 120*/                 { 4,            32,         1,       1,     false },
        /* Format_R64G64B64A64_SFloat = 121*/               { 4,            32,         1,       1,     false },

        /* Format_B10G11R11_UFloat_Pack32 = 122*/           { 3,            4,          1,       1,     false },
        /* Format_E5B9G9R9_UFloat_Pack32 = 123*/            { 3,            4,          1,       1,     false },

        /* Format_D16_UNorm = 124*/                         { 1,            2,          1,       1,     false },
        /* Format_X8_D24_UNorm_Pack32 = 125*/               { 1,            4,          1,       1,     false },
        /* Format_D32_SFloat = 126*/                        { 1,            4,          1,       1,     false },
        /* Format_S8_UInt = 127*/                           { 1,            1,          1,       1,     false },
        /* Format_D16_UNorm_S8_UInt = 128*/                 { 2,            3,          1,       1,     false },
        /* Format_D24_UNorm_S8_UInt = 129*/                 { 2,            4,          1,       1,     false },
        /* Format_D32_SFloat_S8_UInt = 130*/                { 2,            8,          1,       1,     false },

        /* Format_BC1_RGB_UNorm_Block = 131*/               { 3,            8,          4,       4,     true },
        /* Format_BC1_RGB_SRGB_Block = 132*/                { 3,            8,          4,       4,     true },
        /* Format_BC1_RGBA_UNorm_Block = 133*/              { 4,            8,          4,       4,     true },
        /* Format_BC1_RGBA_SRGB_Block = 134*/               { 4,            8,          4,       4,     true },
        /* Format_BC2_UNorm_Block = 135*/                   { 4,            16,         4,       4,     true },
        /* Format_BC2_SRGB_Block = 136*/                    { 4,            16,         4,       4,     true },
        /* Format_BC3_UNorm_Block = 137*/                   { 4,            16,         4,       4,     true },
        /* Format_BC3_SRGB_Block = 138*/                    { 4,            16,         4,       4,     true },
        /* Format_BC4_UNorm_Block = 139*/                   { 1,            8,          4,       4,     true },
        /* Format_BC4_SNorm_Block = 140*/                   { 1,            8,          4,       4,     true },
        /* Format_BC5_UNorm_Block = 141*/                   { 2,            16,         4,       4,     true },
        /* Format_BC5_SNorm_Block = 142*/                   { 2,            16,         4,       4,     true },
        /* Format_BC6H_UFloat_Block = 143*/                 { 3,            16,         4,       4,     true },
        /* Format_BC6H_SFloat_Block = 144*/                 { 3,            16,         4,       4,     true },
        /* Format_BC7_UNorm_Block = 145*/                   { 4,            16,         4,       4,     true },
        /* Format_BC7_SRGB_Block = 146*/                    { 4,            16,         4,       4,     true },

        /* Format_ETC2_R8G8B8_UNorm_Block = 147*/           { 3,            8,          4,       4,     true },
        /* Format_ETC2_R8G8B8_SRGB_Block = 148*/            { 3,            8,          4,       4,     true },
        /* Format_ETC2_R8G8B8A1_UNorm_Block = 149*/         { 4,            8,          4,       4,     true },
        /* Format_ETC2_R8G8B8A1_SRGB_Block = 150*/          { 4,            8,          4,       4,     true },
        /* Format_ETC2_R8G8B8A8_UNorm_Block = 151*/         { 4,            16,         4,       4,     true },
        /* Format_ETC2_R8G8B8A8_SRGB_Block = 152*/          { 4,            16,         4,       4,     true },
        /* Format_EAC_R11_UNorm_Block = 153*/               { 1,            8,          4,       4,     true },
        /* Format_EAC_R11_SNorm_Block = 154*/               { 1,            8,          4,       4,     true },
        /* Format_EAC_R11G11_UNorm_Block = 155*/            { 2,            16,         4,       4,     true },
        /* Format_EAC_R11G11_SNorm_Block = 156*/            { 2,            16,         4,       4,     true },

        /* Format_ASTC_4x4_UNorm_Block = 157*/              { 4,            16,         4,       4,     true },
        /* Format_ASTC_4x4_SRGB_Block = 158*/               { 4,            16,         4,       4,     true },
        /* Format_ASTC_5x4_UNorm_Block = 159*/              { 4,            16,         5,       4,     true },
        /* Format_ASTC_5x4_SRGB_Block = 160*/               { 4,            16,         5,       4,     true },
        /* Format_ASTC_5x5_UNorm_Block = 161*/              { 4,            16,         5,       5,     true },
        /* Format_ASTC_5x5_SRGB_Block = 162*/               { 4,            16,         5,       5,     true },
        /* Format_ASTC_6x5_UNorm_Block = 163*/              { 4,            16,         6,       5,     true },
        /* Format_ASTC_6x5_SRGB_Block = 164*/               { 4,            16,         6,       5,     true },
        /* Format_ASTC_6x6_UNorm_Block = 165*/              { 4,            16,         6,       6,     true },
        /* Format_ASTC_6x6_SRGB_Block = 166*/               { 4,            16,         6,       6,     true },
        /* Format_ASTC_8x5_UNorm_Block = 167*/              { 4,            16,         8,       5,     true },
        /* Format_ASTC_8x5_SRGB_Block = 168*/               { 4,            16,         8,       5,     true },
        /* Format_ASTC_8x6_UNorm_Block = 169*/              { 4,            16,         8,       6,     true },
        /* Format_ASTC_8x6_SRGB_Block = 170*/               { 4,            16,         8,       6,     true },
        /* Format_ASTC_8x8_UNorm_Block = 171*/              { 4,            16,         8,       8,     true },
        /* Format_ASTC_8x8_SRGB_Block = 172*/               { 4,            16,         8,       8,     true },
        /* Format_ASTC_10x5_UNorm_Block = 173*/             { 4,            16,         10,      5,     true },
        /* Format_ASTC_10x5_SRGB_Block = 174*/              { 4,            16,         10,      5,     true },
        /* Format_ASTC_10x6_UNorm_Block = 175*/             { 4,            16,         10,      6,     true },
        /* Format_ASTC_10x6_SRGB_Block = 176*/              { 4,            16,         10,      6,     true },
        /* Format_ASTC_10x8_UNorm_Block = 177*/             { 4,            16,         10,      8,     true },
        /* Format_ASTC_10x8_SRGB_Block = 178*/              { 4,            16,         10,      8,     true },
        /* Format_ASTC_10x10_UNorm_Block = 179*/            { 4,            16,         10,      10,    true },
        /* Format_ASTC_10x10_SRGB_Block = 180*/             { 4,            16,         10,      10,    true },
        /* Format_ASTC_12x10_UNorm_Block = 181*/            { 4,            16,         12,      10,    true },
        /* Format_ASTC_12x10_SRGB_Block = 182*/             { 4,            16,         12,      10,    true },
        /* Format_ASTC_12x12_UNorm_Block = 183*/            { 4,            16,         12,      12,    true },
        /* Format_ASTC_12x12_SRGB_Block = 184*/             { 4,            16,         12,      12,    true }
    };

    u32 ImageFormat::getFormatCountComponent(Format format)
    {
        return k_formatList[format][0];
    }

    u32 ImageFormat::getFormatBlockSize(Format format)
    {
        return k_formatList[format][1];
    }

    core::Dimension2D ImageFormat::getBlockDimension(Format format)
    {
        return core::Dimension2D(k_formatList[format][2], k_formatList[format][3]);
    }

    bool ImageFormat::isFormatCompressed(Format format)
    {
        return k_formatList[format][4];
    }


} //namespace renderer
} //namespace v3d
