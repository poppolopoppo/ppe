#include "stdafx.h"

#include "RTTI/NativeTypes.h"

#include "RTTI/Any.h"
#include "RTTI/AtomVisitor.h" // needed for Accept() & PrettyPrint()
#include "RTTI/TypeInfos.h"

#include "MetaEnum.h" // cast from FName to enum
#include "MetaObject.h" // needed for PMetaObject manipulation
#include "MetaObjectHelpers.h" // needed for DeepEquals()

#include "IO/Dirpath.h"
#include "IO/Filename.h"
#include "IO/Format.h"
#include "IO/FormatHelpers.h"
#include "IO/String.h"
#include "IO/StringBuilder.h"
#include "IO/TextWriter.h"
#include "Maths/MathHelpers.h"
#include "Maths/PackedVectors.h"
#include "Maths/PackingHelpers.h"
#include "Maths/ScalarMatrix.h"
#include "Maths/ScalarMatrixHelpers.h"
#include "Maths/ScalarVector.h"
#include "Maths/ScalarVectorHelpers.h"
#include "Maths/Quaternion.h"
#include "Maths/Transform.h"
#include "Memory/MemoryView.h"
#include "Misc/Guid.h"
#include "Time/Timestamp.h"

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static bool IsAssignableObject_(const IScalarTraits& src, const ITypeTraits& dst, const PMetaObject& pobj) {
    if (src == dst)
        return true;

    Assert(src.TypeId() == FTypeId(ENativeType::MetaObject));
    if (dst.TypeId() != FTypeId(ENativeType::MetaObject))
        return false;

    const FMetaClass* dstClass = dst.ToScalar().ObjectClass();
    const FMetaClass* srcClass = nullptr;

    if (pobj) {
        srcClass = pobj->RTTI_Class();

#ifdef WITH_PPE_ASSERT
        Assert(srcClass);
        Assert(src.ObjectClass()->IsAssignableFrom(*srcClass));
#endif
    }
    else {
        srcClass = src.ObjectClass();
    }

    Assert(srcClass);
    Assert(dstClass);
    return (dstClass->IsAssignableFrom(*srcClass));
}
//----------------------------------------------------------------------------
template <typename T>
static Meta::TEnableIf< not std::is_integral_v<T>, bool > PromoteValue_(const T&, const FAtom&) NOEXCEPT {
    return false; // can't promote without explicit rules
}
//----------------------------------------------------------------------------
template <typename _From, typename _To>
static bool PromoteIntegral_(const _From src, _To* dst) NOEXCEPT {
    *dst = _To(src);
    return (_From(*dst) == src);
}
template <typename T>
static Meta::TEnableIf< std::is_integral_v<T>, bool > PromoteValue_(const T src, const FAtom& dst) NOEXCEPT {
    switch (ENativeType(dst.TypeId())) {

    case ENativeType::Int8:         return PromoteIntegral_(src, &dst.FlatData<i8 >());
    case ENativeType::Int16:        return PromoteIntegral_(src, &dst.FlatData<i16>());
    case ENativeType::Int32:        return PromoteIntegral_(src, &dst.FlatData<i32>());
    case ENativeType::Int64:        return PromoteIntegral_(src, &dst.FlatData<i64>());

    case ENativeType::UInt8:        return PromoteIntegral_(src, &dst.FlatData<u8 >());
    case ENativeType::UInt16:       return PromoteIntegral_(src, &dst.FlatData<u16>());
    case ENativeType::UInt32:       return PromoteIntegral_(src, &dst.FlatData<u32>());
    case ENativeType::UInt64:       return PromoteIntegral_(src, &dst.FlatData<u64>());

    case ENativeType::Float:        dst.FlatData<float>() = float(src); return true;
    case ENativeType::Double:       dst.FlatData<double>() = double(src); return true;

    case ENativeType::Bool:         dst.FlatData<bool>() = (src != T()); return true;

    default:
        return false;
    }
}
//----------------------------------------------------------------------------
static bool PromoteValue_(float f, const FAtom& dst) NOEXCEPT {
    switch (ENativeType(dst.TypeId())) {

    case ENativeType::UInt16:       dst.FlatData<u16>() = FP32_to_FP16(f); return true;
    case ENativeType::Double:       dst.FlatData<double>() = double(f); return true;

    default:
        return false;
    }
}
//----------------------------------------------------------------------------
static bool PromoteValue_(double d, const FAtom& dst) NOEXCEPT {
    switch (ENativeType(dst.TypeId())) {

    case ENativeType::Float:        dst.FlatData<float>() = float(d); return true;

    default:
        return false;
    }
}
//----------------------------------------------------------------------------
static bool PromoteValue_(const FString& str, const FAtom& dst) {
    switch (ENativeType(dst.TypeId())) {

    case ENativeType::BinaryData:   dst.FlatData<FBinaryData>().CopyFrom(str.MakeView().Cast<const u8>()); return true;
    case ENativeType::Dirpath:      dst.FlatData<FDirpath>() = ToWString(str); return true;
    case ENativeType::Filename:     dst.FlatData<FFilename>() = ToWString(str); return true;
    case ENativeType::Name:         dst.FlatData<FName>() = str; return true;;
    case ENativeType::WString:      dst.FlatData<FWString>() = ToWString(str); return true;

    default:
        if (dst.TypeFlags() ^ ETypeFlags::Enum) {
            const FMetaEnum* metaEnum = dst.Traits()->ToScalar().EnumClass();
            Assert(metaEnum);

            if (const FMetaEnumValue* v = metaEnum->NameToValueIFP(str.MakeView())) {
                metaEnum->SetValue(dst, *v);
                return true;
            }
        }

        return false;
    }
}
//----------------------------------------------------------------------------
static bool PromoteValue_(const FWString& wstr, const FAtom& dst) {
    switch (ENativeType(dst.TypeId())) {

    case ENativeType::Dirpath:      dst.FlatData<FDirpath>() = wstr; return true;
    case ENativeType::Filename:     dst.FlatData<FFilename>() = wstr; return true;
    case ENativeType::Name:         dst.FlatData<FName>() = ToString(wstr); return true;;
    case ENativeType::WString:      dst.FlatData<FWString>() = wstr; return true;

    default:
        return false;
    }
}
//----------------------------------------------------------------------------
static bool PromoteValue_(const FName& name, const FAtom& dst) {
    if (dst.TypeFlags() ^ ETypeFlags::Enum) {
        const FMetaEnum* metaEnum = dst.Traits()->ToScalar().EnumClass();
        Assert(metaEnum);

        if (const FMetaEnumValue* v = metaEnum->NameToValueIFP(name)) {
            metaEnum->SetValue(dst, *v);
            return true;
        }
    }
    return false;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class TNativeTypeTraits final : public TBaseTypeTraits<T, TBaseScalarTraits<T> > {
    using base_traits = TBaseTypeTraits<T, TBaseScalarTraits<T> >;
    using typename base_traits::value_type;
    using typename base_traits::pointer;
    using typename base_traits::const_pointer;

public:
    CONSTEXPR TNativeTypeTraits();

public: // IScalarTraits
    virtual const FMetaEnum* EnumClass() const override final {
        return nullptr;
    }

    virtual const FMetaClass* ObjectClass() const override final {
        return nullptr;
    }

public: // ITypeTraits
    virtual FStringView TypeName() const override final;

    virtual bool IsDefaultValue(const void* data) const override final {
        Assert(data);

        return (*reinterpret_cast<const_pointer>(data) == Meta::MakeForceInit<T>());
    }

    virtual void ResetToDefaultValue(void* data) const override final {
        Assert(data);

        *reinterpret_cast<pointer>(data) = Meta::MakeForceInit<T>();
    }

    virtual bool DeepEquals(const void* lhs, const void* rhs) const override final {
        return base_traits::Equals(lhs, rhs); // deep equals <=> equals for native types
    }

    virtual void DeepCopy(const void* src, void* dst) const override final {
        base_traits::Copy(src, dst); // deep copy <=> copy for native types
    }

    virtual bool PromoteCopy(const void* src, const FAtom& dst) const override final {
        Assert(src);
        Assert(dst);

        return (not base_traits::PromoteCopy(src, dst)
            ? PromoteValue_(*reinterpret_cast<const_pointer>(src), dst)
            : true );
    }

    virtual bool PromoteMove(void* src, const FAtom& dst) const override final {
        Assert(src);
        Assert(dst);

        return (not base_traits::PromoteMove(src, dst)
            ? PromoteValue_(*reinterpret_cast<const_pointer>(src), dst)
            : true );
    }

    virtual void* Cast(void* data, const PTypeTraits& dst) const override final {
        return base_traits::Cast(data, dst);
    }

    virtual bool Accept(IAtomVisitor* visitor, void* data) const override final {
        Assert(data);

        return visitor->Visit(
            static_cast<const IScalarTraits*>(this),
            *reinterpret_cast<pointer>(data) );
    }
};
//----------------------------------------------------------------------------
// FAny specializations
//----------------------------------------------------------------------------
template <>
bool TNativeTypeTraits<FAny>::PromoteCopy(const void* src, const FAtom& dst) const {
    Assert(src);
    Assert(dst);

    return (not base_traits::PromoteCopy(src, dst)
        ? static_cast<const FAny*>(src)->InnerAtom().PromoteCopy(dst)
        : true );
}
//----------------------------------------------------------------------------
template <>
bool TNativeTypeTraits<FAny>::PromoteMove(void* src, const FAtom& dst) const {
    Assert(src);
    Assert(dst);

    return (not base_traits::PromoteMove(src, dst)
        ? static_cast<FAny*>(src)->InnerAtom().PromoteMove(dst)
        : true );
}
//----------------------------------------------------------------------------
template <>
void* TNativeTypeTraits<FAny>::Cast(void* data, const PTypeTraits& dst) const {
    Assert(data);

    return (*dst != *this
        ? static_cast<FAny*>(data)->Traits()->Cast(
            static_cast<FAny*>(data)->InnerAtom().Data(), dst)
        : data );
}
//----------------------------------------------------------------------------
// PMetaObject specializations
//----------------------------------------------------------------------------
template <>
const FMetaClass* TNativeTypeTraits<PMetaObject>::ObjectClass() const {
    return RTTI::MetaClass<FMetaObject>();
}
//----------------------------------------------------------------------------
template <>
bool TNativeTypeTraits<PMetaObject>::DeepEquals(const void* lhs, const void* rhs) const {
    return DeepEqualsObject(
        *reinterpret_cast<const PMetaObject*>(lhs),
        *reinterpret_cast<const PMetaObject*>(rhs) );
}
//----------------------------------------------------------------------------
template <>
void TNativeTypeTraits<PMetaObject>::DeepCopy(const void* src, void* dst) const {
    DeepCopyObject(*this,
        *reinterpret_cast<const PMetaObject*>(src),
        *reinterpret_cast<PMetaObject*>(dst) );
}
//----------------------------------------------------------------------------
template <>
bool TNativeTypeTraits<PMetaObject>::PromoteCopy(const void* src, const FAtom& dst) const {
    Assert(src);
    Assert(dst);

    return (not base_traits::PromoteCopy(src, dst)
        ? PromoteCopyObject(*this, *reinterpret_cast<const PMetaObject*>(src), dst)
        : true );
}
//----------------------------------------------------------------------------
template <>
bool TNativeTypeTraits<PMetaObject>::PromoteMove(void* src, const FAtom& dst) const {
    Assert(src);
    Assert(dst);

    return (not base_traits::PromoteMove(src, dst)
        ? PromoteMoveObject(*this, *reinterpret_cast<PMetaObject*>(src), dst)
        : true );
}
//----------------------------------------------------------------------------
template <>
void* TNativeTypeTraits<PMetaObject>::Cast(void* data, const PTypeTraits& dst) const {
    return CastObject(*this, *reinterpret_cast<PMetaObject*>(data), dst);
}
//----------------------------------------------------------------------------
// Specialize Traits(), TypeId() & TypeInfos() for each native type
//----------------------------------------------------------------------------
#define DEF_RTTI_NATIVETYPE_TRAITS(_Name, T, _TypeId) \
    template <> \
    CONSTEXPR TNativeTypeTraits<T>::TNativeTypeTraits() \
        : base_traits( \
            FTypeId(ENativeType::_Name), \
            ETypeFlags::Scalar|ETypeFlags::Native|( \
                Meta::TIsPod<T>::value ? ETypeFlags::POD : ETypeFlags(0))|( \
                std::is_trivially_destructible_v<T> ? ETypeFlags::TriviallyDestructible : ETypeFlags(0)), \
            sizeof(T) ) \
    {} \
    \
    template <> \
    FStringView TNativeTypeTraits<T>::TypeName() const { \
        return STRINGIZE(_Name); \
    } \
    \
    /* Global helper for MakeTraits<T>() */ \
    PTypeTraits Traits(Meta::TType<T>) NOEXCEPT { \
        return PTypeTraits::Make< TNativeTypeTraits<T> >(); \
    }

FOREACH_RTTI_NATIVETYPES(DEF_RTTI_NATIVETYPE_TRAITS)

#undef DEF_RTTI_NATIVETYPE_TRAITS
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PTypeTraits MakeTraits(ENativeType nativeType) NOEXCEPT {
    switch (nativeType) {
#define DEF_RTTI_MAKETRAITS(_Name, T, _TypeId) \
    case ENativeType::_Name: return MakeTraits<T>();
    FOREACH_RTTI_NATIVETYPES(DEF_RTTI_MAKETRAITS)
#undef DEF_RTTI_MAKETRAITS
    default:
        AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Traits aliasing used to support packed/quantized data
// - Not very user friendly for manual edition,
// - But at least we can still load/save those types.
//----------------------------------------------------------------------------
#define DEF_RTTI_ALIASING_TRAITS(_FROM, _TO) \
    PTypeTraits Traits(Meta::TType<_FROM>) NOEXCEPT { \
        STATIC_ASSERT(sizeof(_FROM) == sizeof(_TO)); \
        return Traits(Meta::TType<_TO>{}); \
    }
//----------------------------------------------------------------------------
DEF_RTTI_ALIASING_TRAITS(byten,                 i8 )
DEF_RTTI_ALIASING_TRAITS(shortn,                i16)
DEF_RTTI_ALIASING_TRAITS(wordn,                 i32)
DEF_RTTI_ALIASING_TRAITS(ubyten,                u8 )
DEF_RTTI_ALIASING_TRAITS(ushortn,               u16)
DEF_RTTI_ALIASING_TRAITS(uwordn,                u32)
DEF_RTTI_ALIASING_TRAITS(FHalfFloat,            u16)
DEF_RTTI_ALIASING_TRAITS(UX10Y10Z10W2N,         u32)
DEF_RTTI_ALIASING_TRAITS(FGuid,                 COMMA_PROTECT(TTuple<u32, u32, u32, u32>))
DEF_RTTI_ALIASING_TRAITS(FTimestamp,            i64)
DEF_RTTI_ALIASING_TRAITS(u128,                  COMMA_PROTECT(TPair<u64, u64>))
//----------------------------------------------------------------------------
#undef DEF_RTTI_ALIASING_TRAITS
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool PromoteEnum(const IScalarTraits& traits, i64 src, const FAtom& dst) {
    if (ENativeType(dst.TypeId()) == ENativeType::Name) {
        const FMetaEnum* metaEnum = traits.EnumClass();
        Assert(metaEnum);

        if (const FMetaEnumValue* v = metaEnum->ValueToNameIFP(src)) {
            dst.TypedData<RTTI::FName>() = v->Name;
            return true;
        }

        return false;
    }
    else
    {
        return PromoteValue_(src, dst);
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool DeepEqualsObject(const PMetaObject& lhs, const PMetaObject& rhs) {
    if (lhs == rhs)
        return true;
    else if (!lhs || !rhs)
        return false;
    else
        return RTTI::DeepEquals(*lhs, *rhs);
}
//----------------------------------------------------------------------------
void DeepCopyObject(const IScalarTraits& , const PMetaObject& src, PMetaObject& dst) {
    if (src == dst) {
        return;
    }
    else if (nullptr == src) {
        dst = nullptr;
        return;
    }
    else if (nullptr == dst) {
        Verify(src->RTTI_Class()->CreateInstance(dst, false/* don't reset since we deep copy */));
    }

    RTTI::DeepCopy(*src, *dst);
}
//----------------------------------------------------------------------------
bool PromoteCopyObject(const IScalarTraits& self, const PMetaObject& src, const FAtom& dst) {
    Assert(dst);
    Assert(*dst.Traits() != self);

    if (ENativeType(dst.TypeId()) == ENativeType::MetaObject) {
        if (IsAssignableObject_(self, *dst.Traits(), src)) {
            (*reinterpret_cast<PMetaObject*>(dst.Data())) = src;
            return true;
        }
    }

    return false;
}
//----------------------------------------------------------------------------
bool PromoteMoveObject(const IScalarTraits& self, PMetaObject& src, const FAtom& dst) {
    Assert(dst);
    Assert(*dst.Traits() != self);

    if (ENativeType(dst.TypeId()) == ENativeType::MetaObject) {
        if (IsAssignableObject_(self, *dst.Traits(), src)) {
            (*reinterpret_cast<PMetaObject*>(dst.Data())) = std::move(src);
            return true;
        }
    }

    return false;
}
//----------------------------------------------------------------------------
void* CastObject(const IScalarTraits& self, PMetaObject& obj, const PTypeTraits& dst) {
    if (self == *dst) {
        return &obj;
    }
    else if (dst->TypeId() == FTypeId(ENativeType::MetaObject)) {
        if (IsAssignableObject_(self, *dst, obj))
            return &obj;
    }
    return nullptr;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
template <typename T, size_t _Dim>
CONSTEXPR PTypeTraits StaticArrayTraits_(Meta::TType<TScalarVector<T, _Dim>>) NOEXCEPT {
    STATIC_ASSERT(sizeof(TArray<T, _Dim>) == sizeof(TScalarVector<T, _Dim>));
    return PTypeTraits::Make< TStaticArrayTraits<T, _Dim> >();
}
template <typename T, size_t _Width, size_t _Height>
CONSTEXPR PTypeTraits StaticArrayTraits_(Meta::TType<TScalarMatrix<T, _Width, _Height>>) NOEXCEPT {
    STATIC_ASSERT(sizeof(TArray<T, _Width * _Height>) == sizeof(TScalarMatrix<T, _Width, _Height>));
    return PTypeTraits::Make< TStaticArrayTraits<T, _Width * _Height> >();
}
} //!namespace
//----------------------------------------------------------------------------
PTypeTraits Traits(Meta::TType<byte2> t)    NOEXCEPT { return StaticArrayTraits_(t); }
PTypeTraits Traits(Meta::TType<byte4> t)    NOEXCEPT { return StaticArrayTraits_(t); }
PTypeTraits Traits(Meta::TType<ubyte2> t)   NOEXCEPT { return StaticArrayTraits_(t); }
PTypeTraits Traits(Meta::TType<ubyte4> t)   NOEXCEPT { return StaticArrayTraits_(t); }
PTypeTraits Traits(Meta::TType<short2> t)   NOEXCEPT { return StaticArrayTraits_(t); }
PTypeTraits Traits(Meta::TType<short4> t)   NOEXCEPT { return StaticArrayTraits_(t); }
PTypeTraits Traits(Meta::TType<ushort2> t)  NOEXCEPT { return StaticArrayTraits_(t); }
PTypeTraits Traits(Meta::TType<ushort4> t)  NOEXCEPT { return StaticArrayTraits_(t); }
PTypeTraits Traits(Meta::TType<word2> t)    NOEXCEPT { return StaticArrayTraits_(t); }
PTypeTraits Traits(Meta::TType<word3> t)    NOEXCEPT { return StaticArrayTraits_(t); }
PTypeTraits Traits(Meta::TType<word4> t)    NOEXCEPT { return StaticArrayTraits_(t); }
PTypeTraits Traits(Meta::TType<uword2> t)   NOEXCEPT { return StaticArrayTraits_(t); }
PTypeTraits Traits(Meta::TType<uword3> t)   NOEXCEPT { return StaticArrayTraits_(t); }
#ifdef _MSC_VER // workaround a weird compiler bug, #TODO check after a few updates if this is still needed
PTypeTraits Traits(Meta::TType<uword4>)     NOEXCEPT { return PTypeTraits::Make< TStaticArrayTraits<uword, 4> >(); }
#else
PTypeTraits Traits(Meta::TType<uword4> t)   NOEXCEPT { return StaticArrayTraits_(t); }
#endif
PTypeTraits Traits(Meta::TType<float2> t)   NOEXCEPT { return StaticArrayTraits_(t); }
PTypeTraits Traits(Meta::TType<float3> t)   NOEXCEPT { return StaticArrayTraits_(t); }
PTypeTraits Traits(Meta::TType<float4> t)   NOEXCEPT { return StaticArrayTraits_(t); }
PTypeTraits Traits(Meta::TType<float2x2> t) NOEXCEPT { return StaticArrayTraits_(t); }
PTypeTraits Traits(Meta::TType<float3x3> t) NOEXCEPT { return StaticArrayTraits_(t); }
PTypeTraits Traits(Meta::TType<float4x3> t) NOEXCEPT { return StaticArrayTraits_(t); }
PTypeTraits Traits(Meta::TType<float4x4> t) NOEXCEPT { return StaticArrayTraits_(t); }
//----------------------------------------------------------------------------
PTypeTraits Traits(Meta::TType<FQuaternion>) NOEXCEPT {
    STATIC_ASSERT(sizeof(FQuaternion) == sizeof(TArray<float, 4>));
    return PTypeTraits::Make< TStaticArrayTraits<float, 4> >();
}
//----------------------------------------------------------------------------
PTypeTraits Traits(Meta::TType<FTransform>) NOEXCEPT {
    using FTransformAsTuple = TTuple<float4, float3, float3>;
    STATIC_ASSERT(sizeof(FTransform) == sizeof(FTransformAsTuple));
    return Traits(Meta::TType<FTransformAsTuple>{});
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Used to create a tuple with arity only known at runtime (used by parsers)
//----------------------------------------------------------------------------
namespace {
template <size_t _Index>
using TAnyTupleId_ = FAny;
template <size_t... _Indices>
static PTypeTraits MakeAnyTuple_(std::index_sequence<_Indices...>) {
    return MakeTraits< TTuple<TAnyTupleId_<_Indices>...> >();
}
} //!namespace
PTypeTraits MakeAnyTuple(size_t arity) {
    STATIC_ASSERT(MaxArityForTieAsTuple == 8);
    Assert(arity > 1); // tuple with arity < 2 aren't supported, should at least be a pair
    switch (arity) {
#define DEF_RTTI_MAKEANYTUPLE(N) case N: return MakeAnyTuple_(std::make_index_sequence<N>{})
    DEF_RTTI_MAKEANYTUPLE(2);
    DEF_RTTI_MAKEANYTUPLE(3);
    DEF_RTTI_MAKEANYTUPLE(4);
    DEF_RTTI_MAKEANYTUPLE(5);
    DEF_RTTI_MAKEANYTUPLE(6);
    DEF_RTTI_MAKEANYTUPLE(7);
    DEF_RTTI_MAKEANYTUPLE(8);
#undef DEF_RTTI_MAKEANYTUPLE
    default:
        // If you really really want support for higher dimensions,
        // you might want to rise MaxArityForTieAsTuple and implement missing traits ...
        AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define DECL_RTTI_NATIVETYPE_ISSUPPORTED(_Name, T, _TypeId) STATIC_ASSERT(TIsSupportedType<T>::value);
FOREACH_RTTI_NATIVETYPES(DECL_RTTI_NATIVETYPE_ISSUPPORTED)
#undef DECL_RTTI_NATIVETYPE_ISSUPPORTED
//----------------------------------------------------------------------------
STATIC_ASSERT(not TIsSupportedType<void>::value);
STATIC_ASSERT(not TIsSupportedType<FAtom>::value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FString MakeTupleTypeName(const TMemoryView<const PTypeTraits>& elements) {
    PPE_LEAKDETECTOR_WHITELIST_SCOPE();

    FStringBuilder oss;
    oss << "TTuple<";

    auto sep = Fmt::NotFirstTime(", ");
    for (const auto& elt : elements)
        oss << sep << elt->TypeInfos().Name();

    oss << '>';

    return oss.ToString();
}
//----------------------------------------------------------------------------
FString MakeListTypeName(const PTypeTraits& value) {
    PPE_LEAKDETECTOR_WHITELIST_SCOPE();
    return StringFormat("TList<{0}>", value->TypeInfos().Name());
}
//----------------------------------------------------------------------------
FString MakeDicoTypeName(const PTypeTraits& key, const PTypeTraits& value) {
    PPE_LEAKDETECTOR_WHITELIST_SCOPE();
    return StringFormat("TDico<{0}, {1}>", key->TypeInfos().Name(), value->TypeInfos().Name());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE
