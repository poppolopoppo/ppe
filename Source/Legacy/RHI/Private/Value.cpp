﻿// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Value.h"

#include "IO/TextWriter.h"

#include "Memory/HashFunctions.h"

#include "Maths/ScalarVectorHelpers.h"
#include "Maths/PackingHelpers.h"
#include "Maths/ScalarMatrixHelpers.h"

namespace {
using namespace PPE;
using namespace PPE::Graphics;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
struct TValueSizeInBytes_ {
    static size_t Do() { return sizeof(T); }
};
template <>
struct TValueSizeInBytes_<void> {
    static size_t Do() { return 0; }
};
//----------------------------------------------------------------------------
template <typename U, typename V = U, typename = typename std::enable_if< std::is_constructible<U, const V&>::value >::type >
struct TValueCopy_ {
    static void Do(const TMemoryView<u8>& dst, const TMemoryView<const u8>& src) {
        Assert(dst.SizeInBytes() >= sizeof(U));
        Assert(src.SizeInBytes() >= sizeof(V));
        *reinterpret_cast<U*>(dst.data()) = *reinterpret_cast<const V*>(src.data());
    }
};
template <>
struct TValueCopy_<void, void, void> {
    static void Do(const TMemoryView<u8>& , const TMemoryView<const u8>& ) {
        AssertNotReached();
    }
};
//----------------------------------------------------------------------------
template <typename U, typename V = U, typename = typename std::enable_if< std::is_constructible<U, const V&>::value >::type >
struct TValueCopyArray_ {
    static void Do(
        const TMemoryView<u8>& dstValues, size_t dstOffset, size_t dstStride,
        const TMemoryView<const u8>& srcValues, size_t srcOffset, size_t srcStride,
        size_t count ) {
        Assert(dstStride >= dstOffset + sizeof(U));
        Assert(srcStride >= srcOffset + sizeof(V));
        Assert(dstValues.SizeInBytes() >= dstStride * count);
        Assert(srcValues.SizeInBytes() >= srcStride * count);

        u8* pdst = dstValues.data();
        const u8* psrc = srcValues.data();

        forrange(v, 0, count) {
            *reinterpret_cast<U*>(pdst + dstOffset) =
                *reinterpret_cast<const V*>(psrc + srcOffset);

            pdst += dstStride;
            psrc += srcStride;
        }
    }
};
template <>
struct TValueCopyArray_<void, void, void> {
    static void Do(
        const TMemoryView<u8>& , size_t , size_t ,
        const TMemoryView<const u8>& , size_t , size_t ,
        size_t ) {
        AssertNotReached();
    }
};
//----------------------------------------------------------------------------
template <typename T>
struct TValueDefault_ {
    static void Do(const TMemoryView<u8>& dst) {
        Assert(dst.SizeInBytes() >= sizeof(T));
        *reinterpret_cast<T*>(dst.data()) = T();
    }
};
template <>
struct TValueDefault_<void> {
    static void Do(const TMemoryView<u8>& ) {
        AssertNotReached();
    }
};
//----------------------------------------------------------------------------
template <typename T>
struct TValueEquals_ {
    static bool Do(const TMemoryView<const u8>& lhs, const TMemoryView<const u8>& rhs) {
        Assert(lhs.SizeInBytes() >= sizeof(T));
        Assert(rhs.SizeInBytes() >= sizeof(T));
        return (*reinterpret_cast<const T*>(lhs.data()) == *reinterpret_cast<const T*>(rhs.data()) );
    }
};
template <>
struct TValueEquals_<void> {
    static bool Do(const TMemoryView<const u8>& , const TMemoryView<const u8>& ) {
        AssertNotReached();
        return false;
    }
};
//----------------------------------------------------------------------------
template <typename T>
struct TValueHash_ {
    static size_t Do(const TMemoryView<const u8>& data) {
        Assert(data.SizeInBytes() >= sizeof(T));
        using PPE::hash_value;
        return hash_value(*reinterpret_cast<const T*>(data.data()));
    }
};
template <>
struct TValueHash_<void> {
    static size_t Do(const TMemoryView<const u8>& ) {
        AssertNotReached();
        return 0;
    }
};
//----------------------------------------------------------------------------
template <typename T>
struct TValueLerp_ {
    static void Do(const TMemoryView<u8>& dst, const TMemoryView<const u8>& a, const TMemoryView<const u8>& b, float t) {
        Assert(dst.SizeInBytes() >= sizeof(T));
        Assert(a.SizeInBytes() >= sizeof(T));
        Assert(b.SizeInBytes() >= sizeof(T));

        *reinterpret_cast<T*>(dst.data()) = Lerp(
            *reinterpret_cast<const T*>(a.data()),
            *reinterpret_cast<const T*>(b.data()),
            t );
    }
};
template <>
struct TValueLerp_<void> {
    static void Do(const TMemoryView<u8>& , const TMemoryView<const u8>& , const TMemoryView<const u8>& , float ) {
        AssertNotReached();
    }
};
//----------------------------------------------------------------------------
template <typename T>
struct TValueLerpArray_ {
    static void Do(const TMemoryView<u8>& dst, size_t dstStride, const TMemoryView<const u8>& a, const TMemoryView<const u8>& b, const TMemoryView<const float>& ts) {
        Assert(dst.SizeInBytes() >= dstStride * ts.size() + sizeof(T));
        Assert(dstStride >= sizeof(T));
        Assert(a.SizeInBytes() >= sizeof(T));
        Assert(b.SizeInBytes() >= sizeof(T));

        const T& va = *reinterpret_cast<const T*>(a.data());
        const T& vb = *reinterpret_cast<const T*>(b.data());

        u8* pdst = dst.data();
        for (float t : ts) {
            *reinterpret_cast<T*>(pdst) =
                Lerp(va, vb, t);

            pdst += dstStride;
        }
    }
};
template <>
struct TValueLerpArray_<void> {
    static void Do(const TMemoryView<u8>& , size_t , const TMemoryView<const u8>& , const TMemoryView<const u8>& , const TMemoryView<const float>& ) {
        AssertNotReached();
    }
};
//----------------------------------------------------------------------------
template <typename T>
struct TValueBarycentricLerp_ {
    static void Do(const TMemoryView<u8>& dst, const TMemoryView<const u8>& a, const TMemoryView<const u8>& b, const TMemoryView<const u8>& c, const float3& uvw) {
        Assert(dst.SizeInBytes() >= sizeof(T));
        Assert(a.SizeInBytes() >= sizeof(T));
        Assert(b.SizeInBytes() >= sizeof(T));
        Assert(c.SizeInBytes() >= sizeof(T));

        *reinterpret_cast<T*>(dst.data()) =
            BarycentricLerp(
                *reinterpret_cast<const T*>(a.data()),
                *reinterpret_cast<const T*>(b.data()),
                *reinterpret_cast<const T*>(c.data()),
                uvw.x(), uvw.y(), uvw.z() );
    }
};
template <>
struct TValueBarycentricLerp_<void> {
    static void Do(const TMemoryView<u8>& , const TMemoryView<const u8>& , const TMemoryView<const u8>& , const TMemoryView<const u8>& , const float3& ) {
        AssertNotReached();
    }
};
//----------------------------------------------------------------------------
template <typename T>
struct TValueBarycentricLerpArray_ {
    static void Do(const TMemoryView<u8>& dst, size_t dstStride, const TMemoryView<const u8>& a, const TMemoryView<const u8>& b, const TMemoryView<const u8>& c, const TMemoryView<const float3>& uvws) {
        Assert(dst.SizeInBytes() >= dstStride * uvws.size() + sizeof(T));
        Assert(dstStride >= sizeof(T));
        Assert(a.SizeInBytes() >= sizeof(T));
        Assert(b.SizeInBytes() >= sizeof(T));
        Assert(c.SizeInBytes() >= sizeof(T));

#if 0 // workaround MSVC 2017 compiler crash when AVX enabled
        const T& va = *reinterpret_cast<const T*>(a.data());
        const T& vb = *reinterpret_cast<const T*>(b.data());
        const T& vc = *reinterpret_cast<const T*>(c.data());

        u8* pdst = dst.data();
        for (const float3& uvw : uvws) {
            *reinterpret_cast<T*>(pdst) =
                BarycentricLerp(va, vb, vc, uvw.x(), uvw.y(), uvw.z());

            pdst += dstStride;
        }
#else
        forrange(i, 0, uvws.size())
            TValueBarycentricLerp_<T>::Do(dst.SubRange(i * dstStride, sizeof(T)), a, b, c, uvws[i]);
#endif
    }
};
template <>
struct TValueBarycentricLerpArray_<void> {
    static void Do(const TMemoryView<u8>& , size_t , const TMemoryView<const u8>& , const TMemoryView<const u8>& , const TMemoryView<const u8>& , const TMemoryView<const float3>& ) {
        AssertNotReached();
    }
};
//----------------------------------------------------------------------------
template <typename U, typename V>
struct FValueDoNothing_ {
    FORCE_INLINE static void Do() {}
};
//----------------------------------------------------------------------------
template <template <typename... > class _Functor, typename... _Args>
static bool ValuePromote_(EValueType dst, EValueType src, _Args&&... args) {
    switch (src)
    {
    case PPE::Graphics::EValueType::Float2:
        switch (dst)
        {
        case PPE::Graphics::EValueType::Half2:
            _Functor< half2, float2 >::Do(std::forward<_Args>(args)...);
            return true;
        case PPE::Graphics::EValueType::Byte2N:
            _Functor< byte2n, float2 >::Do(std::forward<_Args>(args)...);
            return true;
        case PPE::Graphics::EValueType::UByte2N:
            _Functor< ubyte2n, float2 >::Do(std::forward<_Args>(args)...);
            return true;
        case PPE::Graphics::EValueType::Short2N:
            _Functor< short2n, float2 >::Do(std::forward<_Args>(args)...);
            return true;
        case PPE::Graphics::EValueType::UShort2N:
            _Functor< ushort2n, float2 >::Do(std::forward<_Args>(args)...);
            return true;
        default:
            return false;
        }
        break;

    case PPE::Graphics::EValueType::Half2:
        if (PPE::Graphics::EValueType::Float2 == dst) {
            _Functor< float2, half2 >::Do(std::forward<_Args>(args)...);
            return true;
        }
        break;

    case PPE::Graphics::EValueType::Byte2N:
        if (PPE::Graphics::EValueType::Float2 == dst) {
            _Functor< float2, byte2n >::Do(std::forward<_Args>(args)...);
            return true;
        }
        break;
    case PPE::Graphics::EValueType::UByte2N:
        if (PPE::Graphics::EValueType::Float2 == dst) {
            _Functor< float2, ubyte2n >::Do(std::forward<_Args>(args)...);
            return true;
        }
        break;
    case PPE::Graphics::EValueType::Short2N:
        if (PPE::Graphics::EValueType::Float2 == dst) {
            _Functor< float2, short2n >::Do(std::forward<_Args>(args)...);
            return true;
        }
        break;
    case PPE::Graphics::EValueType::UShort2N:
        if (PPE::Graphics::EValueType::Float2 == dst) {
            _Functor< float2, ushort2n >::Do(std::forward<_Args>(args)...);
            return true;
        }
        break;

    case PPE::Graphics::EValueType::Float3:
        switch (dst)
        {
        case PPE::Graphics::EValueType::UX10Y10Z10W2N:
            _Functor< UX10Y10Z10W2N, float3 >::Do(std::forward<_Args>(args)...);
            return true;
        default:
            return false;
        }
        break;

    case PPE::Graphics::EValueType::Float4:
        switch (dst)
        {
        case PPE::Graphics::EValueType::Half4:
            _Functor< half4, float4 >::Do(std::forward<_Args>(args)...);
            return true;
        case PPE::Graphics::EValueType::Byte4N:
            _Functor< byte4n, float4 >::Do(std::forward<_Args>(args)...);
            return true;
        case PPE::Graphics::EValueType::UByte4N:
            _Functor< ubyte4n, float4 >::Do(std::forward<_Args>(args)...);
            return true;
        case PPE::Graphics::EValueType::Short4N:
            _Functor< short4n, float4 >::Do(std::forward<_Args>(args)...);
            return true;
        case PPE::Graphics::EValueType::UShort4N:
            _Functor< ushort4n, float4 >::Do(std::forward<_Args>(args)...);
            return true;
        case PPE::Graphics::EValueType::UX10Y10Z10W2N:
            _Functor< UX10Y10Z10W2N, float4 >::Do(std::forward<_Args>(args)...);
            return true;
        default:
            return false;
        }
        break;

    case PPE::Graphics::EValueType::Half4:
        if (PPE::Graphics::EValueType::Float4 == dst) {
            _Functor< float4, half4 >::Do(std::forward<_Args>(args)...);
            return true;
        }
        break;
    case PPE::Graphics::EValueType::Byte4N:
        if (PPE::Graphics::EValueType::Float4 == dst) {
            _Functor< float4, byte4n >::Do(std::forward<_Args>(args)...);
            return true;
        }
        break;
    case PPE::Graphics::EValueType::UByte4N:
        if (PPE::Graphics::EValueType::Float4 == dst) {
            _Functor< float4, ubyte4n >::Do(std::forward<_Args>(args)...);
            return true;
        }
        break;
    case PPE::Graphics::EValueType::Short4N:
        if (PPE::Graphics::EValueType::Float4 == dst) {
            _Functor< float4, short4n >::Do(std::forward<_Args>(args)...);
            return true;
        }
        break;
    case PPE::Graphics::EValueType::UShort4N:
        if (PPE::Graphics::EValueType::Float4 == dst) {
            _Functor< float4, ushort4n >::Do(std::forward<_Args>(args)...);
            return true;
        }
        break;

    case PPE::Graphics::EValueType::UX10Y10Z10W2N:
        if (PPE::Graphics::EValueType::Float3 == dst) {
            _Functor< float3, UX10Y10Z10W2N >::Do(std::forward<_Args>(args)...);
            return true;
        }
        if (PPE::Graphics::EValueType::Float4 == dst) {
            _Functor< float4, UX10Y10Z10W2N >::Do(std::forward<_Args>(args)...);
            return true;
        }
        break;

    default:
        break;
    }
    return false;
}
//----------------------------------------------------------------------------
template <typename T>
struct TValueSwap_ {
    static void Do(const TMemoryView<u8>& lhs, const TMemoryView<u8>& rhs) {
        Assert(lhs.SizeInBytes() >= sizeof(T));
        Assert(rhs.SizeInBytes() >= sizeof(T));
        using std::swap;
        using PPE::swap;
        swap(   *reinterpret_cast<T*>(lhs.data()),
                *reinterpret_cast<T*>(rhs.data()) );
    }
};
template <>
struct TValueSwap_<void> {
    static void Do(const TMemoryView<u8>& , const TMemoryView<u8>& ) {
        AssertNotReached();
    }
};
//----------------------------------------------------------------------------
template <typename _Ret, template < typename... > class _Functor, typename... _Args>
_Ret SwitchValue_(EValueType value, _Args&&... args) {
    switch (value)
    {
#define VALUETYPE_SWITCHVALUE_DEF(_Name, T, _TypeId, _Unused) \
    case PPE::Graphics::EValueType::_Name: \
        return _Functor< T >::Do(std::forward<_Args>(args)...);
    FOREACH_PPE_GRAPHIC_VALUETYPE(VALUETYPE_SWITCHVALUE_DEF)
#undef VALUETYPE_SWITCHVALUE_DEF

    case PPE::Graphics::EValueType::Void:
        AssertNotReached();
        break;
    }
    AssertNotImplemented();
    return _Ret();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace

namespace PPE {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FStringView ValueTypeToCStr(EValueType value) {
    switch (value)
    {
    case PPE::Graphics::EValueType::Void:
        return "void";
    case PPE::Graphics::EValueType::Float:
        return "float";
    case PPE::Graphics::EValueType::Float2:
        return "float2";
    case PPE::Graphics::EValueType::Float3:
        return "float3";
    case PPE::Graphics::EValueType::Float4:
        return "float4";
    case PPE::Graphics::EValueType::Float3x3:
        return "float3x3";
    case PPE::Graphics::EValueType::Float4x3:
        return "float4x3";
    case PPE::Graphics::EValueType::Float4x4:
        return "float4x4";
    case PPE::Graphics::EValueType::Bool:
        return "bool";
    case PPE::Graphics::EValueType::Byte:
        return "byte";
    case PPE::Graphics::EValueType::Byte2:
        return "byte2";
    case PPE::Graphics::EValueType::Byte4:
        return "byte4";
    case PPE::Graphics::EValueType::UByte:
        return "ubyte";
    case PPE::Graphics::EValueType::UByte2:
        return "ubyte2";
    case PPE::Graphics::EValueType::UByte4:
        return "ubyte4";
    case PPE::Graphics::EValueType::Short:
        return "short";
    case PPE::Graphics::EValueType::Short2:
        return "short2";
    case PPE::Graphics::EValueType::Short4:
        return "short4";
    case PPE::Graphics::EValueType::UShort:
        return "ushort";
    case PPE::Graphics::EValueType::UShort2:
        return "ushort2";
    case PPE::Graphics::EValueType::UShort4:
        return "ushort4";
    case PPE::Graphics::EValueType::Word:
        return "word";
    case PPE::Graphics::EValueType::Word2:
        return "word2";
    case PPE::Graphics::EValueType::Word3:
        return "word3";
    case PPE::Graphics::EValueType::Word4:
        return "word4";
    case PPE::Graphics::EValueType::UWord:
        return "uword";
    case PPE::Graphics::EValueType::UWord2:
        return "uword2";
    case PPE::Graphics::EValueType::UWord3:
        return "uword3";
    case PPE::Graphics::EValueType::UWord4:
        return "uword4";
    case PPE::Graphics::EValueType::Half:
        return "half";
    case PPE::Graphics::EValueType::Half2:
        return "half2";
    case PPE::Graphics::EValueType::Half4:
        return "half4";
    case PPE::Graphics::EValueType::Byte2N:
        return "byte2n";
    case PPE::Graphics::EValueType::Byte4N:
        return "byte4n";
    case PPE::Graphics::EValueType::UByte2N:
        return "ubyte2n";
    case PPE::Graphics::EValueType::UByte4N:
        return "ubyte4n";
    case PPE::Graphics::EValueType::Short2N:
        return "short2n";
    case PPE::Graphics::EValueType::Short4N:
        return "short4n";
    case PPE::Graphics::EValueType::UShort2N:
        return "ushort2n";
    case PPE::Graphics::EValueType::UShort4N:
        return "ushort4n";
    case PPE::Graphics::EValueType::UX10Y10Z10W2N:
        return "UX10Y10Z10W2N";
    }
    AssertNotImplemented();
    return FStringView();
}
//----------------------------------------------------------------------------
size_t ValueSizeInBytes(EValueType value) {
    return SwitchValue_<size_t, TValueSizeInBytes_>(value);
}
//----------------------------------------------------------------------------
void ValueCopy(EValueType type, const TMemoryView<u8>& dst, const TMemoryView<const u8>& src) {
    SwitchValue_<void, TValueCopy_>(type, dst, src);
}
//----------------------------------------------------------------------------
void ValueDefault(EValueType type, const TMemoryView<u8>& dst) {
    SwitchValue_<void, TValueDefault_>(type, dst);
}
//----------------------------------------------------------------------------
bool ValueEquals(EValueType type, const TMemoryView<const u8>& lhs, const TMemoryView<const u8>& rhs) {
    return SwitchValue_<bool, TValueEquals_>(type, lhs, rhs);
}
//----------------------------------------------------------------------------
hash_t ValueHash(EValueType type, const TMemoryView<const u8>& data) {
    return SwitchValue_<size_t, TValueHash_>(type, data);
}
//----------------------------------------------------------------------------
void ValueLerp(EValueType type, const TMemoryView<u8>& dst, const TMemoryView<const u8>& a, const TMemoryView<const u8>& b, float t) {
    SwitchValue_<void, TValueLerp_>(type, dst, a, b, t);
}
//----------------------------------------------------------------------------
void ValueLerpArray(EValueType type,
                    const TMemoryView<u8>& dst, size_t dstStride,
                    const TMemoryView<const u8>& a,
                    const TMemoryView<const u8>& b,
                    const TMemoryView<const float>& ts ) {
    SwitchValue_<void, TValueLerpArray_>(type, dst, dstStride, a, b, ts);
}
//----------------------------------------------------------------------------
void ValueBarycentricLerp(  EValueType type, const TMemoryView<u8>& dst,
                            const TMemoryView<const u8>& a,
                            const TMemoryView<const u8>& b,
                            const TMemoryView<const u8>& c,
                            const float3& uvw ) {
    SwitchValue_<void, TValueBarycentricLerp_>(type, dst, a, b, c, uvw);
}
//----------------------------------------------------------------------------
void ValueBarycentricLerpArray( EValueType type,
                                const TMemoryView<u8>& dst, size_t dstStride,
                                const TMemoryView<const u8>& a,
                                const TMemoryView<const u8>& b,
                                const TMemoryView<const u8>& c,
                                const TMemoryView<const float3>& uvws ) {
    SwitchValue_<void, TValueBarycentricLerpArray_>(type, dst, dstStride, a, b, c, uvws);
}
//----------------------------------------------------------------------------
bool ValueIsPromotable(EValueType dst, EValueType src) {
    return (src == dst || ValuePromote_<FValueDoNothing_>(dst, src));
}
//----------------------------------------------------------------------------
bool ValuePromote(EValueType output, const TMemoryView<u8>& dst, EValueType input, const TMemoryView<const u8>& src) {
    return (output == input)
        ? ValueCopy(output, dst, src), true
        : ValuePromote_<TValueCopy_>(output, input, dst, src);
}
//----------------------------------------------------------------------------
bool ValuePromoteArray( EValueType output, const TMemoryView<u8>& dst, size_t dstOffset, size_t dstStride,
                        EValueType input, const TMemoryView<const u8>& src, size_t srcOffset, size_t srcStride,
                        size_t count ) {
    Assert(0 != count);
    return (output == input)
        ? SwitchValue_<void, TValueCopyArray_>(output, dst, dstOffset, dstStride, src, srcOffset, srcStride, count), true
        : ValuePromote_<TValueCopyArray_>(output, input, dst, dstOffset, dstStride, src, srcOffset, srcStride, count);
}
//----------------------------------------------------------------------------
void ValueSwap(EValueType type, const TMemoryView<u8>& lhs, const TMemoryView<u8>& rhs) {
    SwitchValue_<void, TValueSwap_>(type, lhs, rhs);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, const FValue& value) {
    switch (value.Type()) {
    case EValueType::Void:
        oss << "void";
        break;

#define VALUETYPE_TO_OSS(_Name, T, _TypeId, _Unused) \
    case EValueType::_Name: \
        oss << value.Get< T >(); \
        break;
    FOREACH_PPE_GRAPHIC_VALUETYPE(VALUETYPE_TO_OSS)
#undef VALUETYPE_TO_OSS
    }
    return oss;
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, const FValue& value) {
    switch (value.Type()) {
    case EValueType::Void:
        oss << L"void";
        break;

#define VALUETYPE_TO_OSS(_Name, T, _TypeId, _Unused) \
    case EValueType::_Name: \
        oss << value.Get< T >(); \
        break;
    FOREACH_PPE_GRAPHIC_VALUETYPE(VALUETYPE_TO_OSS)
#undef VALUETYPE_TO_OSS
    }
    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE

#include "MetaAtom.h"
#include "MetaAtomVisitor.h"
#include "MetaTypeTraits.h"

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <typename T>
struct TValueToMetaAtom_ {
    static bool Do(RTTI::PMetaAtom& dst, const Graphics::FValue& src) {
        dst = RTTI::MakeAtom(src.Get<T>());
        return true;
    }
};
template <>
struct TValueToMetaAtom_<void> {
    static bool Do(RTTI::PMetaAtom& dst, const Graphics::FValue& src) {
        AssertNotReached();
        dst.reset();
        return false;
    }
};
//----------------------------------------------------------------------------
class FMetaAtomToValueVisitor_ : public IMetaAtomConstVisitor {
public:
    FMetaAtomToValueVisitor_(Graphics::FValue* pvalue) : _pvalue(pvalue) { Assert(_pvalue); }
    virtual ~FMetaAtomToValueVisitor_() {}

    virtual void Visit(const IMetaAtomPair* ) override { AssertNotImplemented(); }
    virtual void Visit(const IMetaAtomVector* ) override { AssertNotImplemented(); }
    virtual void Visit(const IMetaAtomDictionary* ) override { AssertNotImplemented(); }

#define DEF_METATYPE_SCALAR(_Name, T, _TypeId, _Unused) \
    virtual void Visit(const TMetaTypedAtom<T>* scalar) override { Visit_(scalar); }
    FOREACH_PPE_RTTI_NATIVE_TYPES(DEF_METATYPE_SCALAR)
#undef DEF_METATYPE_SCALAR

private:
    template <typename T>
    void Unwrap_(const TMetaTypedAtom<T>* scalar, std::true_type) {
        *_pvalue = scalar->Wrapper();
    }

    template <typename T>
    void Unwrap_(const TMetaTypedAtom<T>* scalar, std::false_type) {
        AssertNotImplemented();
    }

    template <typename T>
    void Visit_(const TMetaTypedAtom<T>* scalar) {
        typedef typename std::integral_constant< bool,
            (Graphics::TValueTraits<T>::TypeId != Graphics::EValueType::Void) >::type
            boolean_type;

        Unwrap_(scalar, boolean_type());
    }

    Graphics::FValue* _pvalue;
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
Graphics::EValueType GraphicsValueType(u32 typeId) {
    switch (typeId) {
#define SWITCH_ON_RTTI_NATIVE_TYPE(_Name, T, _TypeId, _Unused) \
    case TMetaType< T >::TypeId: \
        return Graphics::TValueTraits< T >::TypeId;
        FOREACH_PPE_RTTI_NATIVE_TYPES(SWITCH_ON_RTTI_NATIVE_TYPE)
#undef SWITCH_ON_RTTI_NATIVE_TYPE
    };
    return Graphics::EValueType::Void;
}
//----------------------------------------------------------------------------
bool GraphicsValueToAtomIFP(RTTI::PMetaAtom& dst, const Graphics::FValue& src) {
    Assert(!src.empty());
    return SwitchValue_<bool, TValueToMetaAtom_>(src.Type(), dst, src);
}
//----------------------------------------------------------------------------
bool AtomToGraphicsValueIFP(Graphics::FValue& dst, const RTTI::FMetaAtom& src) {
    dst.clear();
    FMetaAtomToValueVisitor_ visitor(&dst);
    src.Accept(&visitor);
    return (!dst.empty());
}
//----------------------------------------------------------------------------
void GraphicsValueToAtom(RTTI::PMetaAtom& dst, const Graphics::FValue& src) {
    if (not GraphicsValueToAtomIFP(dst, src))
        AssertNotReached();
}
//----------------------------------------------------------------------------
void AtomToGraphicsValue(Graphics::FValue& dst, const RTTI::FMetaAtom& src) {
    if (not AtomToGraphicsValueIFP(dst, src))
        AssertNotReached();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE
