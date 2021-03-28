#include "stdafx.h"

#include "RHI/ImageDesc.h"

#include "RHI/EnumHelpers.h"
#include "RHI/ImageHelpers.h"
#include "RHI/PixelFormatInfo.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static void ValidateDimension_(uint3* pdim, FImageLayer* players, const EImageType imageType) {
    Assert(pdim);
    Assert(players);

    switch (imageType) {
    case EImageType::Tex1D: {
        Assert(0 < pdim->x);
        Assert(1 >= pdim->y && 1 >= pdim->z && 1 >= players->Value);
        *pdim = Max(uint3(pdim->x, 0, 0), 1u);
        *players = 1_layer;
        return;
    }
    case EImageType::Tex2D:
    case EImageType::Tex2DMS: {
        Assert(0 < pdim->x && 0 < pdim->y);
        Assert(1 >= pdim->z && 1 >= players->Value);
        *pdim = Max(uint3(pdim->xy, 0), 1u);
        *players = 1_layer;
        return;
    }
    case EImageType::TexCube: {
        Assert(0 < pdim->x && 0 < pdim->y && 1 >= pdim->z);
        Assert(pdim->x == pdim->y);
        Assert(6 == players->Value);
        *pdim = Max(uint3(pdim->xy, 1), 1u);
        *players = 6_layer;
        return;
    }
    case EImageType::Tex3D: {
        Assert(0 < pdim->x && 0 < pdim->y && 0 < pdim->z);
        Assert(1 >= players->Value);
        *pdim = Max(*pdim, 1u);
        *players = 1_layer;
        return;
    }

    case EImageType::Tex1DArray: {
        Assert(0 < pdim->x && 0 < players->Value);
        FALLTHROUGH();
    }
    case EImageType::Tex2DArray:
    case EImageType::Tex2DMSArray: {
        Assert(0 < pdim->x && 0 < pdim->y && 0 < players->Value);
        Assert(1 >= pdim->z);
        *pdim = Max(uint3(pdim->xy, 0), 1u);
        *players = Max(*players, 1_layer);
        return;
    }

    case EImageType::TexCubeArray: {
        Assert(0 < pdim->x && 0 < pdim->y && 1 >= pdim->z);
        Assert(pdim->x == pdim->y);
        Assert(players->Value > 0 && players->Value % 6 == 0);
        *pdim = Max(uint3(pdim->xy, 0), 1u);
        *players = Max(*players, 6_layer);
        return;
    }

    case EImageType::Unknown: AssertNotReached();
    }
}
//----------------------------------------------------------------------------
static u32 NumMipmaps_(const EImageType imageType, const uint3& dim) {
    switch (imageType) {
    case EImageType::Tex2DMS:
    case EImageType::Tex2DMSArray: return 1;

    case EImageType::Tex1D:
    case EImageType::Tex1DArray: return std::ilogb(dim.x.get()) + 1;

    case EImageType::Tex2D:
    case EImageType::Tex3D: return std::ilogb(MaxComponent(dim)) + 1;

    case EImageType::TexCube:
    case EImageType::TexCubeArray:
    case EImageType::Tex2DArray: return std::ilogb(MaxComponent(dim.xy)) + 1;

    case EImageType::Unknown: break;
    }
    AssertNotReached();
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FImageDesc::Validate() {
    Assert(EImageType::Unknown != Type);
    Assert(EPixelFormat::Unknown != Format);

    ValidateDimension_(&Dimensions, &ArrayLayers, Type);

    if (EImageType_IsMultiSampled(Type)) {
        Assert(Samples > 1_samples);
        Assert(MaxLevel <= 1_mipmap);
        MaxLevel = 1_mipmap;
    }
    else {
        Assert(Samples <= 1_samples);
        Samples = 1_samples;
        MaxLevel = FMipmapLevel(Clamp(*MaxLevel, 1u, NumMipmaps_(Type, Dimensions)));
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FImageViewDesc::FImageViewDesc(const FImageDesc& desc)
:   Type(desc.Type)
,   Format(desc.Format)
,   BaseLevel(0)
,   LevelCount(*desc.MaxLevel)
,   BaseLayer(0)
,   LayerCount(*desc.ArrayLayers)
,   Swizzle{"RGBA"_swizzle}
,   AspectMask(EPixelFormat_ToImageAspect(desc.Format))
{}
//----------------------------------------------------------------------------
void FImageViewDesc::Validate(const FImageDesc& desc) {
    if (Format == EPixelFormat::Unknown)
        Format = desc.Format;

    const u32 maxLayers = (EImageType::Tex3D == desc.Type ? 1 : *desc.ArrayLayers);
    BaseLayer = FImageLayer(Clamp(*BaseLayer, 0u, maxLayers - 1));
    LayerCount = Clamp(LayerCount, 1u, maxLayers - *BaseLayer);

    BaseLevel = FMipmapLevel(Clamp(*BaseLevel, 0u, *desc.MaxLevel - 1));
    LevelCount = Clamp(LevelCount, 1u, *desc.MaxLevel - *BaseLevel);

    const EImageAspect mask = EPixelFormat_ToImageAspect(Format);
    AspectMask = (AspectMask == Default ? mask : Meta::EnumAnd(AspectMask, mask));
    Assert(AspectMask != Default);

    if (EImageType::Unknown == Type) {
        switch (desc.Type) {
        case EImageType::TexCube:
            if (6 == LayerCount)
                Type = EImageType::TexCube;
            else if (1 == LayerCount)
                Type = EImageType::Tex2D;
            else
                Type = EImageType::Tex2DArray;
            break;
        case EImageType::TexCubeArray:
            if (LayerCount % 6 == 0)
                Type = EImageType::TexCubeArray;
            else if (1 == LayerCount)
                Type = EImageType::Tex2D;
            else
                Type = EImageType::Tex2DArray;
            break;

        case EImageType::Tex1DArray:
            Type = (1 == LayerCount ? EImageType::Tex1D : EImageType::Tex1DArray);
            break;
        case EImageType::Tex2DArray:
            Type = (1 == LayerCount ? EImageType::Tex2D : EImageType::Tex2DArray);
            break;
        case EImageType::Tex2DMSArray:
            Type = (1 == LayerCount ? EImageType::Tex2DMS : EImageType::Tex2DMSArray);
            break;

        case EImageType::Tex1D:
        case EImageType::Tex2D:
        case EImageType::Tex2DMS:
        case EImageType::Tex3D:
            Type = desc.Type;
            break;

        case EImageType::Unknown: AssertNotImplemented();
        }
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
