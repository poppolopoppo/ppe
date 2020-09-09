#pragma once

#include "RTTI/NativeTypes.h"

#include "Container/AssociativeVector.h"
#include "Container/HashMap.h"

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Key, typename _Value>
static CONSTEXPR const PTypeInfos MakeDicoTypeInfos = []() CONSTEXPR NOEXCEPT -> FTypeInfos {
    return FTypeInfos::CombineTypes(
        FTypeId(ETypeFlags::Dico),
        FTypeInfos::BasicInfos<T>(ETypeFlags::Dico),
        MakeTypeInfos<_Key>(),
        MakeTypeInfos<_Value>()
    );
};
//----------------------------------------------------------------------------
// TBaseDicoTraits<_Key, _Value>
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
class TBaseDicoTraits : public IDicoTraits {
protected:
    using IDicoTraits::IDicoTraits;

public: // ITypeTraits
    virtual FStringView TypeName() const override final;

public: // IDicoTraits
    virtual PTypeTraits KeyTraits() const NOEXCEPT override final { return MakeTraits<_Key>(); }
    virtual PTypeTraits ValueTraits() const NOEXCEPT override final { return MakeTraits<_Value>(); }
};
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
FStringView TBaseDicoTraits<_Key, _Value>::TypeName() const {
    ONE_TIME_INITIALIZE(const FStringView, GTypeName, MakeDicoTypeName(
        MakeTraits<_Key>(),
        MakeTraits<_Value>()
    ));
    return GTypeName;
}
//----------------------------------------------------------------------------
// TDicoTraits<_Dico, _Key, _Value>
//----------------------------------------------------------------------------
template <typename _Dico, typename _Key, typename _Value>
class TDicoTraits : public TBaseTypeTraits<_Dico, TBaseDicoTraits<_Key, _Value> > {
protected:
    using base_traits = TBaseTypeTraits<_Dico, TBaseDicoTraits<_Key, _Value> >;

    using typename base_traits::value_type;
    using typename base_traits::pointer;
    using typename base_traits::const_pointer;

    using base_traits::base_traits;

public: // ITypeTraits
    virtual bool IsDefaultValue(const void* data) const NOEXCEPT override final;
    virtual void ResetToDefaultValue(void* data) const override final;

    virtual bool Equals(const void* lhs, const void* rhs) const NOEXCEPT override final;
    virtual hash_t HashValue(const void* data) const NOEXCEPT override final;

    virtual bool DeepEquals(const void* lhs, const void* rhs) const override final;
    virtual void DeepCopy(const void* src, void* dst) const override final;

    virtual bool PromoteCopy(const void* src, const FAtom& dst) const override final;
    virtual bool PromoteMove(void* src, const FAtom& dst) const NOEXCEPT override final;

    virtual void* Cast(void* data, const PTypeTraits& dst) const override final {
        return base_traits::BaseCast(data, dst);
    }

    virtual PTypeTraits CommonType(const PTypeTraits& other) const NOEXCEPT override final {
        return MakeCommonType<_Dico>(other);
    }

protected:
    static _Dico& Unwrap(void* data) NOEXCEPT {
        Assert(data);
        return (*static_cast<_Dico*>(data));
    }
    static const _Dico& Unwrap(const void* data) NOEXCEPT {
        Assert(data);
        return (*static_cast<const _Dico*>(data));
    }
};
//----------------------------------------------------------------------------
template <typename _Dico, typename _Key, typename _Value>
bool TDicoTraits<_Dico, _Key, _Value>::IsDefaultValue(const void* data) const NOEXCEPT {
    return Unwrap(data).empty();
}
//----------------------------------------------------------------------------
template <typename _Dico, typename _Key, typename _Value>
void TDicoTraits<_Dico, _Key, _Value>::ResetToDefaultValue(void* data) const {
    Unwrap(data).clear();
}
//----------------------------------------------------------------------------
template <typename _Dico, typename _Key, typename _Value>
bool TDicoTraits<_Dico, _Key, _Value>::Equals(const void* lhs, const void* rhs) const NOEXCEPT {
    const _Dico& a = Unwrap(lhs);
    const _Dico& b = Unwrap(rhs);

    if (a.size() != b.size())
        return false;

    for (const auto& elt : a) {
        const auto it = b.find(elt.first);
        if (b.end() == it)
            return false;
    }

    return true;
}
//----------------------------------------------------------------------------
template <typename _Dico, typename _Key, typename _Value>
hash_t TDicoTraits<_Dico, _Key, _Value>::HashValue(const void* data) const NOEXCEPT {
    hash_t h{ ITypeTraits::TypeId() };

    const PTypeTraits& key = MakeTraits<_Key>();
    const PTypeTraits& value = MakeTraits<_Value>();

    // #TODO : would need to sort by keys + values before hashing to be stable :/
    for (const auto& elt : Unwrap(data)) {
        hash_combine(h,
            key->HashValue(&elt.first),
            value->HashValue(&elt.second) );
    }

    return h;
}
//----------------------------------------------------------------------------
template <typename _Dico, typename _Key, typename _Value>
bool TDicoTraits<_Dico, _Key, _Value>::DeepEquals(const void* lhs, const void* rhs) const {
    const _Dico& a = Unwrap(lhs);
    const _Dico& b = Unwrap(rhs);

    if (a.size() != b.size())
        return false;

    const PTypeTraits& value = MakeTraits<_Value>();

    // #TODO : use standard equal instead of deep equal for keys, problem ?
    for (const auto& elt : a) {
        const auto it = b.find(elt.first);
        if (b.end() == it || not value->DeepEquals(&elt.second, &it->second))
            return false;
    }

    return true;
}
//----------------------------------------------------------------------------
template <typename _Dico, typename _Key, typename _Value>
void TDicoTraits<_Dico, _Key, _Value>::DeepCopy(const void* src, void* dst) const {
    const PTypeTraits& key = MakeTraits<_Key>();
    const PTypeTraits& value = MakeTraits<_Value>();

    const _Dico& srcT = Unwrap(src);
    _Dico& dstT = Unwrap(dst);

    const size_t n = srcT.size();
    dstT.clear();
    dstT.reserve(n);

    typename _Dico::value_type cpy;
    for (const auto& elt : srcT) {
        key->DeepCopy(&elt.first, &cpy.first);
        value->DeepCopy(&elt.second, &cpy.second);
        dstT.insert(std::move(cpy));
    }
}
//----------------------------------------------------------------------------
template <typename _Dico, typename _Key, typename _Value>
bool TDicoTraits<_Dico, _Key, _Value>::PromoteCopy(const void* src, const FAtom& dst) const {
    Assert(dst);

    if (base_traits::BasePromoteCopy(src, dst))
        return true;

    if (const IDicoTraits* const pdico = dst.Traits()->AsDico()) {
        const _Dico& srcT = Unwrap(src);

        const PTypeTraits& key = MakeTraits<_Key>();
        const PTypeTraits& value = MakeTraits<_Value>();

        STACKLOCAL_ATOM(pkey, pdico->KeyTraits());

        pdico->Empty(dst.Data(), srcT.size());

        for (const auto& elt : srcT) {
            if (not key->PromoteCopy(&elt.first, pkey))
                return false;

            const FAtom pvalue = pdico->AddDefaultMove(dst.Data(), pkey);

            if (not value->PromoteCopy(&elt.second, pvalue))
                return false;
        }

        return true;
    }

    return false;
}
//----------------------------------------------------------------------------
template <typename _Dico, typename _Key, typename _Value>
bool TDicoTraits<_Dico, _Key, _Value>::PromoteMove(void* src, const FAtom& dst) const NOEXCEPT {
    Assert(dst);

    if (base_traits::BasePromoteMove(src, dst))
        return true;

    if (const IDicoTraits * const pdico = dst.Traits()->AsDico()) {
        _Dico& srcT = Unwrap(src);

        const PTypeTraits& key = MakeTraits<_Key>();
        const PTypeTraits& value = MakeTraits<_Value>();

        STACKLOCAL_ATOM(pkey, pdico->KeyTraits());

        pdico->Empty(dst.Data(), srcT.size());

        for (auto& elt : srcT) {
            if (not key->PromoteMove(const_cast<_Key*>(&elt.first), pkey))
                return false;

            const FAtom pvalue = pdico->AddDefaultMove(dst.Data(), pkey);

            if (not value->PromoteMove(&elt.second, pvalue))
                return false;
        }

        srcT.clear();

        return true;
    }

    return false;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// TAssociativeVectorTraits<_Key, _Value, _EqualTo, _Vector>
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
class TAssociativeVectorTraits final : public TDicoTraits< TAssociativeVector<_Key, _Value, _EqualTo, _Vector>, _Key, _Value > {
    using base_traits = TDicoTraits< TAssociativeVector<_Key, _Value, _EqualTo, _Vector>, _Key, _Value >;
    using typename base_traits::value_type;
    using typename base_traits::pointer;
    using typename base_traits::const_pointer;

    using base_traits::Unwrap;

public: // ITypeTraits
    using base_traits::base_traits;

public: // IDicoTraits:
    using typename base_traits::foreach_fun;

    virtual size_t Count(const void* data) const NOEXCEPT override final;
    virtual bool IsEmpty(const void* data) const NOEXCEPT override final;

    virtual FAtom Find(const void* data, const FAtom& key) const NOEXCEPT override final;

    virtual FAtom AddDefaultCopy(void* data, const FAtom& key) const override final;
    virtual FAtom AddDefaultMove(void* data, const FAtom& key) const override final;

    virtual void AddCopy(void* data, const FAtom& key, const FAtom& value) const override final;
    virtual void AddMove(void* data, const FAtom& key, const FAtom& value) const override final;
    virtual bool Remove(void* data, const FAtom& key) const override final;

    virtual void Reserve(void* data, size_t capacity) const override final;
    virtual void Clear(void* data) const override final;
    virtual void Empty(void* data, size_t capacity) const override final;

    virtual bool ForEach(void* data, const foreach_fun& foreach) const NOEXCEPT override final;
};
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
CONSTEXPR PTypeInfos RTTI_TypeInfos(TTypeTag< TAssociativeVector<_Key, _Value, _EqualTo, _Vector> >) {
    return MakeDicoTypeInfos< TAssociativeVector<_Key, _Value, _EqualTo, _Vector>, _Key, _Value >;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
CONSTEXPR PTypeTraits RTTI_Traits(TTypeTag< TAssociativeVector<_Key, _Value, _EqualTo, _Vector> >) {
    return MakeStaticType< TAssociativeVectorTraits<_Key, _Value, _EqualTo, _Vector>, TAssociativeVector<_Key, _Value, _EqualTo, _Vector> >();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
size_t TAssociativeVectorTraits<_Key, _Value, _EqualTo, _Vector>::Count(const void* data) const NOEXCEPT {
    Assert(data);

    return Unwrap(data).size();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
bool TAssociativeVectorTraits<_Key, _Value, _EqualTo, _Vector>::IsEmpty(const void* data) const NOEXCEPT {
    Assert(data);

    return Unwrap(data).empty();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
FAtom TAssociativeVectorTraits<_Key, _Value, _EqualTo, _Vector>::Find(const void* data, const FAtom& key) const NOEXCEPT {
    Assert(data);

    const value_type& d = Unwrap(data);
    const auto it = d.find(key.TypedConstData<_Key>());
    return (it != d.end())
        ? FAtom(&it->second, MakeTraits<_Value>())
        : FAtom();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
FAtom TAssociativeVectorTraits<_Key, _Value, _EqualTo, _Vector>::AddDefaultCopy(void* data, const FAtom& key) const {
    Assert(data);

    value_type& d = Unwrap(data);
    _Value& value = d.Add(key.TypedConstData<_Key>());
    return FAtom(&value, MakeTraits<_Value>());
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
FAtom TAssociativeVectorTraits<_Key, _Value, _EqualTo, _Vector>::AddDefaultMove(void* data, const FAtom& key) const {
    Assert(data);

    value_type& d = Unwrap(data);
    _Value& value = d.Add(std::move(key.TypedData<_Key>()));
    return FAtom(&value, MakeTraits<_Value>());
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
void TAssociativeVectorTraits<_Key, _Value, _EqualTo, _Vector>::AddCopy(void* data, const FAtom& key, const FAtom& value) const {
    Assert(data);

    Unwrap(data).Insert_AssertUnique(
        key.TypedConstData<_Key>(),
        value.TypedConstData<_Value>());
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
void TAssociativeVectorTraits<_Key, _Value, _EqualTo, _Vector>::AddMove(void* data, const FAtom& key, const FAtom& value) const {
    Assert(data);

    Unwrap(data).Insert_AssertUnique(
        std::move(key.TypedData<_Key>()),
        std::move(value.TypedData<_Value>()));
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
bool TAssociativeVectorTraits<_Key, _Value, _EqualTo, _Vector>::Remove(void* data, const FAtom& key) const {
    Assert(data);

    return Unwrap(data).Erase(key.TypedConstData<_Key>());
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
void TAssociativeVectorTraits<_Key, _Value, _EqualTo, _Vector>::Reserve(void* data, size_t capacity) const {
    Assert(data);

    Unwrap(data).reserve(capacity);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
void TAssociativeVectorTraits<_Key, _Value, _EqualTo, _Vector>::Clear(void* data) const {
    Assert(data);

    Unwrap(data).clear();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
void TAssociativeVectorTraits<_Key, _Value, _EqualTo, _Vector>::Empty(void* data, size_t capacity) const {
    Assert(data);

    value_type& d = Unwrap(data);
    if (capacity) {
        d.clear();
        d.reserve(capacity);
    }
    else {
        d.clear_ReleaseMemory();
    }
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
bool TAssociativeVectorTraits<_Key, _Value, _EqualTo, _Vector>::ForEach(void* data, const foreach_fun& foreach) const NOEXCEPT {
    Assert(data);

    const PTypeTraits key_traits = MakeTraits<_Key>();
    const PTypeTraits value_traits = MakeTraits<_Value>();

    for (const auto& it : Unwrap(data)) {
        const FAtom key(&it.first, key_traits);
        const FAtom value(&it.second, value_traits);
        if (not foreach(key, value))
            return false;
    }

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// THashMapTraits<_Key, _Value, _Hasher, _EqualTo, _Allocator>
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
class THashMapTraits final : public TDicoTraits< THashMap<_Key, _Value, _Hasher, _EqualTo, _Allocator>, _Key, _Value > {
    using base_traits = TDicoTraits< THashMap<_Key, _Value, _Hasher, _EqualTo, _Allocator>, _Key, _Value >;
    using typename base_traits::value_type;
    using typename base_traits::pointer;
    using typename base_traits::const_pointer;

    using base_traits::Unwrap;

public: // ITypeTraits
    using base_traits::base_traits;

public: // IDicoTraits:
    using typename base_traits::foreach_fun;

    virtual size_t Count(const void* data) const NOEXCEPT override final;
    virtual bool IsEmpty(const void* data) const NOEXCEPT override final;

    virtual FAtom Find(const void* data, const FAtom& key) const NOEXCEPT override final;

    virtual FAtom AddDefaultCopy(void* data, const FAtom& key) const override final;
    virtual FAtom AddDefaultMove(void* data, const FAtom& key) const override final;

    virtual void AddCopy(void* data, const FAtom& key, const FAtom& value) const override final;
    virtual void AddMove(void* data, const FAtom& key, const FAtom& value) const override final;
    virtual bool Remove(void* data, const FAtom& key) const override final;

    virtual void Reserve(void* data, size_t capacity) const override final;
    virtual void Clear(void* data) const override final;
    virtual void Empty(void* data, size_t capacity) const override final;

    virtual bool ForEach(void* data, const foreach_fun& foreach) const NOEXCEPT override final;
};
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
CONSTEXPR PTypeInfos RTTI_TypeInfos(TTypeTag< THashMap<_Key, _Value, _Hasher, _EqualTo, _Allocator> >) {
    return MakeDicoTypeInfos< THashMap<_Key, _Value, _Hasher, _EqualTo, _Allocator>, _Key, _Value >;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
CONSTEXPR PTypeTraits RTTI_Traits(TTypeTag< THashMap<_Key, _Value, _Hasher, _EqualTo, _Allocator> >) {
    return MakeStaticType< THashMapTraits<_Key, _Value, _Hasher, _EqualTo, _Allocator>, THashMap<_Key, _Value, _Hasher, _EqualTo, _Allocator> >();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
size_t THashMapTraits<_Key, _Value, _Hasher, _EqualTo, _Allocator>::Count(const void* data) const NOEXCEPT {
    Assert(data);

    return Unwrap(data).size();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
bool THashMapTraits<_Key, _Value, _Hasher, _EqualTo, _Allocator>::IsEmpty(const void* data) const NOEXCEPT {
    Assert(data);

    return Unwrap(data).empty();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
FAtom THashMapTraits<_Key, _Value, _Hasher, _EqualTo, _Allocator>::Find(const void* data, const FAtom& key) const NOEXCEPT {
    Assert(data);

    const value_type& d = Unwrap(data);
    const auto it = d.find(key.TypedConstData<_Key>());
    return (it != d.end())
        ? FAtom(&it->second, MakeTraits<_Value>())
        : FAtom();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
FAtom THashMapTraits<_Key, _Value, _Hasher, _EqualTo, _Allocator>::AddDefaultCopy(void* data, const FAtom& key) const {
    Assert(data);

    value_type& d = Unwrap(data);
    _Value& v = d.Add(key.TypedConstData<_Key>());
    return FAtom(&v, MakeTraits<_Value>());
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
FAtom THashMapTraits<_Key, _Value, _Hasher, _EqualTo, _Allocator>::AddDefaultMove(void* data, const FAtom& key) const {
    Assert(data);

    value_type& d = Unwrap(data);
    _Value& v = d.Add(std::move(key.TypedData<_Key>()));
    return FAtom(&v, MakeTraits<_Value>());
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
void THashMapTraits<_Key, _Value, _Hasher, _EqualTo, _Allocator>::AddCopy(void* data, const FAtom& key, const FAtom& value) const {
    Assert(data);

    Unwrap(data).emplace_AssertUnique(
        key.TypedConstData<_Key>(),
        value.TypedConstData<_Value>());
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
void THashMapTraits<_Key, _Value, _Hasher, _EqualTo, _Allocator>::AddMove(void* data, const FAtom& key, const FAtom& value) const {
    Assert(data);

    Unwrap(data).emplace_AssertUnique(
        std::move(key.TypedData<_Key>()),
        std::move(value.TypedData<_Value>()));
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
bool THashMapTraits<_Key, _Value, _Hasher, _EqualTo, _Allocator>::Remove(void* data, const FAtom& key) const {
    Assert(data);

    return Unwrap(data).erase(key.TypedConstData<_Key>());
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
void THashMapTraits<_Key, _Value, _Hasher, _EqualTo, _Allocator>::Reserve(void* data, size_t capacity) const {
    Assert(data);

    Unwrap(data).reserve(capacity);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
void THashMapTraits<_Key, _Value, _Hasher, _EqualTo, _Allocator>::Clear(void* data) const {
    Assert(data);

    Unwrap(data).clear();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
void THashMapTraits<_Key, _Value, _Hasher, _EqualTo, _Allocator>::Empty(void* data, size_t capacity) const {
    Assert(data);

    value_type& d = Unwrap(data);
    if (capacity) {
        d.clear();
        d.reserve(capacity);
    }
    else {
        d.clear_ReleaseMemory();
    }
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
bool THashMapTraits<_Key, _Value, _Hasher, _EqualTo, _Allocator>::ForEach(void* data, const foreach_fun& foreach) const NOEXCEPT {
    const PTypeTraits key_traits = MakeTraits<_Key>();
    const PTypeTraits value_traits = MakeTraits<_Value>();

    for (const auto& it : Unwrap(data)) {
        const FAtom key(&it.first, key_traits);
        const FAtom value(&it.second, value_traits);
        if (not foreach(key, value))
            return false;
    }

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE
