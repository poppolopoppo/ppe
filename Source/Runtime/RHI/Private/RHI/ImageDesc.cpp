#include "stdafx.h"

#include "RHI/ImageDesc.h"

#include "RHI/EnumHelpers.h"
#include "RHI/ImageHelpers.h"
#include "RHI/PixelFormatHelpers.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static u32 NumMipmaps_(const uint3& dim) {
    return (FPlatformMaths::FloorLog2(MaxComponent(dim)) + 1);
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FImageDesc& FImageDesc::SetView(EImageView value) NOEXCEPT {
    View = value;

    switch (View) {
    case EImageView::_1D:
    case EImageView::_1DArray:
        Type = EImageDim_1D;
        break;

    case EImageView::_2D:
    case EImageView::_2DArray:
        Type = EImageDim_1D;
        break;

    case EImageView::_Cube:
    case EImageView::_CubeArray:
        Type = EImageDim::_2D;
        Flags |= EImageFlags::CubeCompatible;
        break;

    case EImageView::_3D:
        Type = EImageDim_3D;
        Flags |= EImageFlags::Array2DCompatible;
        break;

    case EImageView::Unknown:
    default: AssertNotImplemented();
    }

    return (*this);
}
//----------------------------------------------------------------------------
FImageDesc& FImageDesc::SetDimension(u32 value) NOEXCEPT {
    Dimensions = uint3(value, uint2::One);
    Type = (Default == Type ? EImageDim_1D : Type);
    return (*this);
}
//----------------------------------------------------------------------------
FImageDesc& FImageDesc::SetDimension(const uint2& value) NOEXCEPT {
    Dimensions = uint3(value, 1);
    Type = (Default == Type ? EImageDim_2D : Type);
    return (*this);
}
//----------------------------------------------------------------------------
FImageDesc& FImageDesc::SetDimension(const uint3& value) NOEXCEPT {
    Dimensions = value;
    Type = (Default == Type ? EImageDim_3D : Type);
    return (*this);
}
//----------------------------------------------------------------------------
void FImageDesc::Validate() {
    Assert(EImageDim::Unknown != Type);
    Assert(EPixelFormat::Unknown != Format);

    Dimensions = Max(Dimensions, uint3::One);
    ArrayLayers = Max(ArrayLayers, 1_layer);

    switch (Type) {
    case EImageDim::_1D:
        Assert_NoAssume(not Samples.Enabled());
        Assert_NoAssume(Dimensions.yz == uint2::One);
        Assert_NoAssume(not (Flags & EImageFlags::Array2DCompatible) &&
                        not (Flags & EImageFlags::CubeCompatible)); // those flags are not supported for 1D

        Flags -= (EImageFlags::Array2DCompatible | EImageFlags::CubeCompatible);
        Dimensions = uint3(Dimensions.x, uint2::One);
        Samples = 1_samples;
        break;

    case EImageDim::_2D:
        Assert_NoAssume(Dimensions.z == 1);
        if ((Flags & EImageFlags::CubeCompatible) and not Meta::IsAligned(6, *ArrayLayers))
            Flags -= EImageFlags::CubeCompatible;

        Dimensions.z = 1;
        break;

    case EImageDim::_3D:
        Assert_NoAssume(not Samples.Enabled());
        Assert_NoAssume(ArrayLayers == 1_layer);
        Assert_NoAssume(not (Flags & EImageFlags::CubeCompatible)); // this flag is not supported for 3D

        Flags -= EImageFlags::CubeCompatible;
        Samples = 1_samples;
        ArrayLayers = 1_layer;
        break;

    case EImageDim::Unknown:
    default: AssertNotImplemented();
    }

    // validate samples and mipmaps
    if (Samples.Enabled()) {
        Assert(MaxLevel <= 1_mipmap);
        MaxLevel = 1_mipmap;
    }
    else {
        Samples = 1_samples;
        MaxLevel = FMipmapLevel(Clamp(*MaxLevel, 1u, NumMipmaps_(Dimensions)));
    }

    // set default view
    if (View == Default) {
        switch (Type) {
        case EImageDim::_1D:
            View = (ArrayLayers > 1_layer
                ? EImageView_1DArray
                : EImageView_1D );
            break;

        case EImageDim::_2D:
            if (ArrayLayers > 6_layer && Flags & EImageFlags::CubeCompatible)
                View = EImageView_CubeArray;
            else if (ArrayLayers == 6_layer && Flags & EImageFlags::CubeCompatible)
                View = EImageView_Cube;
            else if (ArrayLayers > 1_layer)
                View = EImageView_2DArray;
            else
                View = EImageView_2D;
            break;

        case EImageDim::_3D:
            View = EImageView_3D;
            break;

        case EImageDim::Unknown:
        default: AssertNotImplemented();
        }
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FImageViewDesc::FImageViewDesc(const FImageDesc& desc)
:   View(desc.View)
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
    BaseLevel = FMipmapLevel{ Clamp(*BaseLevel, 0u, *desc.MaxLevel - 1) };
    LevelCount = Clamp(LevelCount, 1u, *desc.MaxLevel - *BaseLevel);

    // validate format
    if (Format == Default) {
        Format = desc.Format;
    }
    else if (Format != desc.Format && not (desc.Flags & EImageFlags::MutableFormat)) {
        AssertMessage_NoAssume(L"can't change format for immutable image", false);
        Format = desc.Format;
    }

    // validate aspect mask
    const EImageAspect mask = EPixelFormat_ToImageAspect(Format);
    AspectMask = (AspectMask == Default ? mask : BitAnd(AspectMask, mask));
    Assert_NoAssume(AspectMask != Default);


    if (View == Default) {
        // choose view type
        switch (desc.Type) {
        case EImageDim::_1D:
            View = (LayerCount > 1
                ? EImageView_1DArray
                : EImageView_1D );
            break;

        case EImageDim::_2D:
            if (LayerCount > 6_layer && desc.Flags & EImageFlags::CubeCompatible)
                View = EImageView_CubeArray;
            else if (LayerCount == 6_layer && desc.Flags & EImageFlags::CubeCompatible)
                View = EImageView_Cube;
            else if (LayerCount > 1_layer)
                View = EImageView_2DArray;
            else
                View = EImageView_2D;
            break;

        case EImageDim::_3D:
            View = EImageView_3D;
            break;

        case EImageDim::Unknown: break;
        default: ;
        }
    }
    else {
        // validate view type
        const u32 maxLayers = (desc.Type == EImageDim_3D && View != EImageView_3D
            ? desc.Dimensions.z.get()
            : *desc.ArrayLayers );

        BaseLayer = FImageLayer{ Clamp(*BaseLayer, 0u, maxLayers - 1) };

        switch (View) {
        case EImageView::_1D:
            Assert_NoAssume(desc.Type == EImageDim_1D);
            Assert_NoAssume(LayerCount == UMax || LayerCount == 1);
            LayerCount = 1;
            break;
        case EImageView::_1DArray:
            Assert_NoAssume(desc.Type == EImageDim_1D);
            LayerCount = Clamp(LayerCount, 1u, maxLayers - *BaseLayer);
            break;

        case EImageView::_2D:
            Assert_NoAssume((desc.Type == EImageDim_2D) ||
                            (desc.Type == EImageDim_3D && desc.Flags & EImageFlags::Array2DCompatible) );
            Assert_NoAssume(LayerCount == UMax || LayerCount == 1);
            LayerCount = 1;
            break;
        case EImageView::_2DArray:
            Assert_NoAssume((desc.Type == EImageDim_2D) ||
                            (desc.Type == EImageDim_3D && desc.Flags & EImageFlags::Array2DCompatible) );
            LayerCount = Clamp(LayerCount, 1u, maxLayers - *BaseLayer);
            break;

        case EImageView::_Cube:
            Assert_NoAssume((desc.Type == EImageDim_2D) ||
                            (desc.Type == EImageDim_3D && desc.Flags & EImageFlags::Array2DCompatible) );
            Assert_NoAssume(desc.Flags & EImageFlags::CubeCompatible);
            Assert_NoAssume(UMax == LayerCount || Meta::IsAligned(6u, LayerCount));
            LayerCount = 6;
            break;
        case EImageView::_CubeArray:
            Assert_NoAssume((desc.Type == EImageDim_2D) ||
                            (desc.Type == EImageDim_3D && desc.Flags & EImageFlags::Array2DCompatible) );
            Assert_NoAssume(desc.Flags & EImageFlags::CubeCompatible);
            Assert_NoAssume(UMax == LayerCount || Meta::IsAligned(6u, LayerCount));
            LayerCount = Max(1u, (maxLayers - *BaseLayer) / 6) * 6;
            break;

        case EImageView::_3D:
            Assert_NoAssume(desc.Type == EImageDim_3D);
            Assert_NoAssume(LayerCount == UMax || LayerCount == 1);
            LayerCount = 1;
            break;

        case EImageView::Unknown:
        default: AssertNotImplemented();
        }
    }

}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
