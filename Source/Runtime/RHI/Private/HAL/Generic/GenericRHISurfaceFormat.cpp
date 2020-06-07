#include "stdafx.h"

#include "HAL/Generic/GenericRHISurfaceFormat.h"

#include "HAL/Generic/GenericRHIInstance.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FGenericPixelInfo::ImageSize(
    size_t* pNumStrides,
    size_t* pStrideInBytes,
    size_t width, size_t height ) const NOEXCEPT {
    Assert(pNumStrides);
    Assert(pStrideInBytes);
    Assert(width);
    Assert(height);

    const u32 blockNumPixels = BlockNumPixels();
    Assert(blockNumPixels);

    const size_t blockNumX = (width + blockNumPixels - 1) / blockNumPixels;
    const size_t blockNumY = (height + blockNumPixels - 1) / blockNumPixels;

    *pNumStrides = blockNumY;
    *pNumStrides = ((blockNumX * BlockNumBits()) >> 3);
}
//----------------------------------------------------------------------------
size_t FGenericPixelInfo::ImageSizeInBytes(
    size_t width, size_t height,
    size_t numLevels,
    size_t numSlices/* = 1 */,
    size_t levelOffset/* = 0 */) const NOEXCEPT {
    Assert(levelOffset < numLevels);
    Assert(numLevels);
    Assert(numSlices);
    Assert_NoAssume(Meta::IsAligned(4, width));
    Assert_NoAssume(Meta::IsAligned(4, height));

    size_t sliceSizeInBytes = 0;
    forrange(lvl, 0, numLevels) {
        if (lvl >= levelOffset) {
            size_t numStrides, strideInBytes;
            ImageSize(&numStrides, &strideInBytes, width, height);
            Assert(numStrides);
            Assert(strideInBytes);
            Assert_NoAssume(sliceSizeInBytes == 0 || sliceSizeInBytes > numStrides * strideInBytes);
            sliceSizeInBytes += (numStrides * strideInBytes);
        }
        width >>= 1;
        height >>= 1;
    }

    Assert(sliceSizeInBytes);
    return (sliceSizeInBytes * numSlices);
}
//----------------------------------------------------------------------------
FGenericPixelInfo FGenericPixelInfo::FromFormat(EGenericPixelFormat format) NOEXCEPT {
#define DEF_PIXELINFO(_ENUM,_WINDOWLOG2,_NUMBITSLOG2,_CHANNELS,_FORMATS) \
     case EGenericPixelFormat::_ENUM: return FGenericPixelInfo{ \
        u32(EGenericPixelFormat::_ENUM), \
        _WINDOWLOG2,_NUMBITSLOG2, \
        FGenericPixelInfo::_CHANNELS, \
        FGenericPixelInfo::_FORMATS \
     }

    switch (format) {
    DEF_PIXELINFO(R4G4_UNORM,           0,  0,  RG  ,   UNORM   );
    DEF_PIXELINFO(R4G4B4A4_UNORM,       0,  1,  RGBA,   UNORM   );
    DEF_PIXELINFO(B4G4R4A4_UNORM,       0,  1,  BGRA,   UNORM   );
    DEF_PIXELINFO(R5G6B5_UNORM,         0,  1,  RGB ,   UNORM   );
    DEF_PIXELINFO(B5G6R5_UNORM,         0,  1,  BGR ,   UNORM   );
    DEF_PIXELINFO(R5G5B5A1_UNORM,       0,  1,  RGBA,   UNORM   );
    DEF_PIXELINFO(B5G5R5A1_UNORM,       0,  1,  RGBA,   UNORM   );
    DEF_PIXELINFO(A1R5G5B5_UNORM,       0,  1,  ARGB,   UNORM   );
    DEF_PIXELINFO(R8_UNORM,             0,  0,  R   ,   UNORM   );
    DEF_PIXELINFO(R8_SNORM,             0,  0,  R   ,   SNORM   );
    DEF_PIXELINFO(R8_UINT,              0,  0,  R   ,   UINT    );
    DEF_PIXELINFO(R8_SINT,              0,  0,  R   ,   SINT    );
    DEF_PIXELINFO(R8_SRGB,              0,  0,  R   ,   SRGB    );
    DEF_PIXELINFO(R8G8_UNORM,           0,  1,  RG  ,   UNORM   );
    DEF_PIXELINFO(R8G8_SNORM,           0,  1,  RG  ,   SNORM   );
    DEF_PIXELINFO(R8G8_USCALED,         0,  1,  RG  ,   USCALED );
    DEF_PIXELINFO(R8G8_SSCALED,         0,  1,  RG  ,   SSCALED );
    DEF_PIXELINFO(R8G8_UINT,            0,  1,  RG  ,   UINT    );
    DEF_PIXELINFO(R8G8_SINT,            0,  1,  RG  ,   SINT    );
    DEF_PIXELINFO(R8G8_SRGB,            0,  1,  RG  ,   SRGB    );
    DEF_PIXELINFO(R8G8B8A8_UNORM,       0,  2,  RGBA,   UNORM   );
    DEF_PIXELINFO(R8G8B8A8_SNORM,       0,  2,  RGBA,   SNORM   );
    DEF_PIXELINFO(R8G8B8A8_USCALED,     0,  2,  RGBA,   USCALED );
    DEF_PIXELINFO(R8G8B8A8_SSCALED,     0,  2,  RGBA,   SSCALED );
    DEF_PIXELINFO(R8G8B8A8_UINT,        0,  2,  RGBA,   UINT    );
    DEF_PIXELINFO(R8G8B8A8_SINT,        0,  2,  RGBA,   SINT    );
    DEF_PIXELINFO(R8G8B8A8_SRGB,        0,  2,  RGBA,   SRGB    );
    DEF_PIXELINFO(B8G8R8A8_UNORM,       0,  2,  BGRA,   UNORM   );
    DEF_PIXELINFO(B8G8R8A8_SNORM,       0,  2,  BGRA,   SNORM   );
    DEF_PIXELINFO(B8G8R8A8_USCALED,     0,  2,  BGRA,   USCALED );
    DEF_PIXELINFO(B8G8R8A8_SSCALED,     0,  2,  BGRA,   SSCALED );
    DEF_PIXELINFO(B8G8R8A8_UINT,        0,  2,  BGRA,   UINT    );
    DEF_PIXELINFO(B8G8R8A8_SINT,        0,  2,  BGRA,   SINT    );
    DEF_PIXELINFO(B8G8R8A8_SRGB,        0,  2,  BGRA,   SRGB    );
    DEF_PIXELINFO(A2R10G10B10_UNORM,    0,  2,  ARGB,   UNORM   );
    DEF_PIXELINFO(A2R10G10B10_SNORM,    0,  2,  ARGB,   SNORM   );
    DEF_PIXELINFO(A2R10G10B10_USCALED,  0,  2,  ARGB,   USCALED );
    DEF_PIXELINFO(A2R10G10B10_SSCALED,  0,  2,  ARGB,   SSCALED );
    DEF_PIXELINFO(A2R10G10B10_UINT,     0,  2,  ARGB,   UINT    );
    DEF_PIXELINFO(A2R10G10B10_SINT,     0,  2,  ARGB,   SINT    );
    DEF_PIXELINFO(A2B10G10R10_UNORM,    0,  2,  ABGR,   UNORM   );
    DEF_PIXELINFO(A2B10G10R10_SNORM,    0,  2,  ABGR,   SNORM   );
    DEF_PIXELINFO(A2B10G10R10_USCALED,  0,  2,  ABGR,   USCALED );
    DEF_PIXELINFO(A2B10G10R10_SSCALED,  0,  2,  ABGR,   SSCALED );
    DEF_PIXELINFO(A2B10G10R10_UINT,     0,  2,  ABGR,   UINT    );
    DEF_PIXELINFO(A2B10G10R10_SINT,     0,  2,  ABGR,   SINT    );
    DEF_PIXELINFO(R16_UNORM,            0,  1,  R   ,   UNORM   );
    DEF_PIXELINFO(R16_SNORM,            0,  1,  R   ,   SNORM   );
    DEF_PIXELINFO(R16_USCALED,          0,  1,  R   ,   USCALED );
    DEF_PIXELINFO(R16_SSCALED,          0,  1,  R   ,   SSCALED );
    DEF_PIXELINFO(R16_UINT,             0,  1,  R   ,   UINT    );
    DEF_PIXELINFO(R16_SINT,             0,  1,  R   ,   SINT    );
    DEF_PIXELINFO(R16_SFLOAT,           0,  1,  R   ,   SFLOAT  );
    DEF_PIXELINFO(R16G16_UNORM,         0,  2,  RG  ,   UNORM   );
    DEF_PIXELINFO(R16G16_SNORM,         0,  2,  RG  ,   SNORM   );
    DEF_PIXELINFO(R16G16_USCALED,       0,  2,  RG  ,   USCALED );
    DEF_PIXELINFO(R16G16_SSCALED,       0,  2,  RG  ,   SSCALED );
    DEF_PIXELINFO(R16G16_UINT,          0,  2,  RG  ,   UINT    );
    DEF_PIXELINFO(R16G16_SINT,          0,  2,  RG  ,   SINT    );
    DEF_PIXELINFO(R16G16_SFLOAT,        0,  2,  RG  ,   SFLOAT  );
    DEF_PIXELINFO(R16G16B16A16_UNORM,   0,  3,  RGBA,   UNORM   );
    DEF_PIXELINFO(R16G16B16A16_SNORM,   0,  3,  RGBA,   SNORM   );
    DEF_PIXELINFO(R16G16B16A16_USCALED, 0,  3,  RGBA,   USCALED );
    DEF_PIXELINFO(R16G16B16A16_SSCALED, 0,  3,  RGBA,   SSCALED );
    DEF_PIXELINFO(R16G16B16A16_UINT,    0,  3,  RGBA,   UINT    );
    DEF_PIXELINFO(R16G16B16A16_SINT,    0,  3,  RGBA,   SINT    );
    DEF_PIXELINFO(R16G16B16A16_SFLOAT,  0,  3,  RGBA,   SFLOAT  );
    DEF_PIXELINFO(R32_UINT,             0,  2,  R   ,   UINT    );
    DEF_PIXELINFO(R32_SINT,             0,  2,  R   ,   SINT    );
    DEF_PIXELINFO(R32_SFLOAT,           0,  2,  R   ,   SFLOAT  );
    DEF_PIXELINFO(R32G32_UINT,          0,  3,  RG  ,   UINT    );
    DEF_PIXELINFO(R32G32_SINT,          0,  3,  RG  ,   SINT    );
    DEF_PIXELINFO(R32G32_SFLOAT,        0,  3,  RG  ,   SFLOAT  );
    DEF_PIXELINFO(R32G32B32A32_UINT,    0,  4,  RGBA,   UINT    );
    DEF_PIXELINFO(R32G32B32A32_SINT,    0,  4,  RGBA,   SINT    );
    DEF_PIXELINFO(R32G32B32A32_SFLOAT,  0,  4,  RGBA,   SFLOAT  );
    DEF_PIXELINFO(R64_UINT,             0,  3,  R   ,   UINT    );
    DEF_PIXELINFO(R64_SINT,             0,  3,  R   ,   SINT    );
    DEF_PIXELINFO(R64_SFLOAT,           0,  3,  R   ,   SFLOAT  );
    DEF_PIXELINFO(R64G64_UINT,          0,  4,  RG  ,   UINT    );
    DEF_PIXELINFO(R64G64_SINT,          0,  4,  RG  ,   SINT    );
    DEF_PIXELINFO(R64G64_SFLOAT,        0,  4,  RG  ,   SFLOAT  );
    DEF_PIXELINFO(R64G64B64A64_UINT,    0,  5,  RGBA,   UINT    );
    DEF_PIXELINFO(R64G64B64A64_SINT,    0,  5,  RGBA,   SINT    );
    DEF_PIXELINFO(R64G64B64A64_SFLOAT,  0,  5,  RGBA,   SFLOAT  );
    DEF_PIXELINFO(B10G11R11_UFLOAT,     0,  2,  BGR ,   UFLOAT  );
    DEF_PIXELINFO(E5B9G9R9_UFLOAT,      0,  2,  EBGR,   UFLOAT  );
    DEF_PIXELINFO(D16_UNORM,            0,  1,  R   ,   UNORM   );
    DEF_PIXELINFO(D32_SFLOAT,           0,  2,  R   ,   SFLOAT  );
    DEF_PIXELINFO(D24_UNORM_S8_UINT,    0,  2,  RA  ,   UNORM   );
    DEF_PIXELINFO(BC1_RGB_UNORM,        2,  0,  RGB ,   UNORM   );
    DEF_PIXELINFO(BC1_RGB_SRGB,         2,  0,  RGB ,   SRGB    );
    DEF_PIXELINFO(BC1_RGBA_UNORM,       2,  0,  RGBA,   UNORM   );
    DEF_PIXELINFO(BC1_RGBA_SRGB,        2,  0,  RGBA,   SRGB    );
    DEF_PIXELINFO(BC2_UNORM,            2,  1,  RGBA,   UNORM   );
    DEF_PIXELINFO(BC2_SRGB,             2,  1,  RGBA,   SRGB    );
    DEF_PIXELINFO(BC3_UNORM,            2,  1,  RGBA,   UNORM   );
    DEF_PIXELINFO(BC3_SRGB,             2,  1,  RGBA,   SRGB    );
    DEF_PIXELINFO(BC4_UNORM,            2,  0,  R   ,   UNORM   );
    DEF_PIXELINFO(BC4_SNORM,            2,  0,  R   ,   SNORM   );
    DEF_PIXELINFO(BC5_UNORM,            2,  1,  RG  ,   UNORM   );
    DEF_PIXELINFO(BC5_SNORM,            2,  1,  RG  ,   SNORM   );
    DEF_PIXELINFO(BC6H_UFLOAT,          2,  1,  RGB ,   UFLOAT  );
    DEF_PIXELINFO(BC6H_SFLOAT,          2,  1,  RGB ,   SFLOAT  );
    DEF_PIXELINFO(BC7_UNORM,            2,  1,  RGBA,   UNORM   );
    DEF_PIXELINFO(BC7_SRGB,             2,  1,  RGBA,   SRGB    );
    default:
        AssertNotImplemented();
    }

#undef DEF_PIXELINFO
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
size_t FGenericSurfaceFormat::BestAvailable(const TMemoryView<const FGenericSurfaceFormat>& surfaceFormats) NOEXCEPT {
    const bool enableHDR = FGenericInstance::GEnableHDR;

    int bestScore = 0;
    size_t bestIndex = INDEX_NONE;

    FGenericPixelInfo pixelInfo;
    forrange(i, 0, surfaceFormats.size()) {
        const FGenericSurfaceFormat& it = surfaceFormats[i];
        if (not enableHDR && it.ColorSpace != EGenericColorSpace::SRGB_NONLINEAR)
            continue;

        pixelInfo = FGenericPixelInfo::FromFormat(it.Format);

        int score = 0;
        score += pixelInfo.BitsPerPixel();

        switch (it.ColorSpace) {
        case EGenericColorSpace::PASS_THROUGH:
        case EGenericColorSpace::SRGB_NONLINEAR:
            break;

        case EGenericColorSpace::EXTENDED_SRGB_LINEAR:
        case EGenericColorSpace::EXTENDED_SRGB_NONLINEAR:
            score += 100;
            break;

        case EGenericColorSpace::HDR10_ST2084:
        case EGenericColorSpace::HDR10_HLG:
            score += 1000;
            break;

        case EGenericColorSpace::DISPLAY_P3_NONLINEAR:
        case EGenericColorSpace::DCI_P3_NONLINEAR:
        case EGenericColorSpace::BT709_NONLINEAR:
        case EGenericColorSpace::ADOBERGB_NONLINEAR:
            score += 2000;
            break;

        case EGenericColorSpace::DISPLAY_P3_LINEAR:
        case EGenericColorSpace::BT709_LINEAR:
        case EGenericColorSpace::BT2020_LINEAR:
        case EGenericColorSpace::DOLBYVISION:
        case EGenericColorSpace::ADOBERGB_LINEAR:
            score += 3000;
            break;

        case EGenericColorSpace::DISPLAY_NATIVE_AMD:
            score += 4000;
            break;

        default:
            AssertNotImplemented();
        }

        if (score > bestScore) {
            bestIndex = i;
            bestScore = score;
        }
    }

    Assert(INDEX_NONE != bestIndex);
    Assert_NoAssume(bestScore > 1);
    return bestIndex;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
