// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "RHI/VertexDesc.h"

#include "RHI/EnumHelpers.h"

#include "Maths/PackingHelpers.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <typename T>
static bool VertexCopy_Trivial_(const FRawMemory& dst, const FRawMemoryConst& src) NOEXCEPT {
    src.template Cast<const T>().CopyTo(dst.Cast<T>());
    return true;
}
//----------------------------------------------------------------------------
template <typename _Dst, typename _Src>
static bool VertexCopy_Promote_(const FRawMemory& dst, const FRawMemoryConst& src) NOEXCEPT {
    const TMemoryView<_Dst> dstT = dst.template Cast<_Dst>();
    const TMemoryView<const _Src> srcT = src.template Cast<const _Src>();
    Assert(dstT.size() == srcT.size());

    std::copy(srcT.begin(), srcT.end(), dstT.begin());
    return true;
}
//----------------------------------------------------------------------------
template <typename _Dst, typename _Src>
struct TVertexCopy_Checked_ {
    static FVertexFormatPromote Make(u32 arity) NOEXCEPT {
        Unused(arity);
        return [](const FRawMemory& dst, const FRawMemoryConst& src) NOEXCEPT -> bool{
            const TMemoryView<_Dst> dstT = dst.template Cast<_Dst>();
            const TMemoryView<const _Src> srcT = src.template Cast<const _Src>();
            Assert(dstT.size() == srcT.size());

            forrange(v, 0, srcT.size()) {
                dstT[v] = static_cast<_Dst>(srcT[v]);
                if (static_cast<_Dst>(dstT[v]) != srcT[v])
                    return false;
            }

            return true;
        };
    }
};
// special overload for quantization
template <typename _Dst, typename _Traits>
struct TVertexCopy_Checked_<TBasicNorm<_Dst, _Traits>, float> {
    static FVertexFormatPromote Make(u32 arity) NOEXCEPT {
        Unused(arity);
        return &VertexCopy_Promote_<TBasicNorm<_Dst, _Traits>, float>;
    }
};
// double can't be converted to anything
template <typename _Dst>
struct TVertexCopy_Checked_<_Dst, double> {
    static FVertexFormatPromote Make(u32 arity) NOEXCEPT {
        Unused(arity);
        return nullptr;
    }
};
//----------------------------------------------------------------------------
template <typename... _Args>
struct TVertexFormatPromote_ {
    template <typename... _Remaining>
    static FVertexFormatPromote Make(u32 arity, EVertexFormat fmt, _Remaining... remaining) NOEXCEPT {
        if (EVertexFormat_IsSignedInt(fmt)) {
            switch (EVertexFormat_Type(fmt)) {
            case EVertexFormat::Byte:
                return TVertexFormatPromote_<_Args..., i8>::Make(arity, remaining...);
            case EVertexFormat::Short:
                return TVertexFormatPromote_<_Args..., i16>::Make(arity, remaining...);
            case EVertexFormat::Int:
                return TVertexFormatPromote_<_Args..., i32>::Make(arity, remaining...);
            case EVertexFormat::Long:
                return TVertexFormatPromote_<_Args..., i64>::Make(arity, remaining...);
            default:
                break;
            }
        }
        else if (EVertexFormat_IsUnsignedInt(fmt)) {
            switch (EVertexFormat_Type(fmt)) {
            case EVertexFormat::UByte:
                return TVertexFormatPromote_<_Args..., u8>::Make(arity, remaining...);
            case EVertexFormat::UShort:
                return TVertexFormatPromote_<_Args..., u16>::Make(arity, remaining...);
            case EVertexFormat::UInt:
                return TVertexFormatPromote_<_Args..., u32>::Make(arity, remaining...);
            case EVertexFormat::ULong:
                return TVertexFormatPromote_<_Args..., u64>::Make(arity, remaining...);
            default:
                break;
            }
        }
        else if (EVertexFormat_IsFloatingPoint(fmt)) {
            switch (EVertexFormat_Type(fmt)) {
            case EVertexFormat::_Byte:
                Assert_NoAssume(EVertexFormat_IsNormalized(fmt));
                return TVertexFormatPromote_<_Args..., byten>::Make(arity, remaining...);
            case EVertexFormat::_Short:
                Assert_NoAssume(EVertexFormat_IsNormalized(fmt));
                return TVertexFormatPromote_<_Args..., shortn>::Make(arity, remaining...);
            case EVertexFormat::_UByte:
                Assert_NoAssume(EVertexFormat_IsNormalized(fmt));
                return TVertexFormatPromote_<_Args..., ubyten>::Make(arity, remaining...);
            case EVertexFormat::_UShort:
                Assert_NoAssume(EVertexFormat_IsNormalized(fmt));
                return TVertexFormatPromote_<_Args..., ushortn>::Make(arity, remaining...);
            case EVertexFormat::_Half:
                return TVertexFormatPromote_<_Args..., FHalfFloat>::Make(arity, remaining...);
            case EVertexFormat::_Float:
                return TVertexFormatPromote_<_Args..., float>::Make(arity, remaining...);
            case EVertexFormat::_Double:
                return TVertexFormatPromote_<_Args..., double>::Make(arity, remaining...);
            default:
                break;
            }
        }

        AssertNotImplemented();
    }
};
template <typename _Dst, typename _Src>
struct TVertexFormatPromote_<_Dst, _Src> {
    static FVertexFormatPromote Make(u32 arity) NOEXCEPT {
        // use promote copy only when dst is smaller than src

        IF_CONSTEXPR(std::is_same_v<_Dst, _Src>) {
            // same types, same sizes
            return &VertexCopy_Trivial_<_Dst>;
        }
        else IF_CONSTEXPR( // special exception for normalized/scaled
            EVertexFormat_Flags(VertexAttrib<_Dst>()) != Zero &&
            EVertexFormat_Flags(VertexAttrib<_Src>()) != Zero) {
            return nullptr; // forbidden
        }
        else IF_CONSTEXPR( // special exception for half
            EVertexFormat_Type(VertexAttrib<_Dst>()) == EVertexFormat::_Half ||
            EVertexFormat_Type(VertexAttrib<_Src>()) == EVertexFormat::_Half) {
            return nullptr; // forbidden
        }
        else IF_CONSTEXPR( // both signed, different sizes
            EVertexFormat_IsSignedInt(VertexAttrib<_Dst>()) &&
            EVertexFormat_IsSignedInt(VertexAttrib<_Src>())) {
            IF_CONSTEXPR(sizeof(_Dst) >= sizeof(_Src))
                return &VertexCopy_Promote_<_Dst, _Src>;
            else
                return TVertexCopy_Checked_<_Dst, _Src>::Make(arity);
        }
        else IF_CONSTEXPR( // both unsigned, different sizes
            EVertexFormat_IsUnsignedInt(VertexAttrib<_Dst>()) &&
            EVertexFormat_IsUnsignedInt(VertexAttrib<_Src>())) {
            IF_CONSTEXPR(sizeof(_Dst) >= sizeof(_Src))
                return &VertexCopy_Promote_<_Dst, _Src>;
            else
                return TVertexCopy_Checked_<_Dst, _Src>::Make(arity);
        }
        else IF_CONSTEXPR( // both floating point, different sizes
            EVertexFormat_IsFloatingPoint(VertexAttrib<_Dst>()) &&
            EVertexFormat_IsFloatingPoint(VertexAttrib<_Src>())) {;
            IF_CONSTEXPR(sizeof(_Dst) >= sizeof(_Src))
                return &VertexCopy_Promote_<_Dst, _Src>;
            else
                return TVertexCopy_Checked_<_Dst, _Src>::Make(arity);
        }
        else {
            return nullptr;
        }
    }
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FVertexFormatPromote EVertexFormat_Promote(EVertexFormat dst, EVertexFormat src) NOEXCEPT {
    const u32 arity = EVertexFormat_Arity(src);
    if (EVertexFormat_Arity(dst) == arity &&
        EVertexFormat_IsSignedInt(dst) == EVertexFormat_IsSignedInt(src) &&
        EVertexFormat_IsUnsignedInt(dst) == EVertexFormat_IsUnsignedInt(src) &&
        EVertexFormat_IsFloatingPoint(dst) == EVertexFormat_IsFloatingPoint(src) )
        return TVertexFormatPromote_<>::Make(arity, dst, src);

    return nullptr;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
