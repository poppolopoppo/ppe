#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core.RTTI/Atom.h"
#include "Core.RTTI/AtomHelpers.h"
#include "Core.RTTI/TypeTraits.h"
#include "Core.RTTI/Typedefs.h"

#include "Core.RTTI/NativeTypes.Definitions-inl.h"

#include "Core/Container/HashMap.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define DECL_RTTI_NATIVETYPE_TRAITS(_Name, T, _TypeId) CORE_RTTI_API PTypeTraits Traits(Meta::TType<T>);
FOREACH_RTTI_NATIVETYPES(DECL_RTTI_NATIVETYPE_TRAITS)
#undef DECL_RTTI_NATIVETYPE_TRAITS
//----------------------------------------------------------------------------
enum class ENativeType : FTypeId {
#define DECL_RTTI_NATIVETYPE_ENUM(_Name, T, _TypeId) _Name = _TypeId,
FOREACH_RTTI_NATIVETYPES(DECL_RTTI_NATIVETYPE_ENUM)
#undef DECL_RTTI_NATIVETYPE_ENUM
}; //!enum class ENativeType
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Parent>
class TBaseTypeTraits : public _Parent {
protected:
    typedef T value_type;
    typedef Meta::TAddPointer<T> pointer;
    typedef Meta::TAddPointer<const T> const_pointer;

public: // ITypeTraits
    virtual void Create(const FAtom& atom) const override final;
    virtual void Destroy(const FAtom& atom) const override final;

    //virtual FTypeId TypeId() const override final;
    //virtual FTypeInfos TypeInfos() const override final;
    virtual size_t SizeInBytes() const override final { return sizeof(T); }

    virtual bool IsDefaultValue(const FAtom& value) const override final;

    virtual bool Equals(const FAtom& lhs, const FAtom& rhs) const override final;
    virtual void Copy(const FAtom& src, const FAtom& dst) const override final;
    virtual void Move(const FAtom& src, const FAtom& dst) const override final;

    virtual void Swap(const FAtom& lhs, const FAtom& rhs) const override final;

    //virtual bool DeepEquals(const FAtom& lhs, const FAtom& rhs) const override final;
    //virtual void DeepCopy(const FAtom& src, const FAtom& dst) const override final;

    //virtual bool PromoteCopy(const FAtom& from, const FAtom& to) const override final;
    //virtual bool PromoteMove(const FAtom& from, const FAtom& to) const override final;

    virtual hash_t HashValue(const FAtom& atom) const override final;

    virtual void Format(std::basic_ostream<char>& oss, const FAtom& atom) const override;
    virtual void Format(std::basic_ostream<wchar_t>& oss, const FAtom& atom) const override;
};
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
class TBasePairTraits : public IPairTraits {
public: // ITypeTraits
    virtual FTypeId TypeId() const override final;
    virtual FTypeInfos TypeInfos() const override final;

    virtual bool DeepEquals(const FAtom& lhs, const FAtom& rhs) const override final;
    virtual void DeepCopy(const FAtom& src, const FAtom& dst) const override final;

    virtual bool PromoteCopy(const FAtom& from, const FAtom& to) const override final;
    virtual bool PromoteMove(const FAtom& from, const FAtom& to) const override final;

public: // IPairTraits
    virtual PTypeTraits FirstTraits() const override final { return MakeTraits<_First>(); }
    virtual PTypeTraits SecondTraits() const override final { return MakeTraits<_Second>(); }
};
//----------------------------------------------------------------------------
template <typename T>
class TBaseListTraits : public IListTraits {
public: // ITypeTraits
    virtual FTypeId TypeId() const override final;
    virtual FTypeInfos TypeInfos() const override final;

    virtual bool DeepEquals(const FAtom& lhs, const FAtom& rhs) const override final;
    virtual void DeepCopy(const FAtom& src, const FAtom& dst) const override final;

    virtual bool PromoteCopy(const FAtom& from, const FAtom& to) const override final;
    virtual bool PromoteMove(const FAtom& from, const FAtom& to) const override final;

public: // IListTraits
    virtual PTypeTraits ValueTraits() const override final { return MakeTraits<T>(); }
};
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
class TBaseDicoTraits : public IDicoTraits {
public: // ITypeTraits
    virtual FTypeId TypeId() const override final;
    virtual FTypeInfos TypeInfos() const override final;

    virtual bool DeepEquals(const FAtom& lhs, const FAtom& rhs) const override final;
    virtual void DeepCopy(const FAtom& src, const FAtom& dst) const override final;

    virtual bool PromoteCopy(const FAtom& from, const FAtom& to) const override final;
    virtual bool PromoteMove(const FAtom& from, const FAtom& to) const override final;

public: // IDicoTraits
    virtual PTypeTraits KeyTraits() const override final { return MakeTraits<_Key>(); }
    virtual PTypeTraits ValueTraits() const override final { return MakeTraits<_Value>(); }
};
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
    virtual FAtom First(const FAtom& pair) const override final;
    virtual FAtom Second(const FAtom& pair) const override final;

    virtual void SetFirstCopy(const FAtom& pair, const FAtom& first) const override final;
    virtual void SetFirstMove(const FAtom& pair, const FAtom& first) const override final;

    virtual void SetSecondCopy(const FAtom& pair, const FAtom& second) const override final;
    virtual void SetSecondMove(const FAtom& pair, const FAtom& second) const override final;
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
    using IListTraits::foreach_fun;

    virtual size_t Count(const FAtom& list) const override final;
    virtual bool Empty(const FAtom& list) const override final;

    virtual FAtom At(const FAtom& list, size_t index) const override final;
    virtual size_t Find(const FAtom& list, const FAtom& item) const override final;

    virtual FAtom AddDefault(const FAtom& list) const override final;
    virtual void AddCopy(const FAtom& list, const FAtom& item) const override final;
    virtual void AddMove(const FAtom& list, const FAtom& item) const override final;
    virtual void Erase(const FAtom& list, size_t index) const override final;
    virtual bool Remove(const FAtom& list, const FAtom& item) const override final;

    virtual void Reserve(const FAtom& list, size_t capacity) const override final;
    virtual void Clear(const FAtom& list) const override final;

    virtual bool ForEach(const FAtom& list, const foreach_fun& foreach) const override final;
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
    using IDicoTraits::foreach_fun;

    virtual size_t Count(const FAtom& dico) const override final;
    virtual bool Empty(const FAtom& dico) const override final;

    virtual FAtom Find(const FAtom& dico, const FAtom& key) const override final;

    virtual FAtom AddDefault(const FAtom& dico, const FAtom& key) const override final;
    virtual void AddCopy(const FAtom& dico, const FAtom& key, const FAtom& value) const override final;
    virtual void AddMove(const FAtom& dico, const FAtom& key, const FAtom& value) const override final;
    virtual bool Remove(const FAtom& dico, const FAtom& key) const override final;

    virtual void Reserve(const FAtom& dico, size_t capacity) const override final;
    virtual void Clear(const FAtom& dico) const override final;

    virtual bool ForEach(const FAtom& dico, const foreach_fun& foreach) const override final;
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
    using IDicoTraits::foreach_fun;

    virtual size_t Count(const FAtom& dico) const override final;
    virtual bool Empty(const FAtom& dico) const override final;

    virtual FAtom Find(const FAtom& dico, const FAtom& key) const override final;

    virtual FAtom AddDefault(const FAtom& dico, const FAtom& key) const override final;
    virtual void AddCopy(const FAtom& dico, const FAtom& key, const FAtom& value) const override final;
    virtual void AddMove(const FAtom& dico, const FAtom& key, const FAtom& value) const override final;
    virtual bool Remove(const FAtom& dico, const FAtom& key) const override final;

    virtual void Reserve(const FAtom& dico, size_t capacity) const override final;
    virtual void Clear(const FAtom& dico) const override final;

    virtual bool ForEach(const FAtom& dico, const foreach_fun& foreach) const override final;
};
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
PTypeTraits Traits(Meta::TType< THashMap<_Key, _Value, _Hasher, _EqualTo, _Allocator> >) {
    return PTypeTraits::Make< THashMapTraits<_Key, _Value, _Hasher, _EqualTo, _Allocator> >();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core

#include "Core.RTTI/NativeTypes-inl.h"