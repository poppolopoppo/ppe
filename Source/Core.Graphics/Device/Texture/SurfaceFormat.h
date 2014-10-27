#pragma once

#include "Core.Graphics/Graphics.h"

namespace Core {
    template <typename T>
    class MemoryView;
}

namespace Core {
namespace Graphics {
class DeviceEncapsulator;

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class SurfaceFormatType : u32 {
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
    R10G10B10A2     = 18,
    R11G11B10       = 19,
    R16             = 20,
    R16G16          = 21,
    R16G16B16A16    = 22,
    R16G16B16A16_F  = 23,
    R16G16_F        = 24,
    R16_F           = 25,
    R32             = 26,
    R32G32          = 27,
    R32G32B32A32    = 28,
    R32G32B32A32_F  = 29,
    R32G32_F        = 30,
    R32_F           = 31,

    __COUNT
};
//----------------------------------------------------------------------------
const char *SurfaceFormatTypeToCStr(SurfaceFormatType value);
//----------------------------------------------------------------------------
enum class SurfaceFormatFlags : u32 {
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
enum class SurfaceFormatSupport : u32 {
    None            = 0,
    Checked         = 1 << 0,
    Texture         = 1 << 1,
    RenderTarget    = 1 << 2,
    AutoGenMipMaps  = 1 << 3,
    VertexTexture   = 1 << 4,
    DepthStencil    = 1 << 5,

    All = Checked|Texture|RenderTarget|AutoGenMipMaps|VertexTexture|DepthStencil
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class SurfaceFormat {
public:
    SurfaceFormat(
        const char *name,
        u32 blockSize,
        u32 macroBlockBitCount,
        SurfaceFormatType type,
        SurfaceFormatFlags flags,
        SurfaceFormatSupport support);
    ~SurfaceFormat();

    const char *Name() const { return _name; }
    SurfaceFormatType Type() const { return _type; }
    SurfaceFormatFlags Flags() const { return _flags; }
    SurfaceFormatSupport Support() const { return _support; }

    bool IsRGB() const { return 0 != (u32(SurfaceFormatFlags::RGB) & u32(_flags)); }
    bool IsRA() const { return 0 != (u32(SurfaceFormatFlags::RA) & u32(_flags)); }
    bool IsAlpha() const { return 0 != (u32(SurfaceFormatFlags::Alpha) & u32(_flags)); }
    bool IsLuminance() const { return 0 != (u32(SurfaceFormatFlags::Luminance) & u32(_flags)); }
    bool IsDepth() const { return 0 != (u32(SurfaceFormatFlags::Depth) & u32(_flags)); }
    bool IsStencil() const { return 0 != (u32(SurfaceFormatFlags::Stencil) & u32(_flags)); }
    bool IsBump() const { return 0 != (u32(SurfaceFormatFlags::Bump) & u32(_flags)); }
    bool IsPalette() const { return 0 != (u32(SurfaceFormatFlags::Palette) & u32(_flags)); }
    bool IsFloatingPoint() const { return 0 != (u32(SurfaceFormatFlags::FloatingPoint) & u32(_flags)); }
    bool IsDXTC() const { return 0 != (u32(SurfaceFormatFlags::DXTC) & u32(_flags)); }
    bool IsGammaSpace() const { return 0 != (u32(SurfaceFormatFlags::GammaSpace) & u32(_flags)); }

    bool SupportChecked() const { return 0 != (u32(SurfaceFormatSupport::Checked) & u32(_support)); }
    bool SupportTexture() const { Assert(SupportChecked()); return 0 != (u32(SurfaceFormatSupport::Texture) & u32(_support)); }
    bool SupportRenderTarget() const { Assert(SupportChecked()); return 0 != (u32(SurfaceFormatSupport::RenderTarget) & u32(_support)); }
    bool SupportAutoGenMipMaps() const { Assert(SupportChecked()); return 0 != (u32(SurfaceFormatSupport::AutoGenMipMaps) & u32(_support)); }
    bool SupportVertexTexture() const { Assert(SupportChecked()); return 0 != (u32(SurfaceFormatSupport::VertexTexture) & u32(_support)); }
    bool SupportDepthStencil() const { Assert(SupportChecked()); return 0 != (u32(SurfaceFormatSupport::DepthStencil) & u32(_support)); }

    u32 BlockSize() const { return _blockSize; }
    u32 MacroBlockBitCount() const { return _macroBlockBitCount; }

    u32 Pitch() const;
    u32 BitsPerPixels() const;
    u32 MacroBlockSizeInPixels() const;

    void SizeOfTexture2D(size_t *rowBytes, size_t *numRows, size_t width, size_t height) const;

    size_t SizeOfTexture2DInBytes(size_t width, size_t height) const;
    size_t SizeOfTexture2DInBytes(size_t width, size_t height, size_t levelCount) const;

    bool operator ==(const SurfaceFormat& other) const;
    bool operator !=(const SurfaceFormat& other) const { return !operator ==(other); }

    void CheckSupport(DeviceEncapsulator *device) const;
    void ClearSupport() const;

    static void Start();
    static void Shutdown();

    static void OnDeviceCreate(DeviceEncapsulator *device);
    static void OnDeviceDestroy(DeviceEncapsulator *device);

private:
    const char *_name;

    u32 _blockSize;
    u32 _macroBlockBitCount;

    SurfaceFormatType _type;
    SurfaceFormatFlags _flags;

    mutable SurfaceFormatSupport _support;

public:
    static MemoryView<const SurfaceFormat> AllFormats();

    static const SurfaceFormat *UNKNOWN;
    static const SurfaceFormat *A8;
    static const SurfaceFormat *D16;
    static const SurfaceFormat *D24S8;
    static const SurfaceFormat *D32;
    static const SurfaceFormat *DXN0;
    static const SurfaceFormat *DXT1;
    static const SurfaceFormat *DXT1_SRGB;
    static const SurfaceFormat *DXT3;
    static const SurfaceFormat *DXT3_SRGB;
    static const SurfaceFormat *DXT5;
    static const SurfaceFormat *DXT5_SRGB;
    static const SurfaceFormat *R5G5B5A1;
    static const SurfaceFormat *R5G6B5;
    static const SurfaceFormat *R8;
    static const SurfaceFormat *R8G8;
    static const SurfaceFormat *R8G8B8A8;
    static const SurfaceFormat *R8G8B8A8_SRGB;
    static const SurfaceFormat *R10G10B10A2;
    static const SurfaceFormat *R11G11B10;
    static const SurfaceFormat *R16;
    static const SurfaceFormat *R16G16;
    static const SurfaceFormat *R16G16B16A16;
    static const SurfaceFormat *R16G16B16A16_F;
    static const SurfaceFormat *R16G16_F;
    static const SurfaceFormat *R16_F;
    static const SurfaceFormat *R32;
    static const SurfaceFormat *R32G32;
    static const SurfaceFormat *R32G32B32A32;
    static const SurfaceFormat *R32G32B32A32_F;
    static const SurfaceFormat *R32G32_F;
    static const SurfaceFormat *R32_F;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
