#pragma once

#include "Graphics.h"

namespace PPE {
    template <typename T>
    class TMemoryView;
}

namespace PPE {
namespace Graphics {
class FDeviceEncapsulator;

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class ESurfaceFormatType : u32 {
    UNKNOWN         = 0,
    A8              = 1,
    D16             = 2,
    D24S8           = 3,
    D32             = 4,
    DXN0            = 5,
    DXT1            = 6,
    DXT1_SRGB       = 7,
    DXT3            = 8,
    DXT3_SRGB       = 9,
    DXT5            = 10,
    DXT5_SRGB       = 11,
    R5G5B5A1        = 12,
    R5G6B5          = 13,
    R8              = 14,
    R8G8            = 15,
    R8G8B8A8        = 16,
    R8G8B8A8_SRGB   = 17,
    B8G8R8A8        = 18,
    B8G8R8A8_SRGB   = 19,
    R10G10B10A2     = 20,
    R11G11B10       = 21,
    R16             = 22,
    R16G16          = 23,
    R16G16B16A16    = 24,
    R16G16B16A16_F  = 25,
    R16G16_F        = 26,
    R16_F           = 27,
    R32             = 28,
    R32G32          = 29,
    R32G32B32A32    = 30,
    R32G32B32A32_F  = 31,
    R32G32_F        = 32,
    R32_F           = 33,

    __COUNT
};
//----------------------------------------------------------------------------
FStringView SurfaceFormatTypeToCStr(ESurfaceFormatType value);
//----------------------------------------------------------------------------
enum class ESurfaceFormatFlags : u32 {
    RGB             = 1 << 0,
    RA              = 1 << 1,
    Alpha           = 1 << 2,
    Luminance       = 1 << 3,
    Depth           = 1 << 4,
    Stencil         = 1 << 5,
    Bump            = 1 << 6,
    Palette         = 1 << 7,
    FloatingPoint   = 1 << 8,
    DXTC            = 1 << 9,
    GammaSpace      = 1 <<10,
};
//----------------------------------------------------------------------------
enum class ESurfaceFormatSupport : u32 {
    None            = 0,
    Checked         = 1 << 0,
    FTexture         = 1 << 1,
    FRenderTarget    = 1 << 2,
    AutoGenMipMaps  = 1 << 3,
    VertexTexture   = 1 << 4,
    FDepthStencil    = 1 << 5,

    All = Checked|FTexture|FRenderTarget|AutoGenMipMaps|VertexTexture|FDepthStencil
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FSurfaceFormat {
public:
    FSurfaceFormat(
        const char *name,
        u32 blockSize,
        u32 macroBlockBitCount,
        ESurfaceFormatType type,
        ESurfaceFormatFlags flags,
        ESurfaceFormatSupport support);
    ~FSurfaceFormat();

    const char *Name() const { return _name; }
    ESurfaceFormatType Type() const { return _type; }
    ESurfaceFormatFlags Flags() const { return _flags; }
    ESurfaceFormatSupport Support() const { return _support; }

    bool IsRGB() const { return 0 != (u32(ESurfaceFormatFlags::RGB) & u32(_flags)); }
    bool IsRA() const { return 0 != (u32(ESurfaceFormatFlags::RA) & u32(_flags)); }
    bool IsAlpha() const { return 0 != (u32(ESurfaceFormatFlags::Alpha) & u32(_flags)); }
    bool IsLuminance() const { return 0 != (u32(ESurfaceFormatFlags::Luminance) & u32(_flags)); }
    bool IsDepth() const { return 0 != (u32(ESurfaceFormatFlags::Depth) & u32(_flags)); }
    bool IsStencil() const { return 0 != (u32(ESurfaceFormatFlags::Stencil) & u32(_flags)); }
    bool IsBump() const { return 0 != (u32(ESurfaceFormatFlags::Bump) & u32(_flags)); }
    bool IsPalette() const { return 0 != (u32(ESurfaceFormatFlags::Palette) & u32(_flags)); }
    bool IsFloatingPoint() const { return 0 != (u32(ESurfaceFormatFlags::FloatingPoint) & u32(_flags)); }
    bool IsDXTC() const { return 0 != (u32(ESurfaceFormatFlags::DXTC) & u32(_flags)); }
    bool IsGammaSpace() const { return 0 != (u32(ESurfaceFormatFlags::GammaSpace) & u32(_flags)); }

    bool SupportChecked() const { return 0 != (u32(ESurfaceFormatSupport::Checked) & u32(_support)); }
    bool SupportTexture() const { Assert(SupportChecked()); return 0 != (u32(ESurfaceFormatSupport::FTexture) & u32(_support)); }
    bool SupportRenderTarget() const { Assert(SupportChecked()); return 0 != (u32(ESurfaceFormatSupport::FRenderTarget) & u32(_support)); }
    bool SupportAutoGenMipMaps() const { Assert(SupportChecked()); return 0 != (u32(ESurfaceFormatSupport::AutoGenMipMaps) & u32(_support)); }
    bool SupportVertexTexture() const { Assert(SupportChecked()); return 0 != (u32(ESurfaceFormatSupport::VertexTexture) & u32(_support)); }
    bool SupportDepthStencil() const { Assert(SupportChecked()); return 0 != (u32(ESurfaceFormatSupport::FDepthStencil) & u32(_support)); }

    u32 BlockSize() const { return _blockSize; }
    u32 MacroBlockBitCount() const { return _macroBlockBitCount; }

    u32 Pitch() const;
    u32 BitsPerPixels() const;
    u32 MacroBlockSizeInPixels() const;

    void SizeOfTexture2D(size_t *rowBytes, size_t *numRows, size_t width, size_t height) const;

    size_t SizeOfTexture2DInBytes(size_t width, size_t height) const;
    size_t SizeOfTexture2DInBytes(size_t width, size_t height, size_t levelCount) const;

    size_t SizeOfTexture2DInBytes(const uint2& widthHeight) const;
    size_t SizeOfTexture2DInBytes(const uint2& widthHeight, size_t levelCount) const;

    bool operator ==(const FSurfaceFormat& other) const;
    bool operator !=(const FSurfaceFormat& other) const { return !operator ==(other); }

    void CheckSupport(FDeviceEncapsulator *device) const;
    void ClearSupport(FDeviceEncapsulator *device) const;

    static void Start();
    static void Shutdown();

    static void OnDeviceCreate(FDeviceEncapsulator *device);
    static void OnDeviceDestroy(FDeviceEncapsulator *device);

    static const FSurfaceFormat* FromType(ESurfaceFormatType type);

private:
    const char *_name;

    u32 _blockSize;
    u32 _macroBlockBitCount;

    ESurfaceFormatType _type;
    ESurfaceFormatFlags _flags;

    mutable ESurfaceFormatSupport _support;

public:
    static TMemoryView<const FSurfaceFormat> AllFormats();

    static const FSurfaceFormat *UNKNOWN;
    static const FSurfaceFormat *A8;
    static const FSurfaceFormat *D16;
    static const FSurfaceFormat *D24S8;
    static const FSurfaceFormat *D32;
    static const FSurfaceFormat *DXN0;
    static const FSurfaceFormat *DXT1;
    static const FSurfaceFormat *DXT1_SRGB;
    static const FSurfaceFormat *DXT3;
    static const FSurfaceFormat *DXT3_SRGB;
    static const FSurfaceFormat *DXT5;
    static const FSurfaceFormat *DXT5_SRGB;
    static const FSurfaceFormat *R5G5B5A1;
    static const FSurfaceFormat *R5G6B5;
    static const FSurfaceFormat *R8;
    static const FSurfaceFormat *R8G8;
    static const FSurfaceFormat *R8G8B8A8;
    static const FSurfaceFormat *R8G8B8A8_SRGB;
    static const FSurfaceFormat *B8G8R8A8;
    static const FSurfaceFormat *B8G8R8A8_SRGB;
    static const FSurfaceFormat *R10G10B10A2;
    static const FSurfaceFormat *R11G11B10;
    static const FSurfaceFormat *R16;
    static const FSurfaceFormat *R16G16;
    static const FSurfaceFormat *R16G16B16A16;
    static const FSurfaceFormat *R16G16B16A16_F;
    static const FSurfaceFormat *R16G16_F;
    static const FSurfaceFormat *R16_F;
    static const FSurfaceFormat *R32;
    static const FSurfaceFormat *R32G32;
    static const FSurfaceFormat *R32G32B32A32;
    static const FSurfaceFormat *R32G32B32A32_F;
    static const FSurfaceFormat *R32G32_F;
    static const FSurfaceFormat *R32_F;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
