#pragma once

#include "RHI_fwd.h"

#include "RHI/ResourceId.h"
#include "RHI/VertexEnums.h"

#include "Color/Color_fwd.h"
#include "Maths/Packing_fwd.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
struct TIndexDesc;
template <typename T>
CONSTEXPR EIndexFormat IndexAttrib() {
    return TIndexDesc<T>::Attrib;
}
//----------------------------------------------------------------------------
namespace details {
template <typename T, EIndexFormat _Attrib>
struct TIndexDescImpl {
    using type = T;
    STATIC_CONST_INTEGRAL(EIndexFormat, Attrib, _Attrib);
};
} //!details
//----------------------------------------------------------------------------
template <>
struct TIndexDesc<u16> : details::TIndexDescImpl<u16, EIndexFormat::UShort> {};
template <>
struct TIndexDesc<u32> : details::TIndexDescImpl<u16, EIndexFormat::UInt> {};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
struct TVertexDesc;
template <typename T>
CONSTEXPR EVertexFormat VertexAttrib() {
    return TVertexDesc<T>::Attrib;
}
//----------------------------------------------------------------------------
namespace details {
template <typename T, EVertexFormat _Attrib>
struct TVertexDescImpl {
    using type = T;
    STATIC_CONST_INTEGRAL(EVertexFormat, Attrib, _Attrib);
};
} //!details
//----------------------------------------------------------------------------
// Scalars
//----------------------------------------------------------------------------
template <>
struct TVertexDesc<FHalfFloat> : details::TVertexDescImpl<FHalfFloat, EVertexFormat::Half> {};
template <>
struct TVertexDesc<float> : details::TVertexDescImpl<float, EVertexFormat::Float> {};
template <>
struct TVertexDesc<double> : details::TVertexDescImpl<double, EVertexFormat::Double> {};
template <>
struct TVertexDesc<i8> : details::TVertexDescImpl<i8, EVertexFormat::Byte> {};
template <>
struct TVertexDesc<u8> : details::TVertexDescImpl<u8, EVertexFormat::UByte> {};
template <>
struct TVertexDesc<i16> : details::TVertexDescImpl<i16, EVertexFormat::Short> {};
template <>
struct TVertexDesc<u16> : details::TVertexDescImpl<u16, EVertexFormat::UShort> {};
template <>
struct TVertexDesc<i32> : details::TVertexDescImpl<i32, EVertexFormat::Int> {};
template <>
struct TVertexDesc<u32> : details::TVertexDescImpl<u32, EVertexFormat::UInt> {};
template <>
struct TVertexDesc<i64> : details::TVertexDescImpl<i64, EVertexFormat::Long> {};
template <>
struct TVertexDesc<u64> : details::TVertexDescImpl<u64, EVertexFormat::ULong> {};
template <>
struct TVertexDesc<byten> : details::TVertexDescImpl<byten, EVertexFormat::Byte_Norm> {};
template <>
struct TVertexDesc<shortn> : details::TVertexDescImpl<shortn, EVertexFormat::Short_Norm> {};
template <>
struct TVertexDesc<ubyten> : details::TVertexDescImpl<ubyten, EVertexFormat::UByte_Norm> {};
template <>
struct TVertexDesc<ushortn> : details::TVertexDescImpl<ushortn, EVertexFormat::UShort_Norm> {};
//----------------------------------------------------------------------------
// Colors
//----------------------------------------------------------------------------
template <>
struct TVertexDesc<FRgba32f> : details::TVertexDescImpl<FRgba32f, EVertexFormat::Float4> {};
template <>
struct TVertexDesc<FRgba32i> : details::TVertexDescImpl<FRgba32i, EVertexFormat::Int4> {};
template <>
struct TVertexDesc<FRgba32u> : details::TVertexDescImpl<FRgba32u, EVertexFormat::UInt4> {};
template <>
struct TVertexDesc<FRgba8u> : details::TVertexDescImpl<FRgba8u, EVertexFormat::UByte4_Norm> {};
template <>
struct TVertexDesc<FLinearColor> : details::TVertexDescImpl<FRgba32f, EVertexFormat::Float4> {};
template <>
struct TVertexDesc<FColor> : details::TVertexDescImpl<FRgba8u, EVertexFormat::UByte4_Norm> {};
//----------------------------------------------------------------------------
// Vectors
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
struct TVertexDesc< TScalarVector<T, _Dim> > : details::TVertexDescImpl<
    TScalarVector<T, _Dim>,
    BitAnd(VertexAttrib<T>(), EVertexFormat::_FlagsMask | EVertexFormat::_TypeMask) |
    static_cast<EVertexFormat>(_Dim << u32(EVertexFormat::_VecOffset))
>   {};
//----------------------------------------------------------------------------
// Decay
//----------------------------------------------------------------------------
template <typename T>
struct TVertexDesc<T&> : TVertexDesc<T> {};
template <typename T>
struct TVertexDesc<const T&> : TVertexDesc<T> {};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
using FVertexFormatPromote = bool (*)(const FRawMemory& dst, const FRawMemoryConst& src) NOEXCEPT;
NODISCARD PPE_RHI_API FVertexFormatPromote EVertexFormat_Promote(EVertexFormat dst, EVertexFormat src) NOEXCEPT;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
