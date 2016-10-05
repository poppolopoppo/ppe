#include "stdafx.h"

#include "SurfaceFormat.h"

#include "Device/DeviceEncapsulator.h"

#include "Core/Maths/ScalarVector.h"
#include "Core/Memory/MemoryView.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FSurfaceFormat::FSurfaceFormat(
    const char *name,
    u32 blockSize,
    u32 macroBlockBitCount,
    ESurfaceFormatType type,
    ESurfaceFormatFlags flags,
    ESurfaceFormatSupport support)
:   _name(name),
    _blockSize(blockSize), _macroBlockBitCount(macroBlockBitCount),
    _type(type), _flags(flags), _support(support) {
    Assert(name);
    Assert(blockSize);
    Assert(1 == blockSize || 4*4 == blockSize);
}
//----------------------------------------------------------------------------
FSurfaceFormat::~FSurfaceFormat() {}
//----------------------------------------------------------------------------
u32 FSurfaceFormat::Pitch() const {
    return _macroBlockBitCount >> 3;
}
//----------------------------------------------------------------------------
u32 FSurfaceFormat::BitsPerPixels() const {
    return _macroBlockBitCount / _blockSize;
}
//----------------------------------------------------------------------------
u32 FSurfaceFormat::MacroBlockSizeInPixels() const {
    if (1 == _blockSize)
        return 1;
    else {
        Assert(16 == _blockSize);
        return 4;
    }
}
//----------------------------------------------------------------------------
void FSurfaceFormat::SizeOfTexture2D(size_t *rowBytes, size_t *numRows, size_t width, size_t height) const {
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
size_t FSurfaceFormat::SizeOfTexture2DInBytes(size_t width, size_t height) const {
    size_t rowBytes;
    size_t numRows;
    SizeOfTexture2D(&rowBytes, &numRows, width, height);

    return rowBytes * numRows;
}
//----------------------------------------------------------------------------
size_t FSurfaceFormat::SizeOfTexture2DInBytes(size_t width, size_t height, size_t levelCount) const {
    Assert(levelCount);

    size_t totalSizeInBytes = 0;

    for (size_t i = 0; i < levelCount; ++i) {
        totalSizeInBytes += SizeOfTexture2DInBytes(width, height);

        width = std::max<size_t>(1, width >> 1);
        height = std::max<size_t>(1, height >> 1);
    }

    return totalSizeInBytes;
}
//----------------------------------------------------------------------------
size_t FSurfaceFormat::SizeOfTexture2DInBytes(const uint2& widthHeight) const {
    return SizeOfTexture2DInBytes(widthHeight.x(), widthHeight.y());
}
//----------------------------------------------------------------------------
size_t FSurfaceFormat::SizeOfTexture2DInBytes(const uint2& widthHeight, size_t levelCount) const {
    return SizeOfTexture2DInBytes(widthHeight.x(), widthHeight.y(), levelCount);
}
//----------------------------------------------------------------------------
bool FSurfaceFormat::operator ==(const FSurfaceFormat& other) const {
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
void FSurfaceFormat::CheckSupport(FDeviceEncapsulator * /* device */) const {
    // TODO
    _support = ESurfaceFormatSupport::All;
}
//----------------------------------------------------------------------------
void FSurfaceFormat::ClearSupport(FDeviceEncapsulator * /* device */) const {
    _support = ESurfaceFormatSupport::All;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FSurfaceFormat::Start() {
    for (const FSurfaceFormat& fmt : FSurfaceFormat::AllFormats())
        fmt.ClearSupport(nullptr);
}
//----------------------------------------------------------------------------
void FSurfaceFormat::Shutdown() {
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FSurfaceFormat::OnDeviceCreate(FDeviceEncapsulator *device) {
    for (const FSurfaceFormat& fmt : FSurfaceFormat::AllFormats())
        fmt.CheckSupport(device);
}
//----------------------------------------------------------------------------
void FSurfaceFormat::OnDeviceDestroy(FDeviceEncapsulator *device) {
    for (const FSurfaceFormat& fmt : FSurfaceFormat::AllFormats())
        fmt.ClearSupport(device);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const FSurfaceFormat* FSurfaceFormat::FromType(ESurfaceFormatType type) {
    switch (type)
    {
    case Core::Graphics::ESurfaceFormatType::UNKNOWN:
        return FSurfaceFormat::UNKNOWN;
    case Core::Graphics::ESurfaceFormatType::A8:
        return FSurfaceFormat::A8;
    case Core::Graphics::ESurfaceFormatType::D16:
        return FSurfaceFormat::D16;
    case Core::Graphics::ESurfaceFormatType::D24S8:
        return FSurfaceFormat::D24S8;
    case Core::Graphics::ESurfaceFormatType::D32:
        return FSurfaceFormat::D32;
    case Core::Graphics::ESurfaceFormatType::DXN0:
        return FSurfaceFormat::DXN0;
    case Core::Graphics::ESurfaceFormatType::DXT1:
        return FSurfaceFormat::DXT1;
    case Core::Graphics::ESurfaceFormatType::DXT1_SRGB:
        return FSurfaceFormat::DXT1_SRGB;
    case Core::Graphics::ESurfaceFormatType::DXT3:
        return FSurfaceFormat::DXT3;
    case Core::Graphics::ESurfaceFormatType::DXT3_SRGB:
        return FSurfaceFormat::DXT3_SRGB;
    case Core::Graphics::ESurfaceFormatType::DXT5:
        return FSurfaceFormat::DXT5;
    case Core::Graphics::ESurfaceFormatType::DXT5_SRGB:
        return FSurfaceFormat::DXT5_SRGB;
    case Core::Graphics::ESurfaceFormatType::R5G5B5A1:
        return FSurfaceFormat::R5G5B5A1;
    case Core::Graphics::ESurfaceFormatType::R5G6B5:
        return FSurfaceFormat::R5G6B5;
    case Core::Graphics::ESurfaceFormatType::R8:
        return FSurfaceFormat::R8;
    case Core::Graphics::ESurfaceFormatType::R8G8:
        return FSurfaceFormat::R8G8;
    case Core::Graphics::ESurfaceFormatType::R8G8B8A8:
        return FSurfaceFormat::R8G8B8A8;
    case Core::Graphics::ESurfaceFormatType::R8G8B8A8_SRGB:
        return FSurfaceFormat::R8G8B8A8_SRGB;
    case Core::Graphics::ESurfaceFormatType::B8G8R8A8:
        return FSurfaceFormat::B8G8R8A8;
    case Core::Graphics::ESurfaceFormatType::B8G8R8A8_SRGB:
        return FSurfaceFormat::B8G8R8A8_SRGB;
    case Core::Graphics::ESurfaceFormatType::R10G10B10A2:
        return FSurfaceFormat::R10G10B10A2;
    case Core::Graphics::ESurfaceFormatType::R11G11B10:
        return FSurfaceFormat::R11G11B10;
    case Core::Graphics::ESurfaceFormatType::R16:
        return FSurfaceFormat::R16;
    case Core::Graphics::ESurfaceFormatType::R16G16:
        return FSurfaceFormat::R16G16;
    case Core::Graphics::ESurfaceFormatType::R16G16B16A16:
        return FSurfaceFormat::R16G16B16A16;
    case Core::Graphics::ESurfaceFormatType::R16G16B16A16_F:
        return FSurfaceFormat::R16G16B16A16_F;
    case Core::Graphics::ESurfaceFormatType::R16G16_F:
        return FSurfaceFormat::R16G16_F;
    case Core::Graphics::ESurfaceFormatType::R16_F:
        return FSurfaceFormat::R16_F;
    case Core::Graphics::ESurfaceFormatType::R32:
        return FSurfaceFormat::R32;
    case Core::Graphics::ESurfaceFormatType::R32G32:
        return FSurfaceFormat::R32G32;
    case Core::Graphics::ESurfaceFormatType::R32G32B32A32:
        return FSurfaceFormat::R32G32B32A32;
    case Core::Graphics::ESurfaceFormatType::R32G32B32A32_F:
        return FSurfaceFormat::R32G32B32A32_F;
    case Core::Graphics::ESurfaceFormatType::R32G32_F:
        return FSurfaceFormat::R32G32_F;
    case Core::Graphics::ESurfaceFormatType::R32_F:
        return FSurfaceFormat::R32_F;
    default:
        AssertNotImplemented();
    }
    return nullptr;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FStringView SurfaceFormatTypeToCStr(ESurfaceFormatType value) {
    switch (value)
    {
    case Core::Graphics::ESurfaceFormatType::UNKNOWN:
        return MakeStringView("UNKNOWN");
    case Core::Graphics::ESurfaceFormatType::A8:
        return MakeStringView("A8");
    case Core::Graphics::ESurfaceFormatType::D16:
        return MakeStringView("D16");
    case Core::Graphics::ESurfaceFormatType::D24S8:
        return MakeStringView("D24S8");
    case Core::Graphics::ESurfaceFormatType::D32:
        return MakeStringView("D32");
    case Core::Graphics::ESurfaceFormatType::DXN0:
        return MakeStringView("DXN0");
    case Core::Graphics::ESurfaceFormatType::DXT1:
        return MakeStringView("DXT1");
    case Core::Graphics::ESurfaceFormatType::DXT1_SRGB:
        return MakeStringView("DXT1_SRGB");
    case Core::Graphics::ESurfaceFormatType::DXT3:
        return MakeStringView("DXT3");
    case Core::Graphics::ESurfaceFormatType::DXT3_SRGB:
        return MakeStringView("DXT3_SRGB");
    case Core::Graphics::ESurfaceFormatType::DXT5:
        return MakeStringView("DXT5");
    case Core::Graphics::ESurfaceFormatType::DXT5_SRGB:
        return MakeStringView("DXT5_SRGB");
    case Core::Graphics::ESurfaceFormatType::R5G5B5A1:
        return MakeStringView("R5G5B5A1");
    case Core::Graphics::ESurfaceFormatType::R5G6B5:
        return MakeStringView("R5G6B5");
    case Core::Graphics::ESurfaceFormatType::R8:
        return MakeStringView("R8");
    case Core::Graphics::ESurfaceFormatType::R8G8:
        return MakeStringView("R8G8");
    case Core::Graphics::ESurfaceFormatType::R8G8B8A8:
        return MakeStringView("R8G8B8A8");
    case Core::Graphics::ESurfaceFormatType::R8G8B8A8_SRGB:
        return MakeStringView("R8G8B8A8_SRGB");
    case Core::Graphics::ESurfaceFormatType::B8G8R8A8:
        return MakeStringView("B8G8R8A8");
    case Core::Graphics::ESurfaceFormatType::B8G8R8A8_SRGB:
        return MakeStringView("B8G8R8A8_SRGB");
    case Core::Graphics::ESurfaceFormatType::R10G10B10A2:
        return MakeStringView("R10G10B10A2");
    case Core::Graphics::ESurfaceFormatType::R11G11B10:
        return MakeStringView("R11G11B10");
    case Core::Graphics::ESurfaceFormatType::R16:
        return MakeStringView("R16");
    case Core::Graphics::ESurfaceFormatType::R16G16:
        return MakeStringView("R16G16");
    case Core::Graphics::ESurfaceFormatType::R16G16B16A16:
        return MakeStringView("R16G16B16A16");
    case Core::Graphics::ESurfaceFormatType::R16G16B16A16_F:
        return MakeStringView("R16G16B16A16_F");
    case Core::Graphics::ESurfaceFormatType::R16G16_F:
        return MakeStringView("R16G16_F");
    case Core::Graphics::ESurfaceFormatType::R16_F:
        return MakeStringView("R16_F");
    case Core::Graphics::ESurfaceFormatType::R32:
        return MakeStringView("R32");
    case Core::Graphics::ESurfaceFormatType::R32G32:
        return MakeStringView("R32G32");
    case Core::Graphics::ESurfaceFormatType::R32G32B32A32:
        return MakeStringView("R32G32B32A32");
    case Core::Graphics::ESurfaceFormatType::R32G32B32A32_F:
        return MakeStringView("R32G32B32A32_F");
    case Core::Graphics::ESurfaceFormatType::R32G32_F:
        return MakeStringView("R32G32_F");
    case Core::Graphics::ESurfaceFormatType::R32_F:
        return MakeStringView("R32_F");
    default:
        AssertNotImplemented();
    }
    return FStringView();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
#define DEF_SURFACEFORMAT_INSTANCE(_Type, _BlockSize, _MacroBlockBitCount, _Flags) \
    FSurfaceFormat( \
            STRINGIZE(_Type), \
            _BlockSize, _MacroBlockBitCount, \
            ESurfaceFormatType::_Type, \
            static_cast<ESurfaceFormatFlags>(_Flags), \
            static_cast<ESurfaceFormatSupport>(0) \
            )
//----------------------------------------------------------------------------
#define SFMT_RGB u32(ESurfaceFormatFlags::RGB)
#define SFMT_RA u32(ESurfaceFormatFlags::RA)
#define SFMT_Alpha u32(ESurfaceFormatFlags::Alpha)
#define SFMT_Luminance u32(ESurfaceFormatFlags::Luminance)
#define SFMT_Depth u32(ESurfaceFormatFlags::Depth)
#define SFMT_Stencil u32(ESurfaceFormatFlags::Stencil)
#define SFMT_Bump u32(ESurfaceFormatFlags::Bump)
#define SFMT_Palette u32(ESurfaceFormatFlags::Palette)
#define SFMT_FloatingPoint u32(ESurfaceFormatFlags::FloatingPoint)
#define SFMT_DXTC u32(ESurfaceFormatFlags::DXTC)
#define SFMT_GammaSpace u32(ESurfaceFormatFlags::GammaSpace)
//----------------------------------------------------------------------------
static const FSurfaceFormat gAllSurfaceFormats[(u32)ESurfaceFormatType::__COUNT] = {
DEF_SURFACEFORMAT_INSTANCE(UNKNOWN,             1,  0,  0),
DEF_SURFACEFORMAT_INSTANCE(A8,                  1,  8,  SFMT_Alpha),
DEF_SURFACEFORMAT_INSTANCE(D16,                 1, 16,  SFMT_Depth),
DEF_SURFACEFORMAT_INSTANCE(D24S8,               1, 32,  SFMT_Depth|SFMT_Stencil),
DEF_SURFACEFORMAT_INSTANCE(D32,                 1, 32,  SFMT_Depth),
DEF_SURFACEFORMAT_INSTANCE(DXN0,                16,128, SFMT_Bump|SFMT_DXTC),
DEF_SURFACEFORMAT_INSTANCE(DXT1,                16, 64, SFMT_RGB|SFMT_DXTC),
DEF_SURFACEFORMAT_INSTANCE(DXT1_SRGB,           16, 64, SFMT_RGB|SFMT_GammaSpace|SFMT_DXTC),
DEF_SURFACEFORMAT_INSTANCE(DXT3,                16,128, SFMT_RGB|SFMT_Alpha|SFMT_DXTC),
DEF_SURFACEFORMAT_INSTANCE(DXT3_SRGB,           16,128, SFMT_RGB|SFMT_Alpha|SFMT_GammaSpace|SFMT_DXTC),
DEF_SURFACEFORMAT_INSTANCE(DXT5,                16,128, SFMT_RA|SFMT_DXTC),
DEF_SURFACEFORMAT_INSTANCE(DXT5_SRGB,           16,128, SFMT_RA|SFMT_GammaSpace|SFMT_DXTC),
DEF_SURFACEFORMAT_INSTANCE(R5G5B5A1,            1, 16,  SFMT_RGB|SFMT_Alpha),
DEF_SURFACEFORMAT_INSTANCE(R5G6B5,              1, 16,  SFMT_RGB),
DEF_SURFACEFORMAT_INSTANCE(R8,                  1,  8,  SFMT_Luminance),
DEF_SURFACEFORMAT_INSTANCE(R8G8,                1, 16,  SFMT_RA),
DEF_SURFACEFORMAT_INSTANCE(R8G8B8A8,            1, 32,  SFMT_RGB|SFMT_Alpha),
DEF_SURFACEFORMAT_INSTANCE(R8G8B8A8_SRGB,       1, 32,  SFMT_RGB|SFMT_Alpha|SFMT_GammaSpace),
DEF_SURFACEFORMAT_INSTANCE(B8G8R8A8,            1, 32,  SFMT_RGB|SFMT_Alpha),
DEF_SURFACEFORMAT_INSTANCE(B8G8R8A8_SRGB,       1, 32,  SFMT_RGB|SFMT_Alpha|SFMT_GammaSpace),
DEF_SURFACEFORMAT_INSTANCE(R10G10B10A2,         1, 32,  SFMT_RGB|SFMT_Alpha),
DEF_SURFACEFORMAT_INSTANCE(R11G11B10,           1, 32,  SFMT_RGB|SFMT_FloatingPoint),
DEF_SURFACEFORMAT_INSTANCE(R16,                 1, 16,  SFMT_Luminance),
DEF_SURFACEFORMAT_INSTANCE(R16G16,              1, 32,  SFMT_RA),
DEF_SURFACEFORMAT_INSTANCE(R16G16B16A16,        1, 64,  SFMT_RGB|SFMT_Alpha),
DEF_SURFACEFORMAT_INSTANCE(R16G16B16A16_F,      1, 64,  SFMT_RGB|SFMT_Alpha|SFMT_FloatingPoint),
DEF_SURFACEFORMAT_INSTANCE(R16G16_F,            1, 32,  SFMT_FloatingPoint),
DEF_SURFACEFORMAT_INSTANCE(R16_F,               1, 16,  SFMT_Luminance|SFMT_FloatingPoint),
DEF_SURFACEFORMAT_INSTANCE(R32,                 1, 32,  SFMT_Luminance),
DEF_SURFACEFORMAT_INSTANCE(R32G32,              1, 64,  SFMT_RA),
DEF_SURFACEFORMAT_INSTANCE(R32G32B32A32,        1,128,  SFMT_RGB|SFMT_Alpha),
DEF_SURFACEFORMAT_INSTANCE(R32G32B32A32_F,      1,128,  SFMT_RGB|SFMT_Alpha|SFMT_FloatingPoint),
DEF_SURFACEFORMAT_INSTANCE(R32G32_F,            1, 64,  SFMT_RA|SFMT_FloatingPoint),
DEF_SURFACEFORMAT_INSTANCE(R32_F,               1, 32,  SFMT_Luminance|SFMT_FloatingPoint)
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
TMemoryView<const FSurfaceFormat> FSurfaceFormat::AllFormats() {
    return MakeView(gAllSurfaceFormats);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define DEF_SURFACEFORMAT_MEMBER(_Type) \
    STATIC_ASSERT(static_cast<size_t>(ESurfaceFormatType::_Type) < lengthof(gAllSurfaceFormats)); \
    const FSurfaceFormat *FSurfaceFormat::_Type = &gAllSurfaceFormats[static_cast<size_t>(ESurfaceFormatType::_Type)];
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
DEF_SURFACEFORMAT_MEMBER(B8G8R8A8)
DEF_SURFACEFORMAT_MEMBER(B8G8R8A8_SRGB)
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
