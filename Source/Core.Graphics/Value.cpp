#include "stdafx.h"

#include "Value.h"

#include "Core/Memory/HashFunctions.h"

#include "Core/Maths/ScalarVectorHelpers.h"
#include "Core/Maths/PackingHelpers.h"
#include "Core/Maths/ScalarMatrixHelpers.h"

namespace {
using namespace Core;
using namespace Core::Graphics;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
struct ValueSizeInBytes_ {
    size_t operator ()() const { return sizeof(T); }
};
template <>
struct ValueSizeInBytes_<void> {
    size_t operator ()() const { return 0; }
};
//----------------------------------------------------------------------------
template <typename U, typename V = U, typename = std::enable_if< std::is_constructible<U, const V&>::value >::type >
struct ValueCopy_ {
    void operator ()(const MemoryView<u8>& dst, const MemoryView<const u8>& src) const {
        Assert(dst.SizeInBytes() >= sizeof(U));
        Assert(src.SizeInBytes() >= sizeof(V));
        *reinterpret_cast<U*>(dst.data()) = *reinterpret_cast<const V*>(src.data());
    }
};
template <>
struct ValueCopy_<void, void, void> {
    void operator ()(const MemoryView<u8>& , const MemoryView<const u8>& ) {
        AssertNotReached();
    }
};
//----------------------------------------------------------------------------
template <typename U, typename V = U, typename = std::enable_if< std::is_constructible<U, const V&>::value >::type >
struct ValueCopyArray_ {
    void operator ()(const MemoryView<u8>& dst, size_t dstStride, const MemoryView<const u8>& src, size_t srcStride, size_t count) const {
        Assert(dst.SizeInBytes() >= dstStride * count + sizeof(U));
        Assert(src.SizeInBytes() >= srcStride * count + sizeof(V));

        u8* pdst = dst.data();
        const u8* psrc = src.data();

        forrange(v, 0, count) {
            *reinterpret_cast<U*>(pdst) =
                *reinterpret_cast<const V*>(psrc);

            pdst += dstStride;
            psrc += srcStride;
        }
    }
};
template <>
struct ValueCopyArray_<void, void, void> {
    void operator ()(const MemoryView<u8>& , size_t , const MemoryView<const u8>& , size_t , size_t ) {
        AssertNotReached();
    }
};
//----------------------------------------------------------------------------
template <typename T>
struct ValueDefault_ {
    void operator ()(const MemoryView<u8>& dst) const {
        Assert(dst.SizeInBytes() >= sizeof(T));
        *reinterpret_cast<T*>(dst.data()) = T();
    }
};
template <>
struct ValueDefault_<void> {
    void operator ()(const MemoryView<u8>& ) const {
        AssertNotReached();
    }
};
//----------------------------------------------------------------------------
template <typename T>
struct ValueEquals_ {
    bool operator ()(const MemoryView<const u8>& lhs, const MemoryView<const u8>& rhs) const {
        Assert(lhs.SizeInBytes() >= sizeof(T));
        Assert(rhs.SizeInBytes() >= sizeof(T));
        return (*reinterpret_cast<const T*>(lhs.data()) == *reinterpret_cast<const T*>(rhs.data()) );
    }
};
template <>
struct ValueEquals_<void> {
    bool operator ()(const MemoryView<const u8>& , const MemoryView<const u8>& ) const {
        AssertNotReached();
        return false;
    }
};
//----------------------------------------------------------------------------
template <typename T>
struct ValueHash_ {
    size_t operator ()(const MemoryView<const u8>& data) const {
        Assert(data.SizeInBytes() >= sizeof(T));
        using Core::hash_value;
        return hash_value(*reinterpret_cast<const T*>(data.data()));
    }
};
template <>
struct ValueHash_<void> {
    size_t operator ()(const MemoryView<const u8>& ) const {
        AssertNotReached();
        return 0;
    }
};
//----------------------------------------------------------------------------
template <typename T>
struct ValueLerp_ {
    void operator ()(const MemoryView<u8>& dst, const MemoryView<const u8>& a, const MemoryView<const u8>& b, float t) const {
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
struct ValueLerp_<void> {
    void operator ()(const MemoryView<u8>& , const MemoryView<const u8>& , const MemoryView<const u8>& , float ) const {
        AssertNotReached();
    }
};
//----------------------------------------------------------------------------
template <typename T>
struct ValueLerpArray_ {
    void operator ()(const MemoryView<u8>& dst, size_t dstStride, const MemoryView<const u8>& a, const MemoryView<const u8>& b, const MemoryView<const float>& ts) const {
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
struct ValueLerpArray_<void> {
    void operator ()(const MemoryView<u8>& , size_t , const MemoryView<const u8>& , const MemoryView<const u8>& , const MemoryView<const float>& ) {
        AssertNotReached();
    }
};
//----------------------------------------------------------------------------
template <typename T>
struct ValueBarycentricLerp_ {
    void operator ()(const MemoryView<u8>& dst, const MemoryView<const u8>& a, const MemoryView<const u8>& b, const MemoryView<const u8>& c, const float3& uvw) const {
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
struct ValueBarycentricLerp_<void> {
    void operator ()(const MemoryView<u8>& , const MemoryView<const u8>& , const MemoryView<const u8>& , const MemoryView<const u8>& , const float3& ) const {
        AssertNotReached();
    }
};
//----------------------------------------------------------------------------
template <typename T>
struct ValueBarycentricLerpArray_ {
    void operator ()(const MemoryView<u8>& dst, size_t dstStride, const MemoryView<const u8>& a, const MemoryView<const u8>& b, const MemoryView<const u8>& c, const MemoryView<const float3>& uvws) const {
        Assert(dst.SizeInBytes() >= dstStride * uvws.size() + sizeof(T));
        Assert(dstStride >= sizeof(T));
        Assert(a.SizeInBytes() >= sizeof(T));
        Assert(b.SizeInBytes() >= sizeof(T));
        Assert(c.SizeInBytes() >= sizeof(T));

        const T& va = *reinterpret_cast<const T*>(a.data());
        const T& vb = *reinterpret_cast<const T*>(b.data());
        const T& vc = *reinterpret_cast<const T*>(c.data());

        u8* pdst = dst.data();
        for (const float3& uvw : uvws) {
            *reinterpret_cast<T*>(pdst) =
                BarycentricLerp(va, vb, vc, uvw.x(), uvw.y(), uvw.z());

            pdst += dstStride;
        }
    }
};
template <>
struct ValueBarycentricLerpArray_<void> {
    void operator ()(const MemoryView<u8>& , size_t , const MemoryView<const u8>& , const MemoryView<const u8>& , const MemoryView<const u8>& , const MemoryView<const float3>& ) {
        AssertNotReached();
    }
};
//----------------------------------------------------------------------------
template <typename U, typename V>
struct ValueDoNothing_ {
    FORCE_INLINE void operator ()() const {}
};
//----------------------------------------------------------------------------
template <template <typename... > class _Functor, typename... _Args>
static bool ValuePromote_(ValueType dst, ValueType src, _Args&&... args) {
    switch (src)
    {
    case Core::Graphics::ValueType::Float2:
        switch (dst)
        {
        case Core::Graphics::ValueType::Half2:
            _Functor< half2, float2 >()(std::forward<_Args>(args)...);
            return true;
        case Core::Graphics::ValueType::Byte2N:
            _Functor< byte2n, float2 >()(std::forward<_Args>(args)...);
            return true;
        case Core::Graphics::ValueType::UByte2N:
            _Functor< ubyte2n, float2 >()(std::forward<_Args>(args)...);
            return true;
        case Core::Graphics::ValueType::Short2N:
            _Functor< short2n, float2 >()(std::forward<_Args>(args)...);
            return true;
        case Core::Graphics::ValueType::UShort2N:
            _Functor< ushort2n, float2 >()(std::forward<_Args>(args)...);
            return true;
        default:
            return false;
        }
        break;

    case Core::Graphics::ValueType::Half2:
        if (Core::Graphics::ValueType::Float2 == dst) {
            _Functor< float2, half2 >()(std::forward<_Args>(args)...);
            return true;
        }
        break;

    case Core::Graphics::ValueType::Byte2N:
        if (Core::Graphics::ValueType::Float2 == dst) {
            _Functor< float2, byte2n >()(std::forward<_Args>(args)...);
            return true;
        }
        break;
    case Core::Graphics::ValueType::UByte2N:
        if (Core::Graphics::ValueType::Float2 == dst) {
            _Functor< float2, ubyte2n >()(std::forward<_Args>(args)...);
            return true;
        }
        break;
    case Core::Graphics::ValueType::Short2N:
        if (Core::Graphics::ValueType::Float2 == dst) {
            _Functor< float2, short2n >()(std::forward<_Args>(args)...);
            return true;
        }
        break;
    case Core::Graphics::ValueType::UShort2N:
        if (Core::Graphics::ValueType::Float2 == dst) {
            _Functor< float2, ushort2n >()(std::forward<_Args>(args)...);
            return true;
        }
        break;

    case Core::Graphics::ValueType::Float3:
        switch (dst)
        {
        case Core::Graphics::ValueType::UX10Y10Z10W2N:
            _Functor< UX10Y10Z10W2N, float3 >()(std::forward<_Args>(args)...);
            return true;
        default:
            return false;
        }
        break;

    case Core::Graphics::ValueType::Float4:
        switch (dst)
        {
        case Core::Graphics::ValueType::Half4:
            _Functor< half4, float4 >()(std::forward<_Args>(args)...);
            return true;
        case Core::Graphics::ValueType::Byte4N:
            _Functor< byte4n, float4 >()(std::forward<_Args>(args)...);
            return true;
        case Core::Graphics::ValueType::UByte4N:
            _Functor< ubyte4n, float4 >()(std::forward<_Args>(args)...);
            return true;
        case Core::Graphics::ValueType::Short4N:
            _Functor< short4n, float4 >()(std::forward<_Args>(args)...);
            return true;
        case Core::Graphics::ValueType::UShort4N:
            _Functor< ushort4n, float4 >()(std::forward<_Args>(args)...);
            return true;
        case Core::Graphics::ValueType::UX10Y10Z10W2N:
            _Functor< UX10Y10Z10W2N, float4 >()(std::forward<_Args>(args)...);
            return true;
        default:
            return false;
        }
        break;

    case Core::Graphics::ValueType::Half4:
        if (Core::Graphics::ValueType::Float4 == dst) {
            _Functor< float4, half4 >()(std::forward<_Args>(args)...);
            return true;
        }
        break;
    case Core::Graphics::ValueType::Byte4N:
        if (Core::Graphics::ValueType::Float4 == dst) {
            _Functor< float4, byte4n >()(std::forward<_Args>(args)...);
            return true;
        }
        break;
    case Core::Graphics::ValueType::UByte4N:
        if (Core::Graphics::ValueType::Float4 == dst) {
            _Functor< float4, ubyte4n >()(std::forward<_Args>(args)...);
            return true;
        }
        break;
    case Core::Graphics::ValueType::Short4N:
        if (Core::Graphics::ValueType::Float4 == dst) {
            _Functor< float4, short4n >()(std::forward<_Args>(args)...);
            return true;
        }
        break;
    case Core::Graphics::ValueType::UShort4N:
        if (Core::Graphics::ValueType::Float4 == dst) {
            _Functor< float4, ushort4n >()(std::forward<_Args>(args)...);
            return true;
        }
        break;

    case Core::Graphics::ValueType::UX10Y10Z10W2N:
        if (Core::Graphics::ValueType::Float3 == dst) {
            _Functor< float3, UX10Y10Z10W2N >()(std::forward<_Args>(args)...);
            return true;
        }
        if (Core::Graphics::ValueType::Float4 == dst) {
            _Functor< float4, UX10Y10Z10W2N >()(std::forward<_Args>(args)...);
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
struct ValueSwap_ {
    void operator ()(const MemoryView<u8>& lhs, const MemoryView<u8>& rhs) const {
        Assert(lhs.SizeInBytes() >= sizeof(T));
        Assert(rhs.SizeInBytes() >= sizeof(T));
        using std::swap;
        using Core::swap;
        swap(   *reinterpret_cast<T*>(lhs.data()),
                *reinterpret_cast<T*>(rhs.data()) );
    }
};
template <>
struct ValueSwap_<void> {
    void operator ()(const MemoryView<u8>& , const MemoryView<u8>& ) {
        AssertNotReached();
    }
};
//----------------------------------------------------------------------------
template <typename _Ret, template < typename... > class _Functor, typename... _Args>
_Ret SwitchValue_(ValueType value, _Args&&... args) {
    switch (value)
    {
#define VALUETYPE_SWITCHVALUE_DEF(_Name, T, _TypeId, _Unused) \
    case Core::Graphics::ValueType::_Name: \
        return _Functor< T >()(std::forward<_Args>(args)...);
    FOREACH_CORE_GRAPHIC_VALUETYPE(VALUETYPE_SWITCHVALUE_DEF)
#undef VALUETYPE_SWITCHVALUE_DEF

    case Core::Graphics::ValueType::Void:
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

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
StringSlice ValueTypeToCStr(ValueType value) {
    switch (value)
    {
    case Core::Graphics::ValueType::Void:
        return "void";
    case Core::Graphics::ValueType::Float:
        return "float";
    case Core::Graphics::ValueType::Float2:
        return "float2";
    case Core::Graphics::ValueType::Float3:
        return "float3";
    case Core::Graphics::ValueType::Float4:
        return "float4";
    case Core::Graphics::ValueType::Float3x3:
        return "float3x3";
    case Core::Graphics::ValueType::Float4x3:
        return "float4x3";
    case Core::Graphics::ValueType::Float4x4:
        return "float4x4";
    case Core::Graphics::ValueType::Bool:
        return "bool";
    case Core::Graphics::ValueType::Byte:
        return "byte";
    case Core::Graphics::ValueType::Byte2:
        return "byte2";
    case Core::Graphics::ValueType::Byte4:
        return "byte4";
    case Core::Graphics::ValueType::UByte:
        return "ubyte";
    case Core::Graphics::ValueType::UByte2:
        return "ubyte2";
    case Core::Graphics::ValueType::UByte4:
        return "ubyte4";
    case Core::Graphics::ValueType::Short:
        return "short";
    case Core::Graphics::ValueType::Short2:
        return "short2";
    case Core::Graphics::ValueType::Short4:
        return "short4";
    case Core::Graphics::ValueType::UShort:
        return "ushort";
    case Core::Graphics::ValueType::UShort2:
        return "ushort2";
    case Core::Graphics::ValueType::UShort4:
        return "ushort4";
    case Core::Graphics::ValueType::Word:
        return "word";
    case Core::Graphics::ValueType::Word2:
        return "word2";
    case Core::Graphics::ValueType::Word3:
        return "word3";
    case Core::Graphics::ValueType::Word4:
        return "word4";
    case Core::Graphics::ValueType::UWord:
        return "uword";
    case Core::Graphics::ValueType::UWord2:
        return "uword2";
    case Core::Graphics::ValueType::UWord3:
        return "uword3";
    case Core::Graphics::ValueType::UWord4:
        return "uword4";
    case Core::Graphics::ValueType::Half:
        return "half";
    case Core::Graphics::ValueType::Half2:
        return "half2";
    case Core::Graphics::ValueType::Half4:
        return "half4";
    case Core::Graphics::ValueType::Byte2N:
        return "byte2n";
    case Core::Graphics::ValueType::Byte4N:
        return "byte4n";
    case Core::Graphics::ValueType::UByte2N:
        return "ubyte2n";
    case Core::Graphics::ValueType::UByte4N:
        return "ubyte4n";
    case Core::Graphics::ValueType::Short2N:
        return "short2n";
    case Core::Graphics::ValueType::Short4N:
        return "short4n";
    case Core::Graphics::ValueType::UShort2N:
        return "ushort2n";
    case Core::Graphics::ValueType::UShort4N:
        return "ushort4n";
    case Core::Graphics::ValueType::UX10Y10Z10W2N:
        return "UX10Y10Z10W2N";
    }
    AssertNotImplemented();
    return StringSlice();
}
//----------------------------------------------------------------------------
size_t ValueSizeInBytes(ValueType value) {
    return SwitchValue_<size_t, ValueSizeInBytes_>(value);
}
//----------------------------------------------------------------------------
void ValueCopy(ValueType type, const MemoryView<u8>& dst, const MemoryView<const u8>& src) {
    SwitchValue_<void, ValueCopy_>(type, dst, src);
}
//----------------------------------------------------------------------------
void ValueDefault(ValueType type, const MemoryView<u8>& dst) {
    SwitchValue_<void, ValueDefault_>(type, dst);
}
//----------------------------------------------------------------------------
bool ValueEquals(ValueType type, const MemoryView<const u8>& lhs, const MemoryView<const u8>& rhs) {
    return SwitchValue_<bool, ValueEquals_>(type, lhs, rhs);
}
//----------------------------------------------------------------------------
hash_t ValueHash(ValueType type, const MemoryView<const u8>& data) {
    return SwitchValue_<size_t, ValueHash_>(type, data);
}
//----------------------------------------------------------------------------
void ValueLerp(ValueType type, const MemoryView<u8>& dst, const MemoryView<const u8>& a, const MemoryView<const u8>& b, float t) {
    SwitchValue_<void, ValueLerp_>(type, dst, a, b, t);
}
//----------------------------------------------------------------------------
void ValueLerpArray(ValueType type,
                    const MemoryView<u8>& dst, size_t dstStride,
                    const MemoryView<const u8>& a,
                    const MemoryView<const u8>& b,
                    const MemoryView<const float>& ts ) {
    SwitchValue_<void, ValueLerpArray_>(type, dst, dstStride, a, b, ts);
}
//----------------------------------------------------------------------------
void ValueBarycentricLerp(  ValueType type, const MemoryView<u8>& dst,
                            const MemoryView<const u8>& a,
                            const MemoryView<const u8>& b,
                            const MemoryView<const u8>& c,
                            const float3& uvw ) {
    SwitchValue_<void, ValueBarycentricLerp_>(type, dst, a, b, c, uvw);
}
//----------------------------------------------------------------------------
void ValueBarycentricLerpArray( ValueType type,
                                const MemoryView<u8>& dst, size_t dstStride,
                                const MemoryView<const u8>& a,
                                const MemoryView<const u8>& b,
                                const MemoryView<const u8>& c,
                                const MemoryView<const float3>& uvws ) {
    SwitchValue_<void, ValueBarycentricLerpArray_>(type, dst, dstStride, a, b, c, uvws);
}
//----------------------------------------------------------------------------
bool ValueIsPromotable(ValueType dst, ValueType src) {
    return (src == dst || ValuePromote_<ValueDoNothing_>(dst, src));
}
//----------------------------------------------------------------------------
bool ValuePromote(ValueType output, const MemoryView<u8>& dst, ValueType input, const MemoryView<const u8>& src) {
    return (output == input)
        ? ValueCopy(output, dst, src), true
        : ValuePromote_<ValueCopy_>(output, input, dst, src);
}
//----------------------------------------------------------------------------
bool ValuePromoteArray( ValueType output, const MemoryView<u8>& dst, size_t dstStride,
                        ValueType input, const MemoryView<const u8>& src, size_t srcStride,
                        size_t count ) {
    Assert(0 != count);
    return (output == input)
        ? SwitchValue_<void, ValueCopyArray_>(output, dst, dstStride, src, srcStride, count), true
        : ValuePromote_<ValueCopyArray_>(output, input, dst, dstStride, src, srcStride, count);
}
//----------------------------------------------------------------------------
void ValueSwap(ValueType type, const MemoryView<u8>& lhs, const MemoryView<u8>& rhs) {
    SwitchValue_<void, ValueSwap_>(type, lhs, rhs);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core

#include "Core.RTTI/MetaAtom.h"
#include "Core.RTTI/MetaAtomVisitor.h"
#include "Core.RTTI/MetaTypeTraits.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <typename T>
struct ValueToMetaAtom_ {
    void operator ()(RTTI::PMetaAtom& dst, const Graphics::Value& src) const {
        dst = RTTI::MakeAtom(src.Get<T>());
    }
};
template <>
struct ValueToMetaAtom_<void> {
    void operator ()(RTTI::PMetaAtom& dst, const Graphics::Value& src) const {
        AssertNotReached();
        dst.reset();
    }
};
//----------------------------------------------------------------------------
class MetaAtomToValueVisitor_ : public IMetaAtomConstVisitor {
public:
    MetaAtomToValueVisitor_(Graphics::Value* pvalue) : _pvalue(pvalue) { Assert(_pvalue); }
    virtual ~MetaAtomToValueVisitor_() {}

    virtual void Visit(const IMetaAtomPair* ) override { AssertNotImplemented(); }
    virtual void Visit(const IMetaAtomVector* ) override { AssertNotImplemented(); }
    virtual void Visit(const IMetaAtomDictionary* ) override { AssertNotImplemented(); }

#define DEF_METATYPE_SCALAR(_Name, T, _TypeId, _Unused) \
    virtual void Visit(const MetaTypedAtom<T>* scalar) override { Visit_(scalar); }
    FOREACH_CORE_RTTI_NATIVE_TYPES(DEF_METATYPE_SCALAR)
#undef DEF_METATYPE_SCALAR

private:
    template <typename T>
    void Unwrap_(const MetaTypedAtom<T>* scalar, std::true_type) {
        *_pvalue = scalar->Wrapper();
    }

    template <typename T>
    void Unwrap_(const MetaTypedAtom<T>* scalar, std::false_type) {
        AssertNotImplemented();
    }

    template <typename T>
    void Visit_(const MetaTypedAtom<T>* scalar) {
        typedef typename std::integral_constant< bool,
            (Graphics::ValueTraits<T>::TypeId != Graphics::ValueType::Void) >::type
            boolean_type;

        Unwrap_(scalar, boolean_type());
    }

    Graphics::Value* _pvalue;
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void GraphicsValueToAtom(RTTI::PMetaAtom& dst, const Graphics::Value& src) {
    Assert(!src.empty());
    SwitchValue_<void, ValueToMetaAtom_>(src.Type(), dst, src);
}
//----------------------------------------------------------------------------
void AtomToGraphicsValue(Graphics::Value& dst, const RTTI::PMetaAtom& src) {
    Assert(src);
    dst.clear();
    MetaAtomToValueVisitor_ visitor(&dst);
    src->Accept(&visitor);
    AssertRelease(!dst.empty());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
