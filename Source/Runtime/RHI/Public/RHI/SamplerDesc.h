#pragma once

#include "RHI_fwd.h"

#include "RHI/RenderStateEnums.h"
#include "RHI/SamplerEnums.h"

#include "Container/Array.h"
#include "Container/Hash.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FSamplerDesc {
    using FAddressModes = TStaticArray<EAddressMode, 3>;

    EBorderColor BorderColor{ EBorderColor::FloatTransparentBlack };
    ECompareOp CompareOp{ ECompareOp::Unknown };
    ETextureFilter MagFilter{ ETextureFilter::Nearest };
    ETextureFilter MinFilter{ ETextureFilter::Nearest };
    EMipmapFilter MipmapFilter{ EMipmapFilter::Nearest };
    FAddressModes AddressModes{ EAddressMode::Repeat, EAddressMode::Repeat, EAddressMode::Repeat };
    float MaxAnisotropy{ 0.0f };
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

    bool operator ==(const FSamplerDesc& other) const {
        return (BorderColor == other.BorderColor
            && CompareOp == other.CompareOp
            && MagFilter == other.MagFilter
            && MinFilter == other.MinFilter
            && MipmapFilter == other.MipmapFilter
            && AddressModes == other.AddressModes
            && MaxAnisotropy == other.MaxAnisotropy
            && MipLodBias == other.MipLodBias
            && MinLod == other.MinLod
            && MaxLod == other.MaxLod
            && EnableUnnormalizedCoords == other.EnableUnnormalizedCoords );
    }
    bool operator !=(const FSamplerDesc& other) const {
        return (not operator ==(other));
    }

    friend hash_t hash_value(const FSamplerDesc& desc) {
        return hash_tuple(
            desc.BorderColor, desc.CompareOp,
            desc.MagFilter, desc.MinFilter, desc.MipmapFilter,
            desc.AddressModes, desc.MaxAnisotropy,
            desc.MipLodBias, desc.MinLod, desc.MaxLod,
            desc.EnableUnnormalizedCoords );
    }

};
PPE_ASSUME_TYPE_AS_POD(FSamplerDesc);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
