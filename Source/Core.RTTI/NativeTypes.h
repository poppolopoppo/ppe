#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core.RTTI/Atom.h"
#include "Core.RTTI/AtomHelpers.h"
#include "Core.RTTI/TypeTraits.h"
#include "Core.RTTI/Typedefs.h"

#include "Core.RTTI/NativeTypes.Definitions-inl.h"

#include "Core/Container/AssociativeVector.h"
#include "Core/Container/HashMap.h"
#include "Core/Container/Vector.h"
#include "Core/IO/TextWriter_fwd.h"
#include "Core/Memory/SegregatedMemoryPool.h" // Allocate()/Deallocate()

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define DECL_RTTI_NATIVETYPE_TRAITS(_Name, T, _TypeId) \
    CORE_RTTI_API PTypeTraits Traits(Meta::TType<T>);
FOREACH_RTTI_NATIVETYPES(DECL_RTTI_NATIVETYPE_TRAITS)
#undef DECL_RTTI_NATIVETYPE_TRAITS
//----------------------------------------------------------------------------
enum class ENativeType : FTypeId {
    Invalid = 0,
#define DECL_RTTI_NATIVETYPE_ENUM(_Name, T, _TypeId) _Name = _TypeId,
FOREACH_RTTI_NATIVETYPES(DECL_RTTI_NATIVETYPE_ENUM)
#undef DECL_RTTI_NATIVETYPE_ENUM
    __Count
}; //!enum class ENativeType
//----------------------------------------------------------------------------
PTypeTraits MakeTraits(ENativeType nativeType);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Parent>
class TBaseTypeTraits : public _Parent {
protected:
    typedef T value_type;
    typedef Meta::TAddPointer<T> pointer;
    typedef Meta::TAddPointer<const T> const_pointer;

    using memorypool_type = TTypedSegregatedMemoryPool<POOL_TAG(RTTI), T, false>;

public: // ITypeTraits
    virtual void* Allocate() const override final;
    virtual void Deallocate(void* ptr) const override final;

    virtual void Create(void* data) const override final;
    virtual void CreateCopy(void* data, const void* other) const override final;
    virtual void CreateMove(void* data, void* rvalue) const override final;
    virtual void Destroy(void* data) const override final;

    //virtual FTypeId TypeId() const override final;
    //virtual ETypeFlags TypeFlags() const override final;
    //virtual FTypeInfos TypeInfos() const override final;
    virtual size_t SizeInBytes() const override final { return sizeof(T); }

    //virtual bool IsDefaultValue(const void* data) const override final;
    //virtual void ResetToDefaultValue(void* data) const override final;

    virtual bool Equals(const void* lhs, const void* rhs) const override final;
    virtual void Copy(const void* src, void* dst) const override final;
    virtual void Move(void* src, void* dst) const override final;

    virtual void Swap(void* lhs, void* rhs) const override final;

    //virtual bool DeepEquals(const void* lhs, const void* rhs) const override final;
    //virtual void DeepCopy(const void* src, void* dst) const override final;

    //virtual bool PromoteCopy(const void* src, const FAtom& dst) const override final;
    //virtual bool PromoteMove(void* src, const FAtom& dst) const override final;

    virtual void* Cast(void* data, const PTypeTraits& dst) const override;

    virtual hash_t HashValue(const void* data) const override final;
};
//----------------------------------------------------------------------------
template <typename T>
class TBaseScalarTraits : public IScalarTraits {
public: // ITypeTraits
};
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
class TBasePairTraits : public IPairTraits {
public: // ITypeTraits
    virtual FTypeId TypeId() const override final;
    virtual ETypeFlags TypeFlags() const override final;
    virtual FTypeInfos TypeInfos() const override final;

    virtual bool IsDefaultValue(const void* data) const override final;
    virtual void ResetToDefaultValue(void* data) const override final;

    virtual bool DeepEquals(const void* lhs, const void* dst) const override final;
    virtual void DeepCopy(const void* src, void* dst) const override final;

    virtual bool PromoteCopy(const void* src, const FAtom& dst) const override final;
    virtual bool PromoteMove(void* src, const FAtom& dst) const override final;

public: // IPairTraits
    virtual PTypeTraits FirstTraits() const override final { return MakeTraits<_First>(); }
    virtual PTypeTraits SecondTraits() const override final { return MakeTraits<_Second>(); }
};
//----------------------------------------------------------------------------
template <typename T>
class TBaseListTraits : public IListTraits {
public: // ITypeTraits
    virtual FTypeId TypeId() const override final;
    virtual ETypeFlags TypeFlags() const override final;
    virtual FTypeInfos TypeInfos() const override final;

    virtual bool IsDefaultValue(const void* data) const override final;
    virtual void ResetToDefaultValue(void* data) const override final;

    virtual bool DeepEquals(const void* lhs, const void* rhs) const override final;
    virtual void DeepCopy(const void* src, void* dst) const override final;

    virtual bool PromoteCopy(const void* src, const FAtom& dst) const override final;
    virtual bool PromoteMove(void* src, const FAtom& dst) const override final;

public: // IListTraits
    virtual PTypeTraits ValueTraits() const override final { return MakeTraits<T>(); }
};
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
class TBaseDicoTraits : public IDicoTraits {
public: // ITypeTraits
    virtual FTypeId TypeId() const override final;
    virtual ETypeFlags TypeFlags() const override final;
    virtual FTypeInfos TypeInfos() const override final;

    virtual bool IsDefaultValue(const void* data) const override final;
    virtual void ResetToDefaultValue(void* data) const override final;

    virtual bool DeepEquals(const void* lhs, const void* rhs) const override final;
    virtual void DeepCopy(const void* src, void* dst) const override final;

    virtual bool PromoteCopy(const void* src, const FAtom& dst) const override final;
    virtual bool PromoteMove(void* src, const FAtom& dst) const override final;

public: // IDicoTraits
    virtual PTypeTraits KeyTraits() const override final { return MakeTraits<_Key>(); }
    virtual PTypeTraits ValueTraits() const override final { return MakeTraits<_Value>(); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Add strong typing support for TRefPtr<T> where T inherits from FMetaObject
class CORE_RTTI_API FBaseObjectTraits : public TBaseTypeTraits<PMetaObject, TBaseScalarTraits<PMetaObject>> {
public: // ITypeTraits
    virtual FTypeId TypeId() const override final;
    virtual ETypeFlags TypeFlags() const override final;
    virtual FTypeInfos TypeInfos() const override final;

    virtual bool IsDefaultValue(const void* data) const override final;
    virtual void ResetToDefaultValue(void* data) const override final;

    virtual bool DeepEquals(const void* lhs, const void* rhs) const override final;
    virtual void DeepCopy(const void* src, void* dst) const override final;

    virtual bool PromoteCopy(const void* src, const FAtom& dst) const override final;
    virtual bool PromoteMove(void* src, const FAtom& dst) const override final;

    virtual void* Cast(void* data, const PTypeTraits& dst) const override final;

    virtual bool Accept(IAtomVisitor* visitor, void* data) const override final;

public: // FBaseObjectTraits
    virtual const FMetaClass* MetaClass() const = 0;
};
//----------------------------------------------------------------------------
template <typename T>
class TObjectTraits : public FBaseObjectTraits {
public: // FBaseObjectTraits
    virtual const FMetaClass* MetaClass() const override final {
        return RTTI::MetaClass<T>();
    }
};
//----------------------------------------------------------------------------
template <typename _Class>
PTypeTraits Traits(Meta::TType< TRefPtr<_Class> >, Meta::TEnableIf< std::is_base_of_v<FMetaObject, _Class> >* = nullptr) {
    return PTypeTraits::Make< TObjectTraits<Meta::TRemoveConst<_Class>> >();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
class TPairTraits : public TBaseTypeTraits< TPair<_First, _Second>, TBasePairTraits<_First, _Second> > {
    using base_traits = TBaseTypeTraits< TPair<_First, _Second>, TBasePairTraits<_First, _Second> >;
    using typename base_traits::value_type;
    using typename base_traits::pointer;
    using typename base_traits::const_pointer;

public: // IPairTraits
    virtual FAtom First(void* data) const override final;
    virtual FAtom Second(void* data) const override final;

    virtual void SetFirstCopy(void* data, const FAtom& other) const override final;
    virtual void SetFirstMove(void* data, const FAtom& rvalue) const override final;

    virtual void SetSecondCopy(void* data, const FAtom& other) const override final;
    virtual void SetSecondMove(void* data, const FAtom& rvalue) const override final;
};
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
PTypeTraits Traits(Meta::TType< TPair<_First, _Second> >) {
    return PTypeTraits::Make< TPairTraits<_First, _Second> >();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _VectorLike>
class TVectorLikeTraits : public TBaseTypeTraits< _VectorLike, TBaseListTraits<typename _VectorLike::value_type> > {
    using item_type = typename _VectorLike::value_type;
    using base_traits = TBaseTypeTraits< _VectorLike, TBaseListTraits<item_type> >;
    using typename base_traits::value_type;
    using typename base_traits::pointer;
    using typename base_traits::const_pointer;

public: // IListTraits
    using typename base_traits::foreach_fun;

    virtual size_t Count(const void* data) const override final;
    virtual bool IsEmpty(const void* data) const override final;

    virtual FAtom At(void* data, size_t index) const override final;
    virtual size_t Find(const void* data, const FAtom& item) const override final;

    virtual FAtom AddDefault(void* data) const override final;
    virtual void AddCopy(void* data, const FAtom& item) const override final;
    virtual void AddMove(void* data, const FAtom& item) const override final;
    virtual void Erase(void* data, size_t index) const override final;
    virtual bool Remove(void* data, const FAtom& item) const override final;

    virtual void Reserve(void* data, size_t capacity) const override final;
    virtual void Clear(void* data) const override final;
    virtual void Empty(void* data, size_t capacaity) const override final;

    virtual bool ForEach(void* data, const foreach_fun& foreach) const override final;
};
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
PTypeTraits Traits(Meta::TType< TVector<T, _Allocator> >) {
    return PTypeTraits::Make< TVectorLikeTraits< TVector<T, _Allocator> > >();
}
//----------------------------------------------------------------------------
template <typename T, size_t _InSitu, typename _Allocator>
PTypeTraits Traits(Meta::TType< TVectorInSitu<T, _InSitu, _Allocator> >) {
    return PTypeTraits::Make< TVectorLikeTraits< TVectorInSitu<T, _InSitu, _Allocator> > >();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
class TAssociativeVectorTraits : public TBaseTypeTraits< TAssociativeVector<_Key, _Value, _EqualTo, _Vector>, TBaseDicoTraits<_Key, _Value> > {
    using base_traits = TBaseTypeTraits< TAssociativeVector<_Key, _Value, _EqualTo, _Vector>, TBaseDicoTraits<_Key, _Value> >;
    using typename base_traits::value_type;
    using typename base_traits::pointer;
    using typename base_traits::const_pointer;

public: // IDicoTraits:
    using typename base_traits::foreach_fun;

    virtual size_t Count(const void* data) const override final;
    virtual bool IsEmpty(const void* data) const override final;

    virtual FAtom Find(const void* data, const FAtom& key) const override final;

    virtual FAtom AddDefault(void* data, FAtom&& rkey) const override final;
    virtual FAtom AddDefault(void* data, const FAtom& key) const override final;

    virtual void AddCopy(void* data, const FAtom& key, const FAtom& value) const override final;
    virtual void AddMove(void* data, const FAtom& key, const FAtom& value) const override final;
    virtual bool Remove(void* data, const FAtom& key) const override final;

    virtual void Reserve(void* data, size_t capacity) const override final;
    virtual void Clear(void* data) const override final;
    virtual void Empty(void* data, size_t capacity) const override final;

    virtual bool ForEach(void* data, const foreach_fun& foreach) const override final;
};
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
PTypeTraits Traits(Meta::TType< TAssociativeVector<_Key, _Value, _EqualTo, _Vector> >) {
    return PTypeTraits::Make< TAssociativeVectorTraits<_Key, _Value, _EqualTo, _Vector> >();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
class THashMapTraits : public TBaseTypeTraits< THashMap<_Key, _Value, _Hasher, _EqualTo, _Allocator>, TBaseDicoTraits<_Key, _Value> > {
    using base_traits = TBaseTypeTraits< THashMap<_Key, _Value, _Hasher, _EqualTo, _Allocator>, TBaseDicoTraits<_Key, _Value> >;
    using typename base_traits::value_type;
    using typename base_traits::pointer;
    using typename base_traits::const_pointer;

public: // IDicoTraits:
    using typename base_traits::foreach_fun;

    virtual size_t Count(const void* data) const override final;
    virtual bool IsEmpty(const void* data) const override final;

    virtual FAtom Find(const void* data, const FAtom& key) const override final;

    virtual FAtom AddDefault(void* data, FAtom&& rkey) const override final;
    virtual FAtom AddDefault(void* data, const FAtom& key) const override final;

    virtual void AddCopy(void* data, const FAtom& key, const FAtom& value) const override final;
    virtual void AddMove(void* data, const FAtom& key, const FAtom& value) const override final;
    virtual bool Remove(void* data, const FAtom& key) const override final;

    virtual void Reserve(void* data, size_t capacity) const override final;
    virtual void Clear(void* data) const override final;
    virtual void Empty(void* data, size_t capacity) const override final;

    virtual bool ForEach(void* data, const foreach_fun& foreach) const override final;
};
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
PTypeTraits Traits(Meta::TType< THashMap<_Key, _Value, _Hasher, _EqualTo, _Allocator> >) {
    return PTypeTraits::Make< THashMapTraits<_Key, _Value, _Hasher, _EqualTo, _Allocator> >();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// RTTI support for enums
template <typename _Enum>
PTypeTraits Traits(Meta::TType< _Enum >, Meta::TEnableIf< std::is_enum_v<_Enum> >* = nullptr) {
    // Beware of the dog !!!
    // *ALWAYS* specify the size of your enums wrapped in RTTI !
    return MakeTraits<typename TIntegral<_Enum>::type>();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// sfinae to detect RTTI support
namespace details {
template <typename T, typename = decltype(Traits(std::declval<Meta::TType<T>>())) >
std::true_type IsSupportedType_(int);
template <typename T>
std::false_type IsSupportedType_(...);
} //!details
template <typename T>
struct TIsSupportedType {
    using is_supported = decltype(details::IsSupportedType_<T>(0));
    STATIC_CONST_INTEGRAL(bool, value, not std::is_same<void, T>::value && is_supported::value);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core

#include "Core.RTTI/NativeTypes-inl.h"
