// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "RTTI/NativeTypes.h"

#include "RTTI/Any.h"
#include "RTTI/AtomVisitor.h" // needed for Accept() & PrettyPrint()
#include "RTTI/TypeInfos.h"
#include "RTTI/Typedefs.h"

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

// static type names:
#include "Allocator/SlabHeap.h"
#include "HAL/PlatformMemory.h"
#include "Meta/Singleton.h"
#include <mutex>

#include "MetaDatabase.h"
#include "Thread/CriticalSection.h"

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static bool IsAssignableObject_(const IScalarTraits& src, const ITypeTraits& dst, const PMetaObject& pobj) NOEXCEPT {
    if (src == dst)
        return true;

    Assert(src.TypeId() == FTypeId(ENativeType::MetaObject));
    if (dst.TypeId() != FTypeId(ENativeType::MetaObject))
        return false;

    const FMetaClass* dstClass = dst.ToScalar().ObjectClass();
    const FMetaClass* srcClass = nullptr;

    if (pobj) {
        srcClass = pobj->RTTI_Class();

#if USE_PPE_ASSERT
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

    case ENativeType::BinaryData:   return str.ConvertTo(&dst.FlatData<FBinaryData>());
    case ENativeType::Dirpath:      return str.ConvertTo(&dst.FlatData<FDirpath>());
    case ENativeType::Filename:     return str.ConvertTo(&dst.FlatData<FFilename>());
    case ENativeType::Name:         return str.ConvertTo(&dst.FlatData<FName>());
    case ENativeType::WString:      dst.FlatData<FWString>() = ToWString(str); return true;

    default:
        if (is_enum_v(dst.TypeFlags()))
            return dst.Traits()->ToScalar().FromString(dst.Data(), str.Converter());

        return false;
    }
}
//----------------------------------------------------------------------------
static bool PromoteValue_(const FWString& wstr, const FAtom& dst) {
    switch (ENativeType(dst.TypeId())) {

    case ENativeType::BinaryData:   return wstr.ConvertTo(&dst.FlatData<FBinaryData>());
    case ENativeType::Dirpath:      return wstr.ConvertTo(&dst.FlatData<FDirpath>());
    case ENativeType::Filename:     return wstr.ConvertTo(&dst.FlatData<FFilename>());
    case ENativeType::WString:      dst.FlatData<FWString>() = wstr; return true;

    default:
        if (is_enum_v(dst.TypeFlags()))
            return dst.Traits()->ToScalar().FromString(dst.Data(),
                FStringConversion{ WCHAR_TO_UTF_8(wstr) });

        return false;
    }
}
//----------------------------------------------------------------------------
static bool PromoteValue_(const FName& name, const FAtom& dst) {
    if (is_enum_v(dst.TypeFlags())) {
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
bool PromoteEnum(const IScalarTraits& traits, FMetaEnumOrd src, const FAtom& dst) {
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
void* CastObject(const IScalarTraits& self, PMetaObject& obj, const PTypeTraits& dst) NOEXCEPT {
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
    else if (nullptr == src.get()) {
        dst = nullptr;
        return;
    }
    else if (nullptr == dst.get()) {
        Verify(src->RTTI_Class()->CreateInstance(dst, false/* don't reset since we deep copy */));
    }

    RTTI::DeepCopy(*src, *dst);
}
//----------------------------------------------------------------------------
bool ObjectFromString(const IScalarTraits& self, PMetaObject* dst, const FStringConversion& iss) NOEXCEPT {
    Assert(dst);

    FLazyPathName pathName;
    if (iss >> &pathName) {
        FMetaDatabaseReadable db;
        *dst = db->ObjectIFP(pathName);
        if (!!*dst) {
            return ((*dst)->RTTI_Traits().PTraits == &self);
        }
    }

    return false;
}
//----------------------------------------------------------------------------
bool PromoteCopyObject(const IScalarTraits& self, const PMetaObject& src, const FAtom& dst) {
    Assert(dst);
    Assert(*dst.Traits() != self);

    if (ENativeType(dst.TypeId()) == ENativeType::MetaObject) {
        if (IsAssignableObject_(self, *dst.Traits(), src)) {
            (*static_cast<PMetaObject*>(dst.Data())) = src;
            return true;
        }
    }

    return false;
}
//----------------------------------------------------------------------------
bool PromoteMoveObject(const IScalarTraits& self, PMetaObject& src, const FAtom& dst) NOEXCEPT {
    Assert(dst);
    Assert(*dst.Traits() != self);

    if (ENativeType(dst.TypeId()) == ENativeType::MetaObject) {
        if (IsAssignableObject_(self, *dst.Traits(), src)) {
            (*static_cast<PMetaObject*>(dst.Data())) = std::move(src);
            return true;
        }
    }

    return false;
}
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
    using base_traits::base_traits;

public: // IScalarTraits
    virtual const FMetaEnum* EnumClass() const NOEXCEPT override final {
        return nullptr;
    }

    virtual const FMetaClass* ObjectClass() const NOEXCEPT override final {
        return nullptr;
    }

public: // ITypeTraits
    virtual FStringView TypeName() const override final;

    virtual bool IsDefaultValue(const void* data) const NOEXCEPT override final {
        Assert(data);

        return (*static_cast<const_pointer>(data) == Meta::MakeForceInit<T>());
    }

    virtual void ResetToDefaultValue(void* data) const override final {
        Assert(data);

        *static_cast<pointer>(data) = Meta::MakeForceInit<T>();
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

        return (not base_traits::BasePromoteCopy(src, dst)
            ? PromoteValue_(*static_cast<const_pointer>(src), dst)
            : true );
    }

    virtual bool PromoteMove(void* src, const FAtom& dst) const NOEXCEPT override final {
        Assert(src);
        Assert(dst);

        return (not base_traits::BasePromoteMove(src, dst)
            ? PromoteValue_(*static_cast<const_pointer>(src), dst)
            : true );
    }

    virtual void* Cast(void* data, const PTypeTraits& dst) const override final {
        return base_traits::BaseCast(data, dst);
    }

    virtual bool Accept(IAtomVisitor* visitor, void* data) const override final {
        Assert(data);

        return visitor->Visit(
            static_cast<const IScalarTraits*>(this),
            *static_cast<pointer>(data) );
    }
};
//----------------------------------------------------------------------------
// FAny specializations
//----------------------------------------------------------------------------
template <>
bool TNativeTypeTraits<FAny>::PromoteCopy(const void* src, const FAtom& dst) const {
    Assert(src);
    Assert(dst);

    if (BasePromoteCopy(src, dst))
        return true;

    const FAny& any = *static_cast<const FAny*>(src);
    return (any.Valid() && any.PromoteCopy(dst));
}
//----------------------------------------------------------------------------
template <>
bool TNativeTypeTraits<FAny>::PromoteMove(void* src, const FAtom& dst) const NOEXCEPT {
    Assert(src);
    Assert(dst);

    if (BasePromoteMove(src, dst))
        return true;

    FAny& any = *static_cast<FAny*>(src);
    return (any.Valid() && any.PromoteMove(dst));
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
const FMetaClass* TNativeTypeTraits<PMetaObject>::ObjectClass() const NOEXCEPT {
    return RTTI::MetaClass<FMetaObject>();
}
//----------------------------------------------------------------------------
template <>
bool TNativeTypeTraits<PMetaObject>::DeepEquals(const void* lhs, const void* rhs) const {
    return DeepEqualsObject(
        *static_cast<const PMetaObject*>(lhs),
        *static_cast<const PMetaObject*>(rhs) );
}
//----------------------------------------------------------------------------
template <>
void TNativeTypeTraits<PMetaObject>::DeepCopy(const void* src, void* dst) const {
    DeepCopyObject(*this,
        *static_cast<const PMetaObject*>(src),
        *static_cast<PMetaObject*>(dst) );
}
//----------------------------------------------------------------------------
template <>
bool TNativeTypeTraits<PMetaObject>::PromoteCopy(const void* src, const FAtom& dst) const {
    Assert(src);
    Assert(dst);

    return (not BasePromoteCopy(src, dst)
        ? PromoteCopyObject(*this, *static_cast<const PMetaObject*>(src), dst)
        : true );
}
//----------------------------------------------------------------------------
template <>
bool TNativeTypeTraits<PMetaObject>::PromoteMove(void* src, const FAtom& dst) const NOEXCEPT {
    Assert(src);
    Assert(dst);

    return (not BasePromoteMove(src, dst)
        ? PromoteMoveObject(*this, *static_cast<PMetaObject*>(src), dst)
        : true );
}
//----------------------------------------------------------------------------
template <>
void* TNativeTypeTraits<PMetaObject>::Cast(void* data, const PTypeTraits& dst) const {
    return CastObject(*this, *static_cast<PMetaObject*>(data), dst);
}
//----------------------------------------------------------------------------
// Specialize Traits(), TypeId() & TypeInfos() for each native type
//----------------------------------------------------------------------------
#define DEF_RTTI_NATIVETYPE_TRAITS(_Name, T, _TypeId) \
    template <> \
    FStringView TNativeTypeTraits< T >::TypeName() const { \
        return STRINGIZE(_Name); \
    } \
    /* Global helper for MakeTraits<T>() */ \
    PTypeTraits RTTI_Traits(TTypeTag< T >) NOEXCEPT { \
        return MakeStaticType< TNativeTypeTraits<T>, T >(); \
    }

FOREACH_RTTI_NATIVETYPES(DEF_RTTI_NATIVETYPE_TRAITS)

#undef DEF_RTTI_NATIVETYPE_TRAITS
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Use to create a tuple with arity only known at runtime (used by parsers)
//----------------------------------------------------------------------------
PTypeTraits MakeAnyTuple(size_t arity) NOEXCEPT {
    STATIC_ASSERT(MaxArityForTieAsTuple == 30);
    Assert(arity > 1 && arity <= 30); // tuple with arity < 2 aren't supported, should at least be a pair
    using a = FAny;
    switch (arity) {
#define DEF_RTTI_MAKEANYTUPLE(N, ...) case N: return RTTI_Traits(TypeTag< TTuple<__VA_ARGS__> >)
    DEF_RTTI_MAKEANYTUPLE( 2, a, a);
    DEF_RTTI_MAKEANYTUPLE( 3, a, a, a);
    DEF_RTTI_MAKEANYTUPLE( 4, a, a, a, a);
    DEF_RTTI_MAKEANYTUPLE( 5, a, a, a, a, a);
    DEF_RTTI_MAKEANYTUPLE( 6, a, a, a, a, a, a);
    DEF_RTTI_MAKEANYTUPLE( 7, a, a, a, a, a, a, a);
    DEF_RTTI_MAKEANYTUPLE( 8, a, a, a, a, a, a, a, a);
    DEF_RTTI_MAKEANYTUPLE( 9, a, a, a, a, a, a, a, a, a);
    DEF_RTTI_MAKEANYTUPLE(10, a, a, a, a, a, a, a, a, a, a);
    DEF_RTTI_MAKEANYTUPLE(11, a, a, a, a, a, a, a, a, a, a, a);
    DEF_RTTI_MAKEANYTUPLE(12, a, a, a, a, a, a, a, a, a, a, a, a);
    DEF_RTTI_MAKEANYTUPLE(13, a, a, a, a, a, a, a, a, a, a, a, a, a);
    DEF_RTTI_MAKEANYTUPLE(14, a, a, a, a, a, a, a, a, a, a, a, a, a, a);
    DEF_RTTI_MAKEANYTUPLE(15, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a);
    DEF_RTTI_MAKEANYTUPLE(16, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a);
    DEF_RTTI_MAKEANYTUPLE(17, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a);
    DEF_RTTI_MAKEANYTUPLE(18, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a);
    DEF_RTTI_MAKEANYTUPLE(19, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a);
    DEF_RTTI_MAKEANYTUPLE(20, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a);
    DEF_RTTI_MAKEANYTUPLE(21, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a);
    DEF_RTTI_MAKEANYTUPLE(22, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a);
    DEF_RTTI_MAKEANYTUPLE(23, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a);
    DEF_RTTI_MAKEANYTUPLE(24, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a);
    DEF_RTTI_MAKEANYTUPLE(25, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a);
    DEF_RTTI_MAKEANYTUPLE(26, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a);
    DEF_RTTI_MAKEANYTUPLE(27, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a);
    DEF_RTTI_MAKEANYTUPLE(28, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a);
    DEF_RTTI_MAKEANYTUPLE(29, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a);
    DEF_RTTI_MAKEANYTUPLE(30, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a);
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
#define DECL_RTTI_NATIVETYPE_ISSUPPORTED(_Name, T, _TypeId) STATIC_ASSERT(has_support_for_v<T>);
FOREACH_RTTI_NATIVETYPES(DECL_RTTI_NATIVETYPE_ISSUPPORTED)
#undef DECL_RTTI_NATIVETYPE_ISSUPPORTED
//----------------------------------------------------------------------------
STATIC_ASSERT(not has_support_for_v<void>);
STATIC_ASSERT(not has_support_for_v<FAtom>);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
class FTypeNamesBuilder_ : public Meta::TStaticSingleton<FTypeNamesBuilder_> {
    using singleton_type = Meta::TStaticSingleton<FTypeNamesBuilder_>;

    FCriticalSection _barrierCS;
    SLABHEAP(TypeNames) _heap;

public:
    using singleton_type::Get;

    static void Create() {
        singleton_type::Create();
    }

    static void Destroy() {
        Get()._heap.ReleaseAll();
        singleton_type::Destroy();
    }

    STATIC_CONST_INTEGRAL(size_t, DefaultReserve, 128);

    class FWritePort : FCriticalScope {
    public:
        explicit FWritePort(FTypeNamesBuilder_& builder)
        :   FCriticalScope(&builder._barrierCS)
        ,   _heap(builder._heap) {
            _outp.Relocate(_heap.AllocateT<u8>(DefaultReserve));
        }

        ~FWritePort() {
            const FRawMemory oldM = _outp.Storage();
            const FRawMemory newM = _heap.ReallocateT(oldM, _outp.size()); // shrink to fit
            Assert_NoAssume(oldM.data() == newM.data()); // should realloc inplace
            Unused(newM);
        }

        void Append(const FStringView& str) {
            const size_t oldSize = _outp.Written().SizeInBytes();
            const size_t newSize = oldSize + str.size();

            if (newSize > _outp.Storage().SizeInBytes()) {
                const size_t newCapacity = SLABHEAP(TypeNames)::SnapSize(Max(_outp.Storage().SizeInBytes() * 2, newSize));
                const FRawMemory oldStorage = _outp.Storage();
                _outp.Relocate(_heap.ReallocateT(oldStorage, newCapacity));
                Assert_NoAssume(_outp.Storage().data() == oldStorage.data()); // should realloc inplace
            }

            _outp.WriteView(str);
        }

        FStringView Written() const {
            return _outp.Written().Cast<const char>();
        }

    private:
        SLABHEAP(TypeNames)& _heap;
        FMemoryViewWriter _outp;
    };

};
} //!namespace
//----------------------------------------------------------------------------
void TypeNamesStart() {
    FTypeNamesBuilder_::Create();
}
//----------------------------------------------------------------------------
void TypeNamesShutdown() {
    FTypeNamesBuilder_::Destroy();
}
//----------------------------------------------------------------------------
FStringView MakeTupleTypeName(const TMemoryView<const PTypeTraits>& elements) {
    STACKLOCAL_POD_ARRAY(FStringView, elementNames, elements.size());
    forrange(i, 0, elements.size()) // fetch outside of the lock
        elementNames[i] = elements[i]->TypeName(); // <- can recurse in this function

    FTypeNamesBuilder_::FWritePort sb{ FTypeNamesBuilder_::Get() };
    sb.Append("TTuple<");
    bool first = true;
    for (const FStringView& name : elementNames) {
        if (!first) sb.Append(", ");
        sb.Append(name);
        first = false;
    }
    sb.Append(">");
    return sb.Written();
}
//----------------------------------------------------------------------------
FStringView MakeListTypeName(const PTypeTraits& value) {
    const FStringView valueName = value->TypeName(); // <- can recurse in this function
    FTypeNamesBuilder_::FWritePort sb{ FTypeNamesBuilder_::Get() };
    sb.Append("TList<");
    sb.Append(valueName);
    sb.Append(">");
    return sb.Written();
}
//----------------------------------------------------------------------------
FStringView MakeDicoTypeName(const PTypeTraits& key, const PTypeTraits& value) {
    const FStringView keyName = key->TypeName(); // outside of the lock
    const FStringView valueName = value->TypeName(); // <- can recurse in this function
    FTypeNamesBuilder_::FWritePort sb{ FTypeNamesBuilder_::Get() };
    sb.Append("TDico<");
    sb.Append(keyName);
    sb.Append(", ");
    sb.Append(valueName);
    sb.Append(">");
    return sb.Written();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Used for manipulating visitors and atoms with fwd decl only :
//----------------------------------------------------------------------------
bool AtomVisit(IAtomVisitor& visitor, const ITupleTraits* tuple, void* data) {
    return visitor.Visit(tuple, data);
}
//----------------------------------------------------------------------------
bool AtomVisit(IAtomVisitor& visitor, const IListTraits* list, void* data) {
    return visitor.Visit(list, data);
}
//----------------------------------------------------------------------------
bool AtomVisit(IAtomVisitor& visitor, const IDicoTraits* dico, void* data) {
    return visitor.Visit(dico, data);
}
//----------------------------------------------------------------------------
#define DEF_ATOMVISIT_SCALAR(_Name, T, _TypeId) \
    bool AtomVisit(IAtomVisitor& visitor, const IScalarTraits* scalar, T& value) { \
        return visitor.Visit(scalar, value); \
    }
FOREACH_RTTI_NATIVETYPES(DEF_ATOMVISIT_SCALAR)
#undef DEF_ATOMVISIT_SCALAR
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE
