#pragma once

#include "RHI_fwd.h"

#include "RHI/RenderStateEnums.h"
#include "RHI/SamplerEnums.h"

#include "Container/Array.h"
#include "Container/Hash.h"
#include "Container/TupleTie.h"
#include "Meta/Optional.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FSamplerDesc {
    using FAddressModes = TStaticArray<EAddressMode, 3>;

    EBorderColor BorderColor{ EBorderColor::FloatTransparentBlack };
    ETextureFilter MagFilter{ ETextureFilter::Nearest };
    ETextureFilter MinFilter{ ETextureFilter::Nearest };
    EMipmapFilter MipmapFilter{ EMipmapFilter::Nearest };
    FAddressModes AddressModes{ EAddressMode::Repeat, EAddressMode::Repeat, EAddressMode::Repeat };
    Meta::TOptional<ECompareOp> CompareOp;
    Meta::TOptional<float> MaxAnisotropy;
    float MipLodBias{ 0.0f };
    float MinLod{ -FLT_MAX };
    float MaxLod{ FLT_MAX };
    bool EnableUnnormalizedCoords{ false };

    FSamplerDesc& SetFilter(ETextureFilter mag, ETextureFilter min, EMipmapFilter mipmap) {
        MagFilter = mag;
        MinFilter = min;
        MipmapFilter = mipmap;
        return (*this);
    }

    FSamplerDesc& SetBorderColor(EBorderColor value) { BorderColor = value; return (*this); }
    FSamplerDesc& SetAddressMode(EAddressMode uvw) { AddressModes = { uvw, uvw, uvw }; return (*this); }
    FSamplerDesc& SetAddressMode(EAddressMode u, EAddressMode v, EAddressMode w) { AddressModes = { u, v, w }; return (*this); }
    FSamplerDesc& SetMipLodBias(float value) { MipLodBias = value; return (*this); }
    FSamplerDesc& SetLodRange(float min, float max) { Assert(min <= max); MinLod = min; MaxLod = max; return (*this); }
    FSamplerDesc& SetAnisotropy(float value) { MaxAnisotropy = value; return (*this); }
    FSamplerDesc& SetCompareOp(ECompareOp value) { CompareOp = value; return (*this); }
    FSamplerDesc& SetNormCoordinates(bool value) { EnableUnnormalizedCoords = value; return (*this); }

    TIE_AS_TUPLE_STRUCT(FSamplerDesc)
};
PPE_ASSUME_TYPE_AS_POD(FSamplerDesc);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
