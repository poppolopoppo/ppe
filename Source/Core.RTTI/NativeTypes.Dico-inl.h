#pragma once

#include "Core.RTTI/NativeTypes.h"

#include "Core/Container/HashMap.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// TBaseDicoTraits<_Key, _Value>
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
class TBaseDicoTraits : public IDicoTraits {
public: // ITypeTraits
    virtual FTypeId TypeId() const override final;
    virtual ETypeFlags TypeFlags() const override final;
    virtual FTypeInfos TypeInfos() const override final;

    virtual bool IsDefaultValue(const void* data) const override final;
    virtual void ResetToDefaultValue(void* data) const override final;

    virtual bool Equals(const void* lhs, const void* rhs) const override final;
    virtual hash_t HashValue(const void* data) const override final;

    virtual bool DeepEquals(const void* lhs, const void* rhs) const override final;
    virtual void DeepCopy(const void* src, void* dst) const override final;

    virtual bool PromoteCopy(const void* src, const FAtom& dst) const override /*final*/;
    virtual bool PromoteMove(void* src, const FAtom& dst) const override /*final*/;

public: // IDicoTraits
    virtual PTypeTraits KeyTraits() const override final { return MakeTraits<_Key>(); }
    virtual PTypeTraits ValueTraits() const override final { return MakeTraits<_Value>(); }
};
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
FTypeId TBaseDicoTraits<_Key, _Value>::TypeId() const {
    ONE_TIME_INITIALIZE(const FTypeId, GCachedTypeId, MakeDicoTypeId(
        MakeTraits<_Key>(),
        MakeTraits<_Value>()
    ));
    return GCachedTypeId;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
ETypeFlags TBaseDicoTraits<_Key, _Value>::TypeFlags() const {
    return ETypeFlags::Dico;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
FTypeInfos TBaseDicoTraits<_Key, _Value>::TypeInfos() const {
    ONE_TIME_INITIALIZE(const FString, GCachedTypeName, MakeDicoTypeName(
        MakeTraits<_Key>(),
        MakeTraits<_Value>()
    ));
    return FTypeInfos(
        GCachedTypeName.MakeView(),
        TBaseDicoTraits<_Key, _Value>::TypeId(),
        TBaseDicoTraits<_Key, _Value>::TypeFlags(),
        SizeInBytes()
    );
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
bool TBaseDicoTraits<_Key, _Value>::IsDefaultValue(const void* data) const {
    return IsEmpty(data);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
void TBaseDicoTraits<_Key, _Value>::ResetToDefaultValue(void* data) const {
    Clear(data);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
bool TBaseDicoTraits<_Key, _Value>::Equals(const void* lhs, const void* rhs) const {
    Assert(lhs);
    Assert(rhs);

    const size_t n = Count(lhs);
    if (Count(rhs) != n)
        return false;

    return ForEach((void*)lhs, [this, rhs](const FAtom& lhsKey, const FAtom& lhsValue) {
        const FAtom rhsValue = Find(rhs, lhsKey);
        return (lhsValue == rhsValue);
    });
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
hash_t TBaseDicoTraits<_Key, _Value>::HashValue(const void* data) const {
    Assert(data);

    hash_t h{ TypeId() };

    ForEach((void*)data, [&h](const FAtom& key, const FAtom& value) {
        hash_combine(h, key.HashValue(), value.HashValue());
        return true;
    });

    return h;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
bool TBaseDicoTraits<_Key, _Value>::PromoteCopy(const void* src, const FAtom& dst) const {
    Assert(src);
    Assert(dst);

    if (const IDicoTraits* const dstDico = dst.Traits()->AsDico()) {
        const size_t n = Count(src);
        dstDico->Empty(dst.Data(), n);

        STACKLOCAL_ATOM(promotedKey, dstDico->KeyTraits());

        return ForEach(const_cast<void*>(src), [&promotedKey, &dst, dstDico](const FAtom& key, const FAtom& value) {
            if (not key.PromoteCopy(promotedKey))
                return false;

            const FAtom promotedValue = dstDico->AddDefaultMove(dst.Data(), promotedKey);
            return value.PromoteCopy(promotedValue);
        });
    }

    return false;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
bool TBaseDicoTraits<_Key, _Value>::PromoteMove(void* src, const FAtom& dst) const {
    Assert(src);
    Assert(dst);

    if (const IDicoTraits* const dstDico = dst.Traits()->AsDico()) {
        const size_t n = Count(src);
        dstDico->Empty(dst.Data(), n);

        STACKLOCAL_ATOM(promotedKey, dstDico->KeyTraits());

        const bool succeed = ForEach(src, [&promotedKey, &dst, dstDico](const FAtom& key, const FAtom& value) {
            if (not key.PromoteMove(promotedKey))
                return false;

            const FAtom promotedValue = dstDico->AddDefaultMove(dst.Data(), promotedKey);
            return value.PromoteMove(promotedValue);
        });

        if (succeed) {
            Clear(src);
            return true;
        }
    }

    return false;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
bool TBaseDicoTraits<_Key, _Value>::DeepEquals(const void* lhs, const void* rhs) const {
    if (Count(lhs) != Count(rhs))
        return false;

    return ForEach(const_cast<void*>(lhs), [this, rhs](const FAtom& key, const FAtom& value) {
        const FAtom other = Find(rhs, key);
        return (other && value.DeepEquals(other));
    });
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
void TBaseDicoTraits<_Key, _Value>::DeepCopy(const void* src, void* dst) const {
    const size_t n = Count(src);
    Empty(dst, n);

    STACKLOCAL_ATOM(promotedKey, KeyTraits());

    ForEach(const_cast<void*>(src), [this, dst, &promotedKey](const FAtom& key, const FAtom& value) {
        key.DeepCopy(promotedKey);
        const FAtom promotedValue = AddDefaultMove(dst, promotedKey);
        value.DeepCopy(promotedValue);
        return true;
    });
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// TAssociativeVectorTraits<_Key, _Value, _EqualTo, _Vector>
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
class TAssociativeVectorTraits final : public TBaseTypeTraits< TAssociativeVector<_Key, _Value, _EqualTo, _Vector>, TBaseDicoTraits<_Key, _Value> > {
    using base_traits = TBaseTypeTraits< TAssociativeVector<_Key, _Value, _EqualTo, _Vector>, TBaseDicoTraits<_Key, _Value> >;
    using typename base_traits::value_type;
    using typename base_traits::pointer;
    using typename base_traits::const_pointer;

public: // IDicoTraits:
    using typename base_traits::foreach_fun;

    virtual size_t Count(const void* data) const override final;
    virtual bool IsEmpty(const void* data) const override final;

    virtual FAtom Find(const void* data, const FAtom& key) const override final;

    virtual FAtom AddDefaultCopy(void* data, const FAtom& key) const override final;
    virtual FAtom AddDefaultMove(void* data, const FAtom& key) const override final;

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
PTypeTraits Traits(Meta::TType< TAssociativeVector<_Key, _Value, _EqualTo, _Vector> >) noexcept {
    return PTypeTraits::Make< TAssociativeVectorTraits<_Key, _Value, _EqualTo, _Vector> >();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
size_t TAssociativeVectorTraits<_Key, _Value, _EqualTo, _Vector>::Count(const void* data) const {
    Assert(data);

    return reinterpret_cast<const value_type*>(data)->size();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
bool TAssociativeVectorTraits<_Key, _Value, _EqualTo, _Vector>::IsEmpty(const void* data) const {
    Assert(data);

    return reinterpret_cast<const value_type*>(data)->empty();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
FAtom TAssociativeVectorTraits<_Key, _Value, _EqualTo, _Vector>::Find(const void* data, const FAtom& key) const {
    Assert(data);

    const value_type& d = (*reinterpret_cast<const value_type*>(data));
    const auto it = d.find(key.TypedConstData<_Key>());
    return (it != d.end())
        ? FAtom(&it->second, MakeTraits<_Value>())
        : FAtom();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
FAtom TAssociativeVectorTraits<_Key, _Value, _EqualTo, _Vector>::AddDefaultCopy(void* data, const FAtom& key) const {
    Assert(data);

    value_type& d = (*reinterpret_cast<value_type*>(data));
    _Value& value = d.Add(key.TypedConstData<_Key>());
    return FAtom(&value, MakeTraits<_Value>());
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
FAtom TAssociativeVectorTraits<_Key, _Value, _EqualTo, _Vector>::AddDefaultMove(void* data, const FAtom& key) const {
    Assert(data);

    value_type& d = (*reinterpret_cast<value_type*>(data));
    _Value& value = d.Add(std::move(key.TypedData<_Key>()));
    return FAtom(&value, MakeTraits<_Value>());
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
void TAssociativeVectorTraits<_Key, _Value, _EqualTo, _Vector>::AddCopy(void* data, const FAtom& key, const FAtom& value) const {
    Assert(data);

    reinterpret_cast<value_type*>(data)->Insert_AssertUnique(
        key.TypedConstData<_Key>(),
        value.TypedConstData<_Value>());
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
void TAssociativeVectorTraits<_Key, _Value, _EqualTo, _Vector>::AddMove(void* data, const FAtom& key, const FAtom& value) const {
    Assert(data);

    reinterpret_cast<value_type*>(data)->Insert_AssertUnique(
        std::move(key.TypedData<_Key>()),
        std::move(value.TypedData<_Value>()));
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
bool TAssociativeVectorTraits<_Key, _Value, _EqualTo, _Vector>::Remove(void* data, const FAtom& key) const {
    Assert(data);

    return reinterpret_cast<value_type*>(data)->Erase(key.TypedConstData<_Key>());
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
void TAssociativeVectorTraits<_Key, _Value, _EqualTo, _Vector>::Reserve(void* data, size_t capacity) const {
    Assert(data);

    reinterpret_cast<value_type*>(data)->reserve(capacity);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
void TAssociativeVectorTraits<_Key, _Value, _EqualTo, _Vector>::Clear(void* data) const {
    Assert(data);

    reinterpret_cast<value_type*>(data)->clear();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
void TAssociativeVectorTraits<_Key, _Value, _EqualTo, _Vector>::Empty(void* data, size_t capacity) const {
    Assert(data);

    value_type& d = (*reinterpret_cast<value_type*>(data));
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
bool TAssociativeVectorTraits<_Key, _Value, _EqualTo, _Vector>::ForEach(void* data, const foreach_fun& foreach) const {
    Assert(data);

    const PTypeTraits key_traits = MakeTraits<_Key>();
    const PTypeTraits value_traits = MakeTraits<_Value>();

    for (const auto& it : *reinterpret_cast<value_type*>(data)) {
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
class THashMapTraits final : public TBaseTypeTraits< THashMap<_Key, _Value, _Hasher, _EqualTo, _Allocator>, TBaseDicoTraits<_Key, _Value> > {
    using base_traits = TBaseTypeTraits< THashMap<_Key, _Value, _Hasher, _EqualTo, _Allocator>, TBaseDicoTraits<_Key, _Value> >;
    using typename base_traits::value_type;
    using typename base_traits::pointer;
    using typename base_traits::const_pointer;

public: // IDicoTraits:
    using typename base_traits::foreach_fun;

    virtual size_t Count(const void* data) const override final;
    virtual bool IsEmpty(const void* data) const override final;

    virtual FAtom Find(const void* data, const FAtom& key) const override final;

    virtual FAtom AddDefaultCopy(void* data, const FAtom& key) const override final;
    virtual FAtom AddDefaultMove(void* data, const FAtom& key) const override final;

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
PTypeTraits Traits(Meta::TType< THashMap<_Key, _Value, _Hasher, _EqualTo, _Allocator> >) noexcept {
    return PTypeTraits::Make< THashMapTraits<_Key, _Value, _Hasher, _EqualTo, _Allocator> >();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
size_t THashMapTraits<_Key, _Value, _Hasher, _EqualTo, _Allocator>::Count(const void* data) const {
    Assert(data);

    return reinterpret_cast<const value_type*>(data)->size();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
bool THashMapTraits<_Key, _Value, _Hasher, _EqualTo, _Allocator>::IsEmpty(const void* data) const {
    Assert(data);

    return reinterpret_cast<const value_type*>(data)->empty();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
FAtom THashMapTraits<_Key, _Value, _Hasher, _EqualTo, _Allocator>::Find(const void* data, const FAtom& key) const {
    Assert(data);

    const value_type& d = (*reinterpret_cast<const value_type*>(data));
    const auto it = d.find(key.TypedConstData<_Key>());
    return (it != d.end())
        ? FAtom(&it->second, MakeTraits<_Value>())
        : FAtom();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
FAtom THashMapTraits<_Key, _Value, _Hasher, _EqualTo, _Allocator>::AddDefaultCopy(void* data, const FAtom& key) const {
    Assert(data);

    value_type& d = (*reinterpret_cast<value_type*>(data));
    _Value& v = d.Add(key.TypedConstData<_Key>());
    return FAtom(&v, MakeTraits<_Value>());
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
FAtom THashMapTraits<_Key, _Value, _Hasher, _EqualTo, _Allocator>::AddDefaultMove(void* data, const FAtom& key) const {
    Assert(data);

    value_type& d = (*reinterpret_cast<value_type*>(data));
    _Value& v = d.Add(std::move(key.TypedData<_Key>()));
    return FAtom(&v, MakeTraits<_Value>());
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
void THashMapTraits<_Key, _Value, _Hasher, _EqualTo, _Allocator>::AddCopy(void* data, const FAtom& key, const FAtom& value) const {
    Assert(data);

    reinterpret_cast<value_type*>(data)->emplace_AssertUnique(
        key.TypedConstData<_Key>(),
        value.TypedConstData<_Value>());
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
void THashMapTraits<_Key, _Value, _Hasher, _EqualTo, _Allocator>::AddMove(void* data, const FAtom& key, const FAtom& value) const {
    Assert(data);

    reinterpret_cast<value_type*>(data)->emplace_AssertUnique(
        std::move(key.TypedData<_Key>()),
        std::move(value.TypedData<_Value>()));
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
bool THashMapTraits<_Key, _Value, _Hasher, _EqualTo, _Allocator>::Remove(void* data, const FAtom& key) const {
    Assert(data);

    return reinterpret_cast<value_type*>(data)->erase(key.TypedConstData<_Key>());
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
void THashMapTraits<_Key, _Value, _Hasher, _EqualTo, _Allocator>::Reserve(void* data, size_t capacity) const {
    Assert(data);

    reinterpret_cast<value_type*>(data)->reserve(capacity);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
void THashMapTraits<_Key, _Value, _Hasher, _EqualTo, _Allocator>::Clear(void* data) const {
    Assert(data);

    reinterpret_cast<value_type*>(data)->clear();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
void THashMapTraits<_Key, _Value, _Hasher, _EqualTo, _Allocator>::Empty(void* data, size_t capacity) const {
    Assert(data);

    value_type& d = (*reinterpret_cast<value_type*>(data));
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
bool THashMapTraits<_Key, _Value, _Hasher, _EqualTo, _Allocator>::ForEach(void* data, const foreach_fun& foreach) const {
    const PTypeTraits key_traits = MakeTraits<_Key>();
    const PTypeTraits value_traits = MakeTraits<_Value>();

    for (const auto& it : *reinterpret_cast<value_type*>(data)) {
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
} //!namespace Core
