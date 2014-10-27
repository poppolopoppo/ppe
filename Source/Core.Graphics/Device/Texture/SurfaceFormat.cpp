#include "stdafx.h"

#include "SurfaceFormat.h"

#include "Device/DeviceEncapsulator.h"

#include "Core/Memory/MemoryView.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SurfaceFormat::SurfaceFormat(
    const char *name,
    u32 blockSize,
    u32 macroBlockBitCount,
    SurfaceFormatType type,
    SurfaceFormatFlags flags,
    SurfaceFormatSupport support)
:   _name(name),
    _blockSize(blockSize), _macroBlockBitCount(macroBlockBitCount),
    _type(type), _flags(flags), _support(support) {
    Assert(name);
    Assert(blockSize);
    Assert(macroBlockBitCount);
    Assert(1 == blockSize || 4*4 == blockSize);
}
//----------------------------------------------------------------------------
SurfaceFormat::~SurfaceFormat() {}
//----------------------------------------------------------------------------
u32 SurfaceFormat::Pitch() const {
    return _macroBlockBitCount >> 3;
}
//----------------------------------------------------------------------------
u32 SurfaceFormat::BitsPerPixels() const {
    return _macroBlockBitCount / _blockSize;
}
//----------------------------------------------------------------------------
u32 SurfaceFormat::MacroBlockSizeInPixels() const {
    if (1 == _blockSize)
        return 1;
    else {
        Assert(16 == _blockSize);
        return 4;
    }
}
//----------------------------------------------------------------------------
void SurfaceFormat::SizeOfTexture2D(size_t *rowBytes, size_t *numRows, size_t width, size_t height) const {
    Assert(rowBytes);
    Assert(numRows);

    const size_t macroBlockSizeInPixels = MacroBlockSizeInPixels();
    Assert(macroBlockSizeInPixels);

    size_t blocksWide = 0;
    if (width > 0)
        blocksWide = std::max<size_t>(1, (width + macroBlockSizeInPixels - 1)/macroBlockSizeInPixels);

    size_t blocksHigh = 0;
    if (height > 0)
        blocksHigh = std::max<size_t>(1, (height + macroBlockSizeInPixels - 1)/macroBlockSizeInPixels);

    *rowBytes = (blocksWide * _macroBlockBitCount) >> 3;
    *numRows = blocksHigh;
}
//----------------------------------------------------------------------------
size_t SurfaceFormat::SizeOfTexture2DInBytes(size_t width, size_t height) const {
    size_t rowBytes;
    size_t numRows;
    SizeOfTexture2D(&rowBytes, &numRows, width, height);

    return rowBytes * numRows;
}
//----------------------------------------------------------------------------
size_t SurfaceFormat::SizeOfTexture2DInBytes(size_t width, size_t height, size_t levelCount) const {
    Assert(levelCount);

    size_t totalSizeInBytes = 0;

    for (size_t i = 0; i < levelCount; ++i) {
        totalSizeInBytes += SizeOfTexture2DInBytes(width, height);

        width = width >> 1;
        height = height >> 1;
    }

    return totalSizeInBytes;
}
//----------------------------------------------------------------------------
bool SurfaceFormat::operator ==(const SurfaceFormat& other) const {
#ifdef _DEBUG
    if (this != &other) {
        Assert(_type != other._type);
        return false;
    }
    else {
        Assert(_name == other._name);
        Assert(_type == other._type);
        Assert(_flags == other._flags);
        Assert(_support == other._support);
        Assert(_blockSize == other._blockSize);
        Assert(_macroBlockBitCount == other._macroBlockBitCount);
        return true;
    }
#else
    return this == &other;
#endif
}
//----------------------------------------------------------------------------
void SurfaceFormat::CheckSupport(DeviceEncapsulator *device) const {
    // TODO
    _support = SurfaceFormatSupport::All;
}
//----------------------------------------------------------------------------
void SurfaceFormat::ClearSupport() const {
    _support = SurfaceFormatSupport::All;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void SurfaceFormat::Start() {
    for (const SurfaceFormat& fmt : SurfaceFormat::AllFormats())
        fmt.ClearSupport();
}
//----------------------------------------------------------------------------
void SurfaceFormat::Shutdown() {
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void SurfaceFormat::OnDeviceCreate(DeviceEncapsulator *device) {
    for (const SurfaceFormat& fmt : SurfaceFormat::AllFormats())
        fmt.CheckSupport(device);
}
//----------------------------------------------------------------------------
void SurfaceFormat::OnDeviceDestroy(DeviceEncapsulator *device) {
    for (const SurfaceFormat& fmt : SurfaceFormat::AllFormats())
        fmt.ClearSupport();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const char *SurfaceFormatTypeToCStr(SurfaceFormatType value) {
    switch (value)
    {
    case Core::Graphics::SurfaceFormatType::UNKNOWN:
        return "UNKNOWN";
    case Core::Graphics::SurfaceFormatType::A8:
        return "A8";
    case Core::Graphics::SurfaceFormatType::D16:
        return "D16";
    case Core::Graphics::SurfaceFormatType::D24S8:
        return "D24S8";
    case Core::Graphics::SurfaceFormatType::D32:
        return "D32";
    case Core::Graphics::SurfaceFormatType::DXN0:
        return "DXN0";
    case Core::Graphics::SurfaceFormatType::DXT1:
        return "DXT1";
    case Core::Graphics::SurfaceFormatType::DXT1_SRGB:
        return "DXT1_SRGB";
    case Core::Graphics::SurfaceFormatType::DXT3:
        return "DXT3";
    case Core::Graphics::SurfaceFormatType::DXT3_SRGB:
        return "DXT3_SRGB";
    case Core::Graphics::SurfaceFormatType::DXT5:
        return "DXT5";
    case Core::Graphics::SurfaceFormatType::DXT5_SRGB:
        return "DXT5_SRGB";
    case Core::Graphics::SurfaceFormatType::R5G5B5A1:
        return "R5G5B5A1";
    case Core::Graphics::SurfaceFormatType::R5G6B5:
        return "R5G6B5";
    case Core::Graphics::SurfaceFormatType::R8:
        return "R8";
    case Core::Graphics::SurfaceFormatType::R8G8:
        return "R8G8";
    case Core::Graphics::SurfaceFormatType::R8G8B8A8:
        return "R8G8B8A8";
    case Core::Graphics::SurfaceFormatType::R8G8B8A8_SRGB:
        return "R8G8B8A8_SRGB";
    case Core::Graphics::SurfaceFormatType::R10G10B10A2:
        return "R10G10B10A2";
    case Core::Graphics::SurfaceFormatType::R11G11B10:
        return "R11G11B10";
    case Core::Graphics::SurfaceFormatType::R16:
        return "R16";
    case Core::Graphics::SurfaceFormatType::R16G16:
        return "R16G16";
    case Core::Graphics::SurfaceFormatType::R16G16B16A16:
        return "R16G16B16A16";
    case Core::Graphics::SurfaceFormatType::R16G16B16A16_F:
        return "R16G16B16A16_F";
    case Core::Graphics::SurfaceFormatType::R16G16_F:
        return "R16G16_F";
    case Core::Graphics::SurfaceFormatType::R16_F:
        return "R16_F";
    case Core::Graphics::SurfaceFormatType::R32:
        return "R32";
    case Core::Graphics::SurfaceFormatType::R32G32:
        return "R32G32";
    case Core::Graphics::SurfaceFormatType::R32G32B32A32:
        return "R32G32B32A32";
    case Core::Graphics::SurfaceFormatType::R32G32B32A32_F:
        return "R32G32B32A32_F";
    case Core::Graphics::SurfaceFormatType::R32G32_F:
        return "R32G32_F";
    case Core::Graphics::SurfaceFormatType::R32_F:
        return "R32_F";
    default:
        AssertNotImplemented();
    }
    return nullptr;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
#define DEF_SURFACEFORMAT_INSTANCE(_Type, _BlockSize, _MacroBlockBitCount, _Flags) \
    SurfaceFormat( \
            STRINGIZE(_Type), \
            _BlockSize, _MacroBlockBitCount, \
            SurfaceFormatType::_Type, \
            static_cast<SurfaceFormatFlags>(_Flags), \
            static_cast<SurfaceFormatSupport>(0) \
            )
//----------------------------------------------------------------------------
#define SFMT_RGB u32(SurfaceFormatFlags::RGB)
#define SFMT_RA u32(SurfaceFormatFlags::RA)
#define SFMT_Alpha u32(SurfaceFormatFlags::Alpha)
#define SFMT_Luminance u32(SurfaceFormatFlags::Luminance)
#define SFMT_Depth u32(SurfaceFormatFlags::Depth)
#define SFMT_Stencil u32(SurfaceFormatFlags::Stencil)
#define SFMT_Bump u32(SurfaceFormatFlags::Bump)
#define SFMT_Palette u32(SurfaceFormatFlags::Palette)
#define SFMT_FloatingPoint u32(SurfaceFormatFlags::FloatingPoint)
#define SFMT_DXTC u32(SurfaceFormatFlags::DXTC)
#define SFMT_GammaSpace u32(SurfaceFormatFlags::GammaSpace)
//----------------------------------------------------------------------------
static const SurfaceFormat gAllSurfaceFormats[(u32)SurfaceFormatType::__COUNT] = {
DEF_SURFACEFORMAT_INSTANCE(UNKNOWN,         1, -1, 0),
DEF_SURFACEFORMAT_INSTANCE(A8,                 1,  8, SFMT_Alpha),
DEF_SURFACEFORMAT_INSTANCE(D16,             1, 16, SFMT_Depth),
DEF_SURFACEFORMAT_INSTANCE(D24S8,             1, 32, SFMT_Depth|SFMT_Stencil),
DEF_SURFACEFORMAT_INSTANCE(D32,             1, 32, SFMT_Depth),
DEF_SURFACEFORMAT_INSTANCE(DXN0,            16,128, SFMT_Bump|SFMT_DXTC),
DEF_SURFACEFORMAT_INSTANCE(DXT1,            16, 64, SFMT_RGB|SFMT_DXTC),
DEF_SURFACEFORMAT_INSTANCE(DXT1_SRGB,        16, 64, SFMT_RGB|SFMT_GammaSpace|SFMT_DXTC),
DEF_SURFACEFORMAT_INSTANCE(DXT3,            16,128, SFMT_RGB|SFMT_Alpha|SFMT_DXTC),
DEF_SURFACEFORMAT_INSTANCE(DXT3_SRGB,        16,128, SFMT_RGB|SFMT_Alpha|SFMT_GammaSpace|SFMT_DXTC),
DEF_SURFACEFORMAT_INSTANCE(DXT5,            16,128, SFMT_RA|SFMT_DXTC),
DEF_SURFACEFORMAT_INSTANCE(DXT5_SRGB,        16,128, SFMT_RA|SFMT_GammaSpace|SFMT_DXTC),
DEF_SURFACEFORMAT_INSTANCE(R5G5B5A1,         1, 16, SFMT_RGB|SFMT_Alpha),
DEF_SURFACEFORMAT_INSTANCE(R5G6B5,             1, 16, SFMT_RGB),
DEF_SURFACEFORMAT_INSTANCE(R8,                 1,  8, SFMT_Luminance),
DEF_SURFACEFORMAT_INSTANCE(R8G8,             1, 16, SFMT_RA),
DEF_SURFACEFORMAT_INSTANCE(R8G8B8A8,         1, 32, SFMT_RGB|SFMT_Alpha),
DEF_SURFACEFORMAT_INSTANCE(R8G8B8A8_SRGB,     1, 32, SFMT_RGB|SFMT_Alpha|SFMT_GammaSpace),
DEF_SURFACEFORMAT_INSTANCE(R10G10B10A2,     1, 32, SFMT_RGB|SFMT_Alpha),
DEF_SURFACEFORMAT_INSTANCE(R11G11B10,         1, 32, SFMT_RGB|SFMT_FloatingPoint),
DEF_SURFACEFORMAT_INSTANCE(R16,             1, 16, SFMT_Luminance),
DEF_SURFACEFORMAT_INSTANCE(R16G16,             1, 32, SFMT_RA),
DEF_SURFACEFORMAT_INSTANCE(R16G16B16A16,     1, 64, SFMT_RGB|SFMT_Alpha),
DEF_SURFACEFORMAT_INSTANCE(R16G16B16A16_F,  1, 64, SFMT_RGB|SFMT_Alpha|SFMT_FloatingPoint),
DEF_SURFACEFORMAT_INSTANCE(R16G16_F,         1, 32, SFMT_FloatingPoint),
DEF_SURFACEFORMAT_INSTANCE(R16_F,             1, 16, SFMT_Luminance|SFMT_FloatingPoint),
DEF_SURFACEFORMAT_INSTANCE(R32,             1, 32, SFMT_Luminance),
DEF_SURFACEFORMAT_INSTANCE(R32G32,             1, 64, SFMT_RA),
DEF_SURFACEFORMAT_INSTANCE(R32G32B32A32,     1,128, SFMT_RGB|SFMT_Alpha),
DEF_SURFACEFORMAT_INSTANCE(R32G32B32A32_F,  1,128, SFMT_RGB|SFMT_Alpha|SFMT_FloatingPoint),
DEF_SURFACEFORMAT_INSTANCE(R32G32_F,         1, 64, SFMT_RA|SFMT_FloatingPoint),
DEF_SURFACEFORMAT_INSTANCE(R32_F,             1, 32, SFMT_Luminance|SFMT_FloatingPoint)
};
//----------------------------------------------------------------------------
#undef SFMT_RGB
#undef SFMT_RA
#undef SFMT_Alpha
#undef SFMT_Luminance
#undef SFMT_Depth
#undef SFMT_Stencil
#undef SFMT_Bump
#undef SFMT_Palette
#undef SFMT_FloatingPoint
#undef SFMT_DXTC
#undef SFMT_GammaSpace
//----------------------------------------------------------------------------
#undef DEF_SURFACEFORMAT_INSTANCE
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
MemoryView<const SurfaceFormat> SurfaceFormat::AllFormats() {
    return MakeView(gAllSurfaceFormats);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define DEF_SURFACEFORMAT_MEMBER(_Type) \
    STATIC_ASSERT(static_cast<size_t>(SurfaceFormatType::_Type) < lengthof(gAllSurfaceFormats)); \
    const SurfaceFormat *SurfaceFormat::_Type = &gAllSurfaceFormats[static_cast<size_t>(SurfaceFormatType::_Type)];
//----------------------------------------------------------------------------
DEF_SURFACEFORMAT_MEMBER(UNKNOWN)
DEF_SURFACEFORMAT_MEMBER(A8)
DEF_SURFACEFORMAT_MEMBER(D16)
DEF_SURFACEFORMAT_MEMBER(D24S8)
DEF_SURFACEFORMAT_MEMBER(D32)
DEF_SURFACEFORMAT_MEMBER(DXN0)
DEF_SURFACEFORMAT_MEMBER(DXT1)
DEF_SURFACEFORMAT_MEMBER(DXT1_SRGB)
DEF_SURFACEFORMAT_MEMBER(DXT3)
DEF_SURFACEFORMAT_MEMBER(DXT3_SRGB)
DEF_SURFACEFORMAT_MEMBER(DXT5)
DEF_SURFACEFORMAT_MEMBER(DXT5_SRGB)
DEF_SURFACEFORMAT_MEMBER(R5G5B5A1)
DEF_SURFACEFORMAT_MEMBER(R5G6B5)
DEF_SURFACEFORMAT_MEMBER(R8)
DEF_SURFACEFORMAT_MEMBER(R8G8)
DEF_SURFACEFORMAT_MEMBER(R8G8B8A8)
DEF_SURFACEFORMAT_MEMBER(R8G8B8A8_SRGB)
DEF_SURFACEFORMAT_MEMBER(R10G10B10A2)
DEF_SURFACEFORMAT_MEMBER(R11G11B10)
DEF_SURFACEFORMAT_MEMBER(R16)
DEF_SURFACEFORMAT_MEMBER(R16G16)
DEF_SURFACEFORMAT_MEMBER(R16G16B16A16)
DEF_SURFACEFORMAT_MEMBER(R16G16B16A16_F)
DEF_SURFACEFORMAT_MEMBER(R16G16_F)
DEF_SURFACEFORMAT_MEMBER(R16_F)
DEF_SURFACEFORMAT_MEMBER(R32)
DEF_SURFACEFORMAT_MEMBER(R32G32)
DEF_SURFACEFORMAT_MEMBER(R32G32B32A32)
DEF_SURFACEFORMAT_MEMBER(R32G32B32A32_F)
DEF_SURFACEFORMAT_MEMBER(R32G32_F)
DEF_SURFACEFORMAT_MEMBER(R32_F)
//----------------------------------------------------------------------------
#undef DEF_SURFACEFORMAT_MEMBER
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
