#include "stdafx.h"

#include "TextureLoader.h"

#include "Core.Graphics/Device/Texture/SurfaceFormat.h"

#include "Core/IO/VFS/VirtualFileSystemStream.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace DDS {
//----------------------------------------------------------------------------
// http://msdn.microsoft.com/en-us/library/windows/apps/jj651550.aspx#view_codecpp
//----------------------------------------------------------------------------
// Function for loading a DDS texture and creating a Direct3D 11 runtime resource for it
//
// Note this function is useful as a light-weight runtime loader for DDS files. For
// a full-featured DDS file reader, writer, and texture processing pipeline see
// the 'Texconv' sample and the 'DirectXTex' library.
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Macros
//--------------------------------------------------------------------------------------
#ifndef MAKEFOURCC
    #define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
                ((u32)(byte)(ch0) | ((u32)(byte)(ch1) << 8) |       \
                ((u32)(byte)(ch2) << 16) | ((u32)(byte)(ch3) << 24))
#endif /* defined(MAKEFOURCC) */

//--------------------------------------------------------------------------------------
// DDS file structure definitions
//
// See DDS.h in the 'Texconv' sample and the 'DirectXTex' library
//--------------------------------------------------------------------------------------
#pragma pack(push, 1)

#define DDS_MAGIC 0x20534444 // "DDS "

#define DDS_FOURCC      0x00000004  // DDPF_FOURCC
#define DDS_RGB         0x00000040  // DDPF_RGB
#define DDS_RGBA        0x00000041  // DDPF_RGB | DDPF_ALPHAPIXELS
#define DDS_LUMINANCE   0x00020000  // DDPF_LUMINANCE
#define DDS_LUMINANCEA  0x00020001  // DDPF_LUMINANCE | DDPF_ALPHAPIXELS
#define DDS_ALPHA       0x00000002  // DDPF_ALPHA
#define DDS_PAL8        0x00000020  // DDPF_PALETTEINDEXED8

#define DDS_HEADER_FLAGS_TEXTURE        0x00001007  // DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT
#define DDS_HEADER_FLAGS_MIPMAP         0x00020000  // DDSD_MIPMAPCOUNT
#define DDS_HEADER_FLAGS_VOLUME         0x00800000  // DDSD_DEPTH
#define DDS_HEADER_FLAGS_PITCH          0x00000008  // DDSD_PITCH
#define DDS_HEADER_FLAGS_LINEARSIZE     0x00080000  // DDSD_LINEARSIZE

#define DDS_HEIGHT 0x00000002 // DDSD_HEIGHT
#define DDS_WIDTH  0x00000004 // DDSD_WIDTH

#define DDS_SURFACE_FLAGS_TEXTURE 0x00001000 // DDSCAPS_TEXTURE
#define DDS_SURFACE_FLAGS_MIPMAP  0x00400008 // DDSCAPS_COMPLEX | DDSCAPS_MIPMAP
#define DDS_SURFACE_FLAGS_CUBEMAP 0x00000008 // DDSCAPS_COMPLEX

#define DDS_CUBEMAP_POSITIVEX 0x00000600 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEX
#define DDS_CUBEMAP_NEGATIVEX 0x00000a00 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEX
#define DDS_CUBEMAP_POSITIVEY 0x00001200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEY
#define DDS_CUBEMAP_NEGATIVEY 0x00002200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEY
#define DDS_CUBEMAP_POSITIVEZ 0x00004200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEZ
#define DDS_CUBEMAP_NEGATIVEZ 0x00008200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEZ

#define DDS_CUBEMAP_ALLFACES (DDS_CUBEMAP_POSITIVEX | DDS_CUBEMAP_NEGATIVEX |\
                              DDS_CUBEMAP_POSITIVEY | DDS_CUBEMAP_NEGATIVEY |\
                              DDS_CUBEMAP_POSITIVEZ | DDS_CUBEMAP_NEGATIVEZ)

#define DDS_CUBEMAP 0x00000200 // DDSCAPS2_CUBEMAP

#define DDS_FLAGS_VOLUME 0x00200000 // DDSCAPS2_VOLUME

enum DDS_DXGI_FORMAT : u32 {
    DDS_DXGI_FORMAT_UNKNOWN = 0,
    DDS_DXGI_FORMAT_R8G8B8A8_UNORM,
    DDS_DXGI_FORMAT_B8G8R8A8_UNORM,
    DDS_DXGI_FORMAT_B8G8R8X8_UNORM,
    DDS_DXGI_FORMAT_R10G10B10A2_UNORM,
    DDS_DXGI_FORMAT_R16G16_UNORM,
    DDS_DXGI_FORMAT_R32_FLOAT,
    DDS_DXGI_FORMAT_B5G5R5A1_UNORM,
    DDS_DXGI_FORMAT_B5G6R5_UNORM,
    DDS_DXGI_FORMAT_B4G4R4A4_UNORM,
    DDS_DXGI_FORMAT_R8_UNORM,
    DDS_DXGI_FORMAT_R16_UNORM,
    DDS_DXGI_FORMAT_R8G8_UNORM,
    DDS_DXGI_FORMAT_A8_UNORM,
    DDS_DXGI_FORMAT_BC1_UNORM,
    DDS_DXGI_FORMAT_BC2_UNORM,
    DDS_DXGI_FORMAT_BC3_UNORM,
    DDS_DXGI_FORMAT_BC4_UNORM,
    DDS_DXGI_FORMAT_BC4_SNORM,
    DDS_DXGI_FORMAT_BC5_UNORM,
    DDS_DXGI_FORMAT_BC5_SNORM,
    DDS_DXGI_FORMAT_ATI2,
    DDS_DXGI_FORMAT_R8G8_B8G8_UNORM,
    DDS_DXGI_FORMAT_G8R8_G8B8_UNORM,
    DDS_DXGI_FORMAT_R16G16B16A16_UNORM,
    DDS_DXGI_FORMAT_R16G16B16A16_SNORM,
    DDS_DXGI_FORMAT_R16_FLOAT,
    DDS_DXGI_FORMAT_R16G16_FLOAT,
    DDS_DXGI_FORMAT_R16G16B16A16_FLOAT,
    DDS_DXGI_FORMAT_R32G32_FLOAT,
    DDS_DXGI_FORMAT_R32G32B32A32_FLOAT,
};

enum DDS_D3D11_DXGI_FORMAT : u32 {
    DDS_D3D11_DXGI_FORMAT_UNKNOWN                       = 0,
    DDS_D3D11_DXGI_FORMAT_R32G32B32A32_TYPELESS       = 1,
    DDS_D3D11_DXGI_FORMAT_R32G32B32A32_FLOAT          = 2,
    DDS_D3D11_DXGI_FORMAT_R32G32B32A32_UINT           = 3,
    DDS_D3D11_DXGI_FORMAT_R32G32B32A32_SINT           = 4,
    DDS_D3D11_DXGI_FORMAT_R32G32B32_TYPELESS          = 5,
    DDS_D3D11_DXGI_FORMAT_R32G32B32_FLOAT             = 6,
    DDS_D3D11_DXGI_FORMAT_R32G32B32_UINT              = 7,
    DDS_D3D11_DXGI_FORMAT_R32G32B32_SINT              = 8,
    DDS_D3D11_DXGI_FORMAT_R16G16B16A16_TYPELESS       = 9,
    DDS_D3D11_DXGI_FORMAT_R16G16B16A16_FLOAT          = 10,
    DDS_D3D11_DXGI_FORMAT_R16G16B16A16_UNORM          = 11,
    DDS_D3D11_DXGI_FORMAT_R16G16B16A16_UINT           = 12,
    DDS_D3D11_DXGI_FORMAT_R16G16B16A16_SNORM          = 13,
    DDS_D3D11_DXGI_FORMAT_R16G16B16A16_SINT           = 14,
    DDS_D3D11_DXGI_FORMAT_R32G32_TYPELESS             = 15,
    DDS_D3D11_DXGI_FORMAT_R32G32_FLOAT                = 16,
    DDS_D3D11_DXGI_FORMAT_R32G32_UINT                 = 17,
    DDS_D3D11_DXGI_FORMAT_R32G32_SINT                 = 18,
    DDS_D3D11_DXGI_FORMAT_R32G8X24_TYPELESS           = 19,
    DDS_D3D11_DXGI_FORMAT_D32_FLOAT_S8X24_UINT        = 20,
    DDS_D3D11_DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS    = 21,
    DDS_D3D11_DXGI_FORMAT_X32_TYPELESS_G8X24_UINT     = 22,
    DDS_D3D11_DXGI_FORMAT_R10G10B10A2_TYPELESS        = 23,
    DDS_D3D11_DXGI_FORMAT_R10G10B10A2_UNORM           = 24,
    DDS_D3D11_DXGI_FORMAT_R10G10B10A2_UINT            = 25,
    DDS_D3D11_DXGI_FORMAT_R11G11B10_FLOAT             = 26,
    DDS_D3D11_DXGI_FORMAT_R8G8B8A8_TYPELESS           = 27,
    DDS_D3D11_DXGI_FORMAT_R8G8B8A8_UNORM              = 28,
    DDS_D3D11_DXGI_FORMAT_R8G8B8A8_UNORM_SRGB         = 29,
    DDS_D3D11_DXGI_FORMAT_R8G8B8A8_UINT               = 30,
    DDS_D3D11_DXGI_FORMAT_R8G8B8A8_SNORM              = 31,
    DDS_D3D11_DXGI_FORMAT_R8G8B8A8_SINT               = 32,
    DDS_D3D11_DXGI_FORMAT_R16G16_TYPELESS             = 33,
    DDS_D3D11_DXGI_FORMAT_R16G16_FLOAT                = 34,
    DDS_D3D11_DXGI_FORMAT_R16G16_UNORM                = 35,
    DDS_D3D11_DXGI_FORMAT_R16G16_UINT                 = 36,
    DDS_D3D11_DXGI_FORMAT_R16G16_SNORM                = 37,
    DDS_D3D11_DXGI_FORMAT_R16G16_SINT                 = 38,
    DDS_D3D11_DXGI_FORMAT_R32_TYPELESS                = 39,
    DDS_D3D11_DXGI_FORMAT_D32_FLOAT                   = 40,
    DDS_D3D11_DXGI_FORMAT_R32_FLOAT                   = 41,
    DDS_D3D11_DXGI_FORMAT_R32_UINT                    = 42,
    DDS_D3D11_DXGI_FORMAT_R32_SINT                    = 43,
    DDS_D3D11_DXGI_FORMAT_R24G8_TYPELESS              = 44,
    DDS_D3D11_DXGI_FORMAT_D24_UNORM_S8_UINT           = 45,
    DDS_D3D11_DXGI_FORMAT_R24_UNORM_X8_TYPELESS       = 46,
    DDS_D3D11_DXGI_FORMAT_X24_TYPELESS_G8_UINT        = 47,
    DDS_D3D11_DXGI_FORMAT_R8G8_TYPELESS               = 48,
    DDS_D3D11_DXGI_FORMAT_R8G8_UNORM                  = 49,
    DDS_D3D11_DXGI_FORMAT_R8G8_UINT                   = 50,
    DDS_D3D11_DXGI_FORMAT_R8G8_SNORM                  = 51,
    DDS_D3D11_DXGI_FORMAT_R8G8_SINT                   = 52,
    DDS_D3D11_DXGI_FORMAT_R16_TYPELESS                = 53,
    DDS_D3D11_DXGI_FORMAT_R16_FLOAT                   = 54,
    DDS_D3D11_DXGI_FORMAT_D16_UNORM                   = 55,
    DDS_D3D11_DXGI_FORMAT_R16_UNORM                   = 56,
    DDS_D3D11_DXGI_FORMAT_R16_UINT                    = 57,
    DDS_D3D11_DXGI_FORMAT_R16_SNORM                   = 58,
    DDS_D3D11_DXGI_FORMAT_R16_SINT                    = 59,
    DDS_D3D11_DXGI_FORMAT_R8_TYPELESS                 = 60,
    DDS_D3D11_DXGI_FORMAT_R8_UNORM                    = 61,
    DDS_D3D11_DXGI_FORMAT_R8_UINT                     = 62,
    DDS_D3D11_DXGI_FORMAT_R8_SNORM                    = 63,
    DDS_D3D11_DXGI_FORMAT_R8_SINT                     = 64,
    DDS_D3D11_DXGI_FORMAT_A8_UNORM                    = 65,
    DDS_D3D11_DXGI_FORMAT_R1_UNORM                    = 66,
    DDS_D3D11_DXGI_FORMAT_R9G9B9E5_SHAREDEXP          = 67,
    DDS_D3D11_DXGI_FORMAT_R8G8_B8G8_UNORM             = 68,
    DDS_D3D11_DXGI_FORMAT_G8R8_G8B8_UNORM             = 69,
    DDS_D3D11_DXGI_FORMAT_BC1_TYPELESS                = 70,
    DDS_D3D11_DXGI_FORMAT_BC1_UNORM                   = 71,
    DDS_D3D11_DXGI_FORMAT_BC1_UNORM_SRGB              = 72,
    DDS_D3D11_DXGI_FORMAT_BC2_TYPELESS                = 73,
    DDS_D3D11_DXGI_FORMAT_BC2_UNORM                   = 74,
    DDS_D3D11_DXGI_FORMAT_BC2_UNORM_SRGB              = 75,
    DDS_D3D11_DXGI_FORMAT_BC3_TYPELESS                = 76,
    DDS_D3D11_DXGI_FORMAT_BC3_UNORM                   = 77,
    DDS_D3D11_DXGI_FORMAT_BC3_UNORM_SRGB              = 78,
    DDS_D3D11_DXGI_FORMAT_BC4_TYPELESS                = 79,
    DDS_D3D11_DXGI_FORMAT_BC4_UNORM                   = 80,
    DDS_D3D11_DXGI_FORMAT_BC4_SNORM                   = 81,
    DDS_D3D11_DXGI_FORMAT_BC5_TYPELESS                = 82,
    DDS_D3D11_DXGI_FORMAT_BC5_UNORM                   = 83,
    DDS_D3D11_DXGI_FORMAT_BC5_SNORM                   = 84,
    DDS_D3D11_DXGI_FORMAT_B5G6R5_UNORM                = 85,
    DDS_D3D11_DXGI_FORMAT_B5G5R5A1_UNORM              = 86,
    DDS_D3D11_DXGI_FORMAT_B8G8R8A8_UNORM              = 87,
    DDS_D3D11_DXGI_FORMAT_B8G8R8X8_UNORM              = 88,
    DDS_D3D11_DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM  = 89,
    DDS_D3D11_DXGI_FORMAT_B8G8R8A8_TYPELESS           = 90,
    DDS_D3D11_DXGI_FORMAT_B8G8R8A8_UNORM_SRGB         = 91,
    DDS_D3D11_DXGI_FORMAT_B8G8R8X8_TYPELESS           = 92,
    DDS_D3D11_DXGI_FORMAT_B8G8R8X8_UNORM_SRGB         = 93,
    DDS_D3D11_DXGI_FORMAT_BC6H_TYPELESS               = 94,
    DDS_D3D11_DXGI_FORMAT_BC6H_UF16                   = 95,
    DDS_D3D11_DXGI_FORMAT_BC6H_SF16                   = 96,
    DDS_D3D11_DXGI_FORMAT_BC7_TYPELESS                = 97,
    DDS_D3D11_DXGI_FORMAT_BC7_UNORM                   = 98,
    DDS_D3D11_DXGI_FORMAT_BC7_UNORM_SRGB              = 99,
};

enum DDS_D3D11_RESOURCE_DIMENSION : u32 {
    DDS_D3D11_RESOURCE_DIMENSION_UNKNOWN    = 0,
    DDS_D3D11_RESOURCE_DIMENSION_BUFFER        = 1,
    DDS_D3D11_RESOURCE_DIMENSION_TEXTURE1D    = 2,
    DDS_D3D11_RESOURCE_DIMENSION_TEXTURE2D    = 3,
    DDS_D3D11_RESOURCE_DIMENSION_TEXTURE3D    = 4
};

enum DDS_D3D11_RESOURCE_MISC_FLAG : u32 {
    DDS_D3D11_RESOURCE_MISC_TEXTURECUBE    = 0x4L,
};

#define    DDS_D3D11_REQ_MIP_LEVELS ( 15 )
#define    DDS_D3D11_REQ_TEXTURE1D_ARRAY_AXIS_DIMENSION ( 2048 )
#define    DDS_D3D11_REQ_TEXTURE1D_U_DIMENSION    ( 16384 )
#define    DDS_D3D11_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION ( 2048 )
#define    DDS_D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION ( 16384 )
#define    DDS_D3D11_REQ_TEXTURE3D_U_V_OR_W_DIMENSION ( 2048 )
#define    DDS_D3D11_REQ_TEXTURECUBE_DIMENSION ( 16384 )

struct DDS_PIXELFORMAT {
    u32  size;
    u32  flags;
    u32  fourCC;
    u32  RGBBitCount;
    u32  RBitMask;
    u32  GBitMask;
    u32  BBitMask;
    u32  ABitMask;
};

struct DDS_HEADER {
    u32          size;
    u32          flags;
    u32          height;
    u32          width;
    u32          pitchOrLinearSize;
    u32          depth; // only if DDS_HEADER_FLAGS_VOLUME is set in flags
    u32          mipMapCount;
    u32          reserved1[11];
    DDS_PIXELFORMAT ddspf;
    u32          caps;
    u32          caps2;
    u32          caps3;
    u32          caps4;
    u32          reserved2;
};

struct DDS_HEADER_DXT10 {
    DDS_DXGI_FORMAT dxgiFormat;
    u32      resourceDimension;
    u32      miscFlag; // see DDS_D3D11_RESOURCE_MISC_FLAG
    u32      arraySize;
    u32      reserved;
};

#pragma pack(pop)

//--------------------------------------------------------------------------------------
#define ISBITMASK(r, g, b, a) (ddpf.RBitMask == r && ddpf.GBitMask == g && ddpf.BBitMask == b && ddpf.ABitMask == a)

static DDS_DXGI_FORMAT GetDXGIFormat(const DDS_PIXELFORMAT& ddpf)
{
    if (ddpf.flags & DDS_RGB)
    {
        // Note that sRGB formats are written using the "DX10" extended header

        switch (ddpf.RGBBitCount)
        {
        case 32:
            if (ISBITMASK(0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000))
            {
                return DDS_DXGI_FORMAT_R8G8B8A8_UNORM;
            }

            if (ISBITMASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000))
            {
                return DDS_DXGI_FORMAT_B8G8R8A8_UNORM;
            }

            if (ISBITMASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000))
            {
                return DDS_DXGI_FORMAT_B8G8R8X8_UNORM;
            }

            // No DXGI format maps to ISBITMASK(0x000000ff, 0x0000ff00, 0x00ff0000, 0x00000000) aka D3DFMT_X8B8G8R8

            // Note that many common DDS reader/writers (including D3DX) swap the
            // the RED/BLUE masks for 10:10:10:2 formats. We assumme
            // below that the 'backwards' header mask is being used since it is most
            // likely written by D3DX. The more robust solution is to use the 'DX10'
            // header extension and specify the DDS_DXGI_FORMAT_R10G10B10A2_UNORM format directly

            // For 'correct' writers, this should be 0x000003ff, 0x000ffc00, 0x3ff00000 for RGB data
            if (ISBITMASK(0x3ff00000, 0x000ffc00, 0x000003ff, 0xc0000000))
            {
                return DDS_DXGI_FORMAT_R10G10B10A2_UNORM;
            }

            // No DXGI format maps to ISBITMASK(0x000003ff, 0x000ffc00, 0x3ff00000, 0xc0000000) aka D3DFMT_A2R10G10B10

            if (ISBITMASK(0x0000ffff, 0xffff0000, 0x00000000, 0x00000000))
            {
                return DDS_DXGI_FORMAT_R16G16_UNORM;
            }

            if (ISBITMASK(0xffffffff, 0x00000000, 0x00000000, 0x00000000))
            {
                // Only 32-bit color channel format in D3D9 was R32F
                return DDS_DXGI_FORMAT_R32_FLOAT; // D3DX writes this out as a FFourCC of 114
            }
            break;

        case 24:
            // No 24bpp DXGI formats aka D3DFMT_R8G8B8
            break;

        case 16:
            if (ISBITMASK(0x7c00, 0x03e0, 0x001f, 0x8000))
            {
                return DDS_DXGI_FORMAT_B5G5R5A1_UNORM;
            }
            if (ISBITMASK(0xf800, 0x07e0, 0x001f, 0x0000))
            {
                return DDS_DXGI_FORMAT_B5G6R5_UNORM;
            }

            // No DXGI format maps to ISBITMASK(0x7c00, 0x03e0, 0x001f, 0x0000) aka D3DFMT_X1R5G5B5
            if (ISBITMASK(0x0f00, 0x00f0, 0x000f, 0xf000))
            {
                return DDS_DXGI_FORMAT_B4G4R4A4_UNORM;
            }

            // No DXGI format maps to ISBITMASK(0x0f00, 0x00f0, 0x000f, 0x0000) aka D3DFMT_X4R4G4B4

            // No 3:3:2, 3:3:2:8, or paletted DXGI formats aka D3DFMT_A8R3G3B2, D3DFMT_R3G3B2, D3DFMT_P8, D3DFMT_A8P8, etc.
            break;
        }
    }
    else if (ddpf.flags & DDS_LUMINANCE)
    {
        if (8 == ddpf.RGBBitCount)
        {
            if (ISBITMASK(0x000000ff, 0x00000000, 0x00000000, 0x00000000))
            {
                return DDS_DXGI_FORMAT_R8_UNORM; // D3DX10/11 writes this out as DX10 extension
            }

            // No DXGI format maps to ISBITMASK(0x0f, 0x00, 0x00, 0xf0) aka D3DFMT_A4L4
        }

        if (16 == ddpf.RGBBitCount)
        {
            if (ISBITMASK(0x0000ffff, 0x00000000, 0x00000000, 0x00000000))
            {
                return DDS_DXGI_FORMAT_R16_UNORM; // D3DX10/11 writes this out as DX10 extension
            }
            if (ISBITMASK(0x000000ff, 0x00000000, 0x00000000, 0x0000ff00))
            {
                return DDS_DXGI_FORMAT_R8G8_UNORM; // D3DX10/11 writes this out as DX10 extension
            }
        }
    }
    else if (ddpf.flags & DDS_ALPHA)
    {
        if (8 == ddpf.RGBBitCount)
        {
            return DDS_DXGI_FORMAT_A8_UNORM;
        }
    }
    else if (ddpf.flags & DDS_FOURCC)
    {
        if (MAKEFOURCC('D', 'X', 'T', '1') == ddpf.fourCC)
        {
            return DDS_DXGI_FORMAT_BC1_UNORM;
        }
        if (MAKEFOURCC('D', 'X', 'T', '3') == ddpf.fourCC)
        {
            return DDS_DXGI_FORMAT_BC2_UNORM;
        }
        if (MAKEFOURCC('D', 'X', 'T', '5') == ddpf.fourCC)
        {
            return DDS_DXGI_FORMAT_BC3_UNORM;
        }

        // While pre-mulitplied alpha isn't directly supported by the DXGI formats,
        // they are basically the same as these BC formats so they can be mapped
        if (MAKEFOURCC('D', 'X', 'T', '2') == ddpf.fourCC)
        {
            return DDS_DXGI_FORMAT_BC2_UNORM;
        }
        if (MAKEFOURCC('D', 'X', 'T', '4') == ddpf.fourCC)
        {
            return DDS_DXGI_FORMAT_BC3_UNORM;
        }

        if (MAKEFOURCC('A', 'T', 'I', '1') == ddpf.fourCC)
        {
            return DDS_DXGI_FORMAT_BC4_UNORM;
        }
        if (MAKEFOURCC('B', 'C', '4', 'U') == ddpf.fourCC)
        {
            return DDS_DXGI_FORMAT_BC4_UNORM;
        }
        if (MAKEFOURCC('B', 'C', '4', 'S') == ddpf.fourCC)
        {
            return DDS_DXGI_FORMAT_BC4_SNORM;
        }

        if (MAKEFOURCC('A', 'T', 'I', '2') == ddpf.fourCC)
        {
            return DDS_DXGI_FORMAT_ATI2;
        }
        if (MAKEFOURCC('B', 'C', '5', 'U') == ddpf.fourCC)
        {
            return DDS_DXGI_FORMAT_BC5_UNORM;
        }
        if (MAKEFOURCC('B', 'C', '5', 'S') == ddpf.fourCC)
        {
            return DDS_DXGI_FORMAT_BC5_SNORM;
        }

        // BC6H and BC7 are written using the "DX10" extended header

        if (MAKEFOURCC('R', 'G', 'B', 'G') == ddpf.fourCC)
        {
            return DDS_DXGI_FORMAT_R8G8_B8G8_UNORM;
        }
        if (MAKEFOURCC('G', 'R', 'G', 'B') == ddpf.fourCC)
        {
            return DDS_DXGI_FORMAT_G8R8_G8B8_UNORM;
        }

        // Check for D3DFORMAT enums being set here
        switch (ddpf.fourCC)
        {
        case 36: // D3DFMT_A16B16G16R16
            return DDS_DXGI_FORMAT_R16G16B16A16_UNORM;

        case 110: // D3DFMT_Q16W16V16U16
            return DDS_DXGI_FORMAT_R16G16B16A16_SNORM;

        case 111: // D3DFMT_R16F
            return DDS_DXGI_FORMAT_R16_FLOAT;

        case 112: // D3DFMT_G16R16F
            return DDS_DXGI_FORMAT_R16G16_FLOAT;

        case 113: // D3DFMT_A16B16G16R16F
            return DDS_DXGI_FORMAT_R16G16B16A16_FLOAT;

        case 114: // D3DFMT_R32F
            return DDS_DXGI_FORMAT_R32_FLOAT;

        case 115: // D3DFMT_G32R32F
            return DDS_DXGI_FORMAT_R32G32_FLOAT;

        case 116: // D3DFMT_A32B32G32R32F
            return DDS_DXGI_FORMAT_R32G32B32A32_FLOAT;
        }
    }

    return DDS_DXGI_FORMAT_UNKNOWN;
}
//----------------------------------------------------------------------------
bool ReadTextureHeader(FTextureHeader& header, IVirtualFileSystemIStream *stream) {
    u32 dwMagicNumber;
    if (!stream->ReadPOD(&dwMagicNumber) || DDS_MAGIC != dwMagicNumber)
        return false;

    DDS_HEADER ddsHeader;
    if (!stream->ReadPOD(&ddsHeader))
        return false;

    // Verify header to validate DDS file
    if (ddsHeader.size != sizeof(DDS_HEADER) ||
        ddsHeader.ddspf.size != sizeof(DDS_PIXELFORMAT)) {
        return false;
    }

    // Check for DX10 extension
    bool bDXT10Header = false;
    DDS_HEADER_DXT10 ddsHeaderDX10;

    if ((ddsHeader.ddspf.flags & DDS_FOURCC) &&
        (MAKEFOURCC('D', 'X', '1', '0') == ddsHeader.ddspf.fourCC)) {
        if (!stream->ReadPOD(&ddsHeaderDX10))
            return false;

        bDXT10Header = true;
    }

    header.Width = ddsHeader.width;
    header.Height = ddsHeader.height;
    header.Depth = ddsHeader.depth;
    header.ArraySize = 1;
    header.IsCubeMap = false;

    u32 resDim = DDS_D3D11_RESOURCE_DIMENSION_UNKNOWN;
    DDS_DXGI_FORMAT format = DDS_DXGI_FORMAT_UNKNOWN;

    header.LevelCount = ddsHeader.mipMapCount;
    if (0 == header.LevelCount) {
        header.LevelCount = 1;
    }

    if (bDXT10Header) {
        header.ArraySize = ddsHeaderDX10.arraySize;
        if (header.ArraySize == 0)
            return false;

        format = ddsHeaderDX10.dxgiFormat;

        switch (ddsHeaderDX10.resourceDimension)
        {
        case DDS_D3D11_RESOURCE_DIMENSION_TEXTURE1D:
            // D3DX writes 1D textures with a fixed Height of 1
            if ((ddsHeader.flags & DDS_HEIGHT) && header.Height != 1)
                return false;

            header.Height = header.Depth = 1;
            break;

        case DDS_D3D11_RESOURCE_DIMENSION_TEXTURE2D:
            if (ddsHeaderDX10.miscFlag & DDS_D3D11_RESOURCE_MISC_TEXTURECUBE) {
                header.ArraySize *= 6;
                header.IsCubeMap = true;
            }

            header.Depth = 1;
            break;

        case DDS_D3D11_RESOURCE_DIMENSION_TEXTURE3D:
            if (!(ddsHeader.flags & DDS_HEADER_FLAGS_VOLUME))
                return false;

            if (header.ArraySize > 1)
                return false;

            Assert(header.Depth > 1);
            break;

        default:
            return false;
        }

        resDim = ddsHeaderDX10.resourceDimension;

        switch (DDS_D3D11_DXGI_FORMAT(format))
        {
        case PPE::Engine::DDS::DDS_D3D11_DXGI_FORMAT_R32G32B32A32_FLOAT:
            header.Format = PPE::Graphics::FSurfaceFormat::R32G32B32A32_F;
            break;
        case PPE::Engine::DDS::DDS_D3D11_DXGI_FORMAT_R32G32B32A32_UINT:
        case PPE::Engine::DDS::DDS_D3D11_DXGI_FORMAT_R32G32B32A32_SINT:
            header.Format = PPE::Graphics::FSurfaceFormat::R32G32B32A32;
            break;
        case PPE::Engine::DDS::DDS_D3D11_DXGI_FORMAT_R16G16B16A16_FLOAT:
            header.Format = PPE::Graphics::FSurfaceFormat::R16G16B16A16_F;
            break;
        case PPE::Engine::DDS::DDS_D3D11_DXGI_FORMAT_R16G16B16A16_UNORM:
        case PPE::Engine::DDS::DDS_D3D11_DXGI_FORMAT_R16G16B16A16_UINT:
        case PPE::Engine::DDS::DDS_D3D11_DXGI_FORMAT_R16G16B16A16_SNORM:
        case PPE::Engine::DDS::DDS_D3D11_DXGI_FORMAT_R16G16B16A16_SINT:
            header.Format = PPE::Graphics::FSurfaceFormat::R16G16B16A16;
            break;
        case PPE::Engine::DDS::DDS_D3D11_DXGI_FORMAT_R32G32_FLOAT:
            header.Format = PPE::Graphics::FSurfaceFormat::R32G32_F;
            break;
        case PPE::Engine::DDS::DDS_D3D11_DXGI_FORMAT_R32G32_UINT:
        case PPE::Engine::DDS::DDS_D3D11_DXGI_FORMAT_R32G32_SINT:
            header.Format = PPE::Graphics::FSurfaceFormat::R32G32;
            break;
        case PPE::Engine::DDS::DDS_D3D11_DXGI_FORMAT_R10G10B10A2_UNORM:
        case PPE::Engine::DDS::DDS_D3D11_DXGI_FORMAT_R10G10B10A2_UINT:
            header.Format = PPE::Graphics::FSurfaceFormat::R10G10B10A2;
            break;
        case PPE::Engine::DDS::DDS_D3D11_DXGI_FORMAT_R11G11B10_FLOAT:
            header.Format = PPE::Graphics::FSurfaceFormat::R11G11B10;
            break;
        case PPE::Engine::DDS::DDS_D3D11_DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
            header.Format = PPE::Graphics::FSurfaceFormat::R8G8B8A8_SRGB;
            break;
        case PPE::Engine::DDS::DDS_D3D11_DXGI_FORMAT_R8G8B8A8_UNORM:
        case PPE::Engine::DDS::DDS_D3D11_DXGI_FORMAT_R8G8B8A8_UINT:
        case PPE::Engine::DDS::DDS_D3D11_DXGI_FORMAT_R8G8B8A8_SNORM:
        case PPE::Engine::DDS::DDS_D3D11_DXGI_FORMAT_R8G8B8A8_SINT:
            header.Format = PPE::Graphics::FSurfaceFormat::R8G8B8A8;
            break;
        case PPE::Engine::DDS::DDS_D3D11_DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
            header.Format = PPE::Graphics::FSurfaceFormat::B8G8R8A8_SRGB;
            break;
        case PPE::Engine::DDS::DDS_D3D11_DXGI_FORMAT_B8G8R8A8_UNORM:
            header.Format = PPE::Graphics::FSurfaceFormat::B8G8R8A8;
            break;
        case PPE::Engine::DDS::DDS_D3D11_DXGI_FORMAT_R16G16_FLOAT:
            header.Format = PPE::Graphics::FSurfaceFormat::R16G16_F;
            break;
        case PPE::Engine::DDS::DDS_D3D11_DXGI_FORMAT_R16G16_UNORM:
        case PPE::Engine::DDS::DDS_D3D11_DXGI_FORMAT_R16G16_UINT:
        case PPE::Engine::DDS::DDS_D3D11_DXGI_FORMAT_R16G16_SNORM:
        case PPE::Engine::DDS::DDS_D3D11_DXGI_FORMAT_R16G16_SINT:
            header.Format = PPE::Graphics::FSurfaceFormat::R16G16_F;
            break;
        case PPE::Engine::DDS::DDS_D3D11_DXGI_FORMAT_R32_FLOAT:
            header.Format = PPE::Graphics::FSurfaceFormat::R32_F;
            break;
        case PPE::Engine::DDS::DDS_D3D11_DXGI_FORMAT_R32_UINT:
        case PPE::Engine::DDS::DDS_D3D11_DXGI_FORMAT_R32_SINT:
            header.Format = PPE::Graphics::FSurfaceFormat::R32;
            break;
        case PPE::Engine::DDS::DDS_D3D11_DXGI_FORMAT_R8G8_UNORM:
        case PPE::Engine::DDS::DDS_D3D11_DXGI_FORMAT_R8G8_UINT:
        case PPE::Engine::DDS::DDS_D3D11_DXGI_FORMAT_R8G8_SNORM:
        case PPE::Engine::DDS::DDS_D3D11_DXGI_FORMAT_R8G8_SINT:
            header.Format = PPE::Graphics::FSurfaceFormat::R8G8;
            break;
        case PPE::Engine::DDS::DDS_D3D11_DXGI_FORMAT_R16_FLOAT:
            header.Format = PPE::Graphics::FSurfaceFormat::R16_F;
            break;
        case PPE::Engine::DDS::DDS_D3D11_DXGI_FORMAT_R16_UNORM:
        case PPE::Engine::DDS::DDS_D3D11_DXGI_FORMAT_R16_UINT:
        case PPE::Engine::DDS::DDS_D3D11_DXGI_FORMAT_R16_SNORM:
        case PPE::Engine::DDS::DDS_D3D11_DXGI_FORMAT_R16_SINT:
            header.Format = PPE::Graphics::FSurfaceFormat::R16;
            break;
        case PPE::Engine::DDS::DDS_D3D11_DXGI_FORMAT_R8_UNORM:
        case PPE::Engine::DDS::DDS_D3D11_DXGI_FORMAT_R8_UINT:
        case PPE::Engine::DDS::DDS_D3D11_DXGI_FORMAT_R8_SNORM:
        case PPE::Engine::DDS::DDS_D3D11_DXGI_FORMAT_R8_SINT:
            header.Format = PPE::Graphics::FSurfaceFormat::R8;
            break;
        case PPE::Engine::DDS::DDS_D3D11_DXGI_FORMAT_A8_UNORM:
            header.Format = PPE::Graphics::FSurfaceFormat::A8;
            break;
        case PPE::Engine::DDS::DDS_D3D11_DXGI_FORMAT_BC1_UNORM:
            header.Format = PPE::Graphics::FSurfaceFormat::DXT1;
            break;
        case PPE::Engine::DDS::DDS_D3D11_DXGI_FORMAT_BC1_UNORM_SRGB:
            header.Format = PPE::Graphics::FSurfaceFormat::DXT1_SRGB;
            break;
        case PPE::Engine::DDS::DDS_D3D11_DXGI_FORMAT_BC2_UNORM:
            header.Format = PPE::Graphics::FSurfaceFormat::DXT3;
            break;
        case PPE::Engine::DDS::DDS_D3D11_DXGI_FORMAT_BC2_UNORM_SRGB:
            header.Format = PPE::Graphics::FSurfaceFormat::DXT3_SRGB;
            break;
        case PPE::Engine::DDS::DDS_D3D11_DXGI_FORMAT_BC3_UNORM:
            header.Format = PPE::Graphics::FSurfaceFormat::DXT5;
            break;
        case PPE::Engine::DDS::DDS_D3D11_DXGI_FORMAT_BC3_UNORM_SRGB:
            header.Format = PPE::Graphics::FSurfaceFormat::DXT5_SRGB;
            break;
        case PPE::Engine::DDS::DDS_D3D11_DXGI_FORMAT_BC5_UNORM:
        case PPE::Engine::DDS::DDS_D3D11_DXGI_FORMAT_BC5_SNORM:
            header.Format = PPE::Graphics::FSurfaceFormat::DXN0;
            break;
        case PPE::Engine::DDS::DDS_D3D11_DXGI_FORMAT_B5G6R5_UNORM:
            header.Format = PPE::Graphics::FSurfaceFormat::R5G6B5;
            break;
        case PPE::Engine::DDS::DDS_D3D11_DXGI_FORMAT_B5G5R5A1_UNORM:
            header.Format = PPE::Graphics::FSurfaceFormat::R5G5B5A1;
            break;

        default:
            return false;
        }
    }
    else
    {
        format = GetDXGIFormat(ddsHeader.ddspf);

        if (format == DDS_DXGI_FORMAT_UNKNOWN)
            return false;

        if (ddsHeader.flags & DDS_HEADER_FLAGS_VOLUME) {
            resDim = DDS_D3D11_RESOURCE_DIMENSION_TEXTURE3D;
        }
        else
        {
            if (ddsHeader.caps2 & DDS_CUBEMAP)
            {
                // We require all six faces to be defined
                if ((ddsHeader.caps2 & DDS_CUBEMAP_ALLFACES) != DDS_CUBEMAP_ALLFACES)
                    return false;

                header.ArraySize = 6;
                header.IsCubeMap = true;
            }

            header.Depth = 1;
            resDim = DDS_D3D11_RESOURCE_DIMENSION_TEXTURE2D;

            // Note there's no way for a legacy Direct3D 9 DDS to express a '1D' texture
        }

        switch (format)
        {
        case DDS_DXGI_FORMAT_A8_UNORM: header.Format = PPE::Graphics::FSurfaceFormat::A8; break;
        case DDS_DXGI_FORMAT_BC5_SNORM: header.Format = PPE::Graphics::FSurfaceFormat::DXN0; break;
        case DDS_DXGI_FORMAT_BC1_UNORM: header.Format = PPE::Graphics::FSurfaceFormat::DXT1; break;
        case DDS_DXGI_FORMAT_BC2_UNORM: header.Format = PPE::Graphics::FSurfaceFormat::DXT3; break;
        case DDS_DXGI_FORMAT_BC3_UNORM: header.Format = PPE::Graphics::FSurfaceFormat::DXT5; break;
        case DDS_DXGI_FORMAT_ATI2: header.Format = PPE::Graphics::FSurfaceFormat::DXN0; break;
        case DDS_DXGI_FORMAT_B5G5R5A1_UNORM: header.Format = PPE::Graphics::FSurfaceFormat::R5G5B5A1; break;
        case DDS_DXGI_FORMAT_B5G6R5_UNORM: header.Format = PPE::Graphics::FSurfaceFormat::R5G6B5; break;
        case DDS_DXGI_FORMAT_R8_UNORM: header.Format = PPE::Graphics::FSurfaceFormat::R8; break;
        case DDS_DXGI_FORMAT_R8G8_UNORM: header.Format = PPE::Graphics::FSurfaceFormat::R8G8; break;
        case DDS_DXGI_FORMAT_R8G8B8A8_UNORM: header.Format = PPE::Graphics::FSurfaceFormat::R8G8B8A8; break;
        case DDS_DXGI_FORMAT_B8G8R8A8_UNORM: header.Format = PPE::Graphics::FSurfaceFormat::B8G8R8A8; break;
        case DDS_DXGI_FORMAT_R10G10B10A2_UNORM: header.Format = PPE::Graphics::FSurfaceFormat::R10G10B10A2; break;
        case DDS_DXGI_FORMAT_R16_UNORM: header.Format = PPE::Graphics::FSurfaceFormat::R16; break;
        case DDS_DXGI_FORMAT_R16G16_UNORM: header.Format = PPE::Graphics::FSurfaceFormat::R16G16; break;
        case DDS_DXGI_FORMAT_R16G16B16A16_UNORM: header.Format = PPE::Graphics::FSurfaceFormat::R16G16B16A16; break;
        case DDS_DXGI_FORMAT_R16G16B16A16_FLOAT: header.Format = PPE::Graphics::FSurfaceFormat::R16G16B16A16_F; break;
        case DDS_DXGI_FORMAT_R16G16_FLOAT: header.Format = PPE::Graphics::FSurfaceFormat::R16G16_F; break;
        case DDS_DXGI_FORMAT_R16_FLOAT: header.Format = PPE::Graphics::FSurfaceFormat::R16_F; break;
        case DDS_DXGI_FORMAT_R32G32B32A32_FLOAT: header.Format = PPE::Graphics::FSurfaceFormat::R32G32B32A32_F; break;
        case DDS_DXGI_FORMAT_R32G32_FLOAT: header.Format = PPE::Graphics::FSurfaceFormat::R32G32_F; break;
        case DDS_DXGI_FORMAT_R32_FLOAT: header.Format = PPE::Graphics::FSurfaceFormat::R32_F; break;

        // TODO : SRGB, INT, UINT
        /* case DDS_DXGI_FORMAT_BC1_UNORM_SRGB: header.Format = PPE::Graphics::FSurfaceFormat::DXT1_SRGB; break;
        case DDS_DXGI_FORMAT_BC2_UNORM_SRGB: header.Format = PPE::Graphics::FSurfaceFormat::DXT3_SRGB; break;
        case DDS_DXGI_FORMAT_BC3_UNORM_SRGB: header.Format = PPE::Graphics::FSurfaceFormat::DXT5_SRGB; break;
        case DDS_DXGI_FORMAT_R8G8B8A8_UNORM_SRGB: header.Format = PPE::Graphics::FSurfaceFormat::R8G8B8A8_SRGB; break;
        case DDS_DXGI_FORMAT_R32_UINT: header.Format = PPE::Graphics::FSurfaceFormat::R32; break;
        case DDS_DXGI_FORMAT_R32G32_UINT: header.Format = PPE::Graphics::FSurfaceFormat::R32G32; break;
        case DDS_DXGI_FORMAT_R32G32B32A32_UINT: header.Format = PPE::Graphics::FSurfaceFormat::R32G32B32A32; break; */

        default:
            return false;
        }
        Assert(header.Format);
    }

    // Bound sizes (for security purposes we don't trust DDS file metadata larger than the D3D 11.x hardware requirements)
    if (header.LevelCount > DDS_D3D11_REQ_MIP_LEVELS)
        return false;

    switch (resDim)
    {
        case DDS_D3D11_RESOURCE_DIMENSION_TEXTURE1D:
            if ((header.ArraySize > DDS_D3D11_REQ_TEXTURE1D_ARRAY_AXIS_DIMENSION) ||
                (header.Width > DDS_D3D11_REQ_TEXTURE1D_U_DIMENSION))
                return false;

            break;

        case DDS_D3D11_RESOURCE_DIMENSION_TEXTURE2D:
            if (header.IsCubeMap)
            {
                // This is the right bound because we set arraySize to (NumCubes*6) above
                if ((header.ArraySize > DDS_D3D11_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION) ||
                    (header.Width > DDS_D3D11_REQ_TEXTURECUBE_DIMENSION) ||
                    (header.Height > DDS_D3D11_REQ_TEXTURECUBE_DIMENSION))
                    return false;
            }
            else if ((header.ArraySize > DDS_D3D11_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION) ||
                    (header.Width > DDS_D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION) ||
                    (header.Height > DDS_D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION))
            {
                return false;
            }
            break;

        case DDS_D3D11_RESOURCE_DIMENSION_TEXTURE3D:
            if ((header.ArraySize > 1) ||
                (header.Width > DDS_D3D11_REQ_TEXTURE3D_U_V_OR_W_DIMENSION) ||
                (header.Height > DDS_D3D11_REQ_TEXTURE3D_U_V_OR_W_DIMENSION) ||
                (header.Depth > DDS_D3D11_REQ_TEXTURE3D_U_V_OR_W_DIMENSION))
                return false;

            break;
    }

    const size_t dataSizeInBytes =
        header.Format->SizeOfTexture2DInBytes(header.Width, header.Height, header.LevelCount) *
        header.Depth *
        header.ArraySize;

    header.SizeInBytes = checked_cast<u32>(dataSizeInBytes);

    const ptrdiff_t dataOffset = sizeof(u32) + sizeof(DDS_HEADER) + (bDXT10Header ? sizeof(DDS_HEADER_DXT10) : 0);
    AssertRelease(stream->TellI() == dataOffset);

    return true;
}
//----------------------------------------------------------------------------
bool ReadTextureData(const FTextureHeader& header, const TMemoryView<u8>& pixels, IVirtualFileSystemIStream *stream) {
    Assert(stream);
    Assert(pixels.SizeInBytes() == header.SizeInBytes);

    if (header.SizeInBytes != stream->ReadSome(pixels.Pointer(), pixels.SizeInBytes()))
        return false;

    return true;
}
//----------------------------------------------------------------------------
} //!namespace DDS
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
