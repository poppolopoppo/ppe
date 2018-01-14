#pragma once

#include "Core.RTTI/NativeTypes.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// TBaseTypeTraits<T, _Parent>
//----------------------------------------------------------------------------
template <typename T, typename _Parent>
void* TBaseTypeTraits<T, _Parent>::Allocate() const {
    return memorypool_type::Allocate();
}
//----------------------------------------------------------------------------
template <typename T, typename _Parent>
void TBaseTypeTraits<T, _Parent>::Deallocate(void* ptr) const {
    Assert(ptr);
    return memorypool_type::Deallocate(ptr);
}
//----------------------------------------------------------------------------
template <typename T, typename _Parent>
void TBaseTypeTraits<T, _Parent>::Create(const FAtom& atom) const {
    Assert(atom);
    Meta::Construct(&atom.TypedData<T>(), Meta::FForceInit{});
}
//----------------------------------------------------------------------------
template <typename T, typename _Parent>
void TBaseTypeTraits<T, _Parent>::CreateCopy(const FAtom& cpy, const FAtom& other) const {
    Assert(cpy);
    Assert(other);
    Meta::Construct(&cpy.TypedData<T>(), other.TypedConstData<T>());
}
//----------------------------------------------------------------------------
template <typename T, typename _Parent>
void TBaseTypeTraits<T, _Parent>::CreateMove(const FAtom& cpy, const FAtom& rvalue) const {
    Assert(cpy);
    Assert(rvalue);
    Meta::Construct(&cpy.TypedData<T>(), std::move(rvalue.TypedData<T>()));
}
//----------------------------------------------------------------------------
template <typename T, typename _Parent>
void TBaseTypeTraits<T, _Parent>::Destroy(const FAtom& atom) const {
    Assert(atom);
    Meta::Destroy(&atom.TypedData<T>());
}
//----------------------------------------------------------------------------
template <typename T, typename _Parent>
bool TBaseTypeTraits<T, _Parent>::IsDefaultValue(const FAtom& value) const {
    return (value.TypedConstData<T>() == Meta::ForceInit<T>());
}
//----------------------------------------------------------------------------
template <typename T, typename _Parent>
bool TBaseTypeTraits<T, _Parent>::Equals(const FAtom& lhs, const FAtom& rhs) const {
    return (lhs.TypedConstData<T>() == rhs.TypedConstData<T>());
}
//----------------------------------------------------------------------------
template <typename T, typename _Parent>
void TBaseTypeTraits<T, _Parent>::Copy(const FAtom& src, const FAtom& dst) const {
    dst.TypedData<T>() = src.TypedConstData<T>();
}
//----------------------------------------------------------------------------
template <typename T, typename _Parent>
void TBaseTypeTraits<T, _Parent>::Move(const FAtom& src, const FAtom& dst) const {
    dst.TypedData<T>() = std::move(src.TypedData<T>());
}
//----------------------------------------------------------------------------
template <typename T, typename _Parent>
void TBaseTypeTraits<T, _Parent>::Swap(const FAtom& lhs, const FAtom& rhs) const  {
    using std::swap;
    swap(lhs.TypedData<T>(), rhs.TypedData<T>());
}
//----------------------------------------------------------------------------
template <typename T, typename _Parent>
void* TBaseTypeTraits<T, _Parent>::Cast(const FAtom& from, const PTypeTraits& to) const {
    return (from.Traits() == to ? from.Data() : nullptr);
}
//----------------------------------------------------------------------------
template <typename T, typename _Parent>
hash_t TBaseTypeTraits<T, _Parent>::HashValue(const FAtom& atom) const {
    return hash_tuple(_Parent::TypeFlags(), atom.TypedConstData<T>());
}
//----------------------------------------------------------------------------
template <typename T, typename _Parent>
void TBaseTypeTraits<T, _Parent>::Format(FTextWriter& oss, const FAtom& atom) const {
    oss << atom.TypedConstData<T>();
}
//----------------------------------------------------------------------------
template <typename T, typename _Parent>
void TBaseTypeTraits<T, _Parent>::Format(FWTextWriter& oss, const FAtom& atom) const {
    oss << atom.TypedConstData<T>();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// TBasePairTraits<_First, _Second>
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
FTypeId TBasePairTraits<_First, _Second>::TypeId() const {
    ONE_TIME_INITIALIZE(const FTypeId, GCachedTypeId, MakePairTypeId(
        MakeTraits<_First>(),
        MakeTraits<_Second>()
    ));
    return GCachedTypeId;
}
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
FTypeInfos TBasePairTraits<_First, _Second>::TypeInfos() const {
    ONE_TIME_INITIALIZE(const FString, GCachedTypeName, MakePairTypeName(
        MakeTraits<_First>(),
        MakeTraits<_Second>()
    ));
    return FTypeInfos(
        GCachedTypeName.MakeView(),
        TBasePairTraits<_First, _Second>::TypeId(),
        ETypeFlags::Pair,
        SizeInBytes()
    );
}
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
bool TBasePairTraits<_First, _Second>::PromoteCopy(const FAtom& from, const FAtom& to) const {
    if (from.Traits() == to.Traits()) {
        Copy(from, to);
        return true;
    }
    else {
        const IPairTraits* const toPair = to.Traits()->AsPair();
        return (toPair &&
            MakeTraits<_First>()->PromoteCopy(First(from), toPair->First(to)) &&
            MakeTraits<_Second>()->PromoteCopy(Second(from), toPair->Second(to)) );
    }
}
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
bool TBasePairTraits<_First, _Second>::PromoteMove(const FAtom& from, const FAtom& to) const {
    if (from.Traits() == to.Traits()) {
        Move(from, to);
        return true;
    }
    else {
        const IPairTraits* const toPair = to.Traits()->AsPair();
        return (toPair &&
            MakeTraits<_First>()->PromoteMove(First(from), toPair->First(to)) &&
            MakeTraits<_Second>()->PromoteMove(Second(from), toPair->Second(to)) );
    }
}
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
bool TBasePairTraits<_First, _Second>::DeepEquals(const FAtom& lhs, const FAtom& rhs) const {
    return (
        MakeTraits<_First>()->DeepEquals(First(lhs), First(rhs)) &&
        MakeTraits<_Second>()->DeepEquals(Second(lhs), Second(rhs)) );
}
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
void TBasePairTraits<_First, _Second>::DeepCopy(const FAtom& src, const FAtom& dst) const {
    MakeTraits<_First>()->DeepCopy(First(src), First(dst));
    MakeTraits<_Second>()->DeepCopy(Second(src), Second(dst));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// TBaseListTraits<T>
//----------------------------------------------------------------------------
template <typename T>
FTypeId TBaseListTraits<T>::TypeId() const {
    ONE_TIME_INITIALIZE(const FTypeId, GCachedTypeId, MakeListTypeId(
        MakeTraits<T>()
    ));
    return GCachedTypeId;
}
//----------------------------------------------------------------------------
template <typename T>
FTypeInfos TBaseListTraits<T>::TypeInfos() const {
    ONE_TIME_INITIALIZE(const FString, GCachedTypeName, MakeListTypeName(
        MakeTraits<T>()
    ));
    return FTypeInfos(
        GCachedTypeName.MakeView(),
        TBaseListTraits<T>::TypeId(),
        ETypeFlags::List,
        SizeInBytes()
    );
}
//----------------------------------------------------------------------------
template <typename T>
bool TBaseListTraits<T>::PromoteCopy(const FAtom& from, const FAtom& to) const {
    if (from.Traits() == to.Traits()) {
        Copy(from, to);
        return true;
    }

    const IListTraits* const toList = to.Traits()->AsList();
    if (nullptr == toList)
        return false;

    const size_t n = Count(from);

    toList->Empty(to, n);

    return ForEach(from, [&to, toList](const FAtom& src) {
        const FAtom dst = toList->AddDefault(to);
        return (src.PromoteCopy(dst));
    });
}
//----------------------------------------------------------------------------
template <typename T>
bool TBaseListTraits<T>::PromoteMove(const FAtom& from, const FAtom& to) const {
    if (from.Traits() == to.Traits()) {
        Move(from, to);
        return true;
    }

    const IListTraits* const toList = to.Traits()->AsList();
    if (nullptr == toList)
        return false;

    const size_t n = Count(from);

    toList->Empty(to, n);

    const bool succeed = ForEach(from, [&to, toList](const FAtom& src) {
        const FAtom dst = toList->AddDefault(to);
        return (src.PromoteMove(dst));
    });

    if (succeed) {
        Clear(from);
        return true;
    }
    else {
        return false;
    }
}
//----------------------------------------------------------------------------
template <typename T>
bool TBaseListTraits<T>::DeepEquals(const FAtom& lhs, const FAtom& rhs) const {
    const size_t n = Count(lhs);
    if (Count(rhs) != n)
        return false;

    const PTypeTraits value_traits = MakeTraits<T>();

    forrange(i, 0, n) {
        const FAtom lhsAtom = At(lhs, i);
        const FAtom rhsAtom = At(rhs, i);

        if (not value_traits->DeepEquals(lhsAtom, rhsAtom))
            return false;
    }

    return true;
}
//----------------------------------------------------------------------------
template <typename T>
void TBaseListTraits<T>::DeepCopy(const FAtom& src, const FAtom& dst) const {
    const size_t n = Count(src);

    Clear(dst);
    Reserve(dst, n);

    const PTypeTraits value_traits = MakeTraits<T>();

    forrange(i, 0, n) {
        const FAtom srcAtom = At(src, i);
        const FAtom dstAtom = AddDefault(dst);

        value_traits->DeepCopy(srcAtom, dstAtom);
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// TBaseDicoTraits<_Key, _Value>
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
FTypeInfos TBaseDicoTraits<_Key, _Value>::TypeInfos() const {
    ONE_TIME_INITIALIZE(const FString, GCachedTypeName, MakeDicoTypeName(
        MakeTraits<_Key>(),
        MakeTraits<_Value>()
    ));
    return FTypeInfos(
        GCachedTypeName.MakeView(),
        TBaseDicoTraits<_Key, _Value>::TypeId(),
        ETypeFlags::Dico,
        SizeInBytes()
    );
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
bool TBaseDicoTraits<_Key, _Value>::PromoteCopy(const FAtom& from, const FAtom& to) const {
    if (from.Traits() == to.Traits()) {
        Copy(from, to);
        return true;
    }

    const IDicoTraits* const toDico = to.Traits()->AsDico();
    if (nullptr == toDico)
        return false;

    const size_t n = Count(from);

    toDico->Empty(to, n);

    STACKLOCAL_ATOM(promotedKey, toDico->KeyTraits());

    return ForEach(from, [&promotedKey, &to, toDico](const FAtom& key, const FAtom& value) {
        if (not key.PromoteCopy(promotedKey))
            return false;

        const FAtom promotedValue = toDico->AddDefault(to, promotedKey);

        return value.PromoteCopy(promotedValue);
    });
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
bool TBaseDicoTraits<_Key, _Value>::PromoteMove(const FAtom& from, const FAtom& to) const {
    if (from.Traits() == to.Traits()) {
        Move(from, to);
        return true;
    }

    const IDicoTraits* const toDico = to.Traits()->AsDico();
    if (nullptr == toDico)
        return false;

    const size_t n = Count(from);

    toDico->Empty(to, n);

    STACKLOCAL_ATOM(promotedKey, toDico->KeyTraits());

    const bool succeed = ForEach(from, [&promotedKey, &to, toDico](const FAtom& key, const FAtom& value) {
        if (not key.PromoteMove(promotedKey))
            return false;

        const FAtom promotedValue = toDico->AddDefault(to, promotedKey);

        return value.PromoteMove(promotedValue);
    });

    if (succeed) {
        Clear(from);
        return true;
    }
    else {
        return false;
    }
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
bool TBaseDicoTraits<_Key, _Value>::DeepEquals(const FAtom& lhs, const FAtom& rhs) const {
    if (Count(lhs) != Count(rhs))
        return false;

    return ForEach(lhs, [this, &rhs](const FAtom& lhsKey, const FAtom& lhsValue) {
        const FAtom rhsValue = Find(rhs, lhsKey);
        return (rhsValue && lhsValue.DeepEquals(rhsValue));
    });
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
void TBaseDicoTraits<_Key, _Value>::DeepCopy(const FAtom& src, const FAtom& dst) const {
    const size_t n = Count(src);

    Clear(dst);
    Reserve(dst, n);

    ForEach(src, [this, &dst](const FAtom& key, const FAtom& value) {
        AddCopy(dst, key, value);
        return true;
    });
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// TPairTraits<_First, _Second>
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
FAtom TPairTraits<_First, _Second>::First(const FAtom& pair) const {
    return FAtom(&pair.TypedData<value_type>().first, MakeTraits<_First>());
}
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
FAtom TPairTraits<_First, _Second>::Second(const FAtom& pair) const {
    return FAtom(&pair.TypedData<value_type>().second, MakeTraits<_Second>());
}
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
void TPairTraits<_First, _Second>::SetFirstCopy(const FAtom& pair, const FAtom& first) const {
    pair.TypedData<value_type>().first = first.TypedConstData<_First>();
}
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
void TPairTraits<_First, _Second>::SetFirstMove(const FAtom& pair, const FAtom& first) const {
    pair.TypedData<value_type>().first = std::move(first.TypedData<_First>());
}
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
void TPairTraits<_First, _Second>::SetSecondCopy(const FAtom& pair, const FAtom& second) const {
    pair.TypedData<value_type>().second = second.TypedConstData<_Second>();
}
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
void TPairTraits<_First, _Second>::SetSecondMove(const FAtom& pair, const FAtom& second) const {
    pair.TypedData<value_type>().second = std::move(second.TypedData<_Second>());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// TVectorLikeTraits<_VectorLike>
//----------------------------------------------------------------------------
template <typename _VectorLike>
size_t TVectorLikeTraits<_VectorLike>::Count(const FAtom& list) const {
    return list.TypedConstData<value_type>().size();
}
//----------------------------------------------------------------------------
template <typename _VectorLike>
bool TVectorLikeTraits<_VectorLike>::IsEmpty(const FAtom& list) const {
    return list.TypedConstData<value_type>().empty();
}
//----------------------------------------------------------------------------
template <typename _VectorLike>
FAtom TVectorLikeTraits<_VectorLike>::At(const FAtom& list, size_t index) const {
    return FAtom(&list.TypedConstData<value_type>()[index], MakeTraits<item_type>());
}
//----------------------------------------------------------------------------
template <typename _VectorLike>
size_t TVectorLikeTraits<_VectorLike>::Find(const FAtom& list, const FAtom& item) const {
    return IndexOf(list.TypedConstData<value_type>(), item.TypedData<item_type>());
}
//----------------------------------------------------------------------------
template <typename _VectorLike>
FAtom TVectorLikeTraits<_VectorLike>::AddDefault(const FAtom& list) const {
    auto& item = list.TypedData<value_type>().push_back_Default();
    return FAtom(&item, MakeTraits<item_type>());
}
//----------------------------------------------------------------------------
template <typename _VectorLike>
void TVectorLikeTraits<_VectorLike>::AddCopy(const FAtom& list, const FAtom& item) const {
    list.TypedData<value_type>().push_back(item.TypedConstData<item_type>());
}
//----------------------------------------------------------------------------
template <typename _VectorLike>
void TVectorLikeTraits<_VectorLike>::AddMove(const FAtom& list, const FAtom& item) const {
    list.TypedData<value_type>().push_back(std::move(item.TypedData<item_type>()));
}
//----------------------------------------------------------------------------
template <typename _VectorLike>
void TVectorLikeTraits<_VectorLike>::Erase(const FAtom& list, size_t index) const {
    value_type& v = list.TypedData<value_type>();
    v.erase(v.begin() + index);
}
//----------------------------------------------------------------------------
template <typename _VectorLike>
bool TVectorLikeTraits<_VectorLike>::Remove(const FAtom& list, const FAtom& item) const {
    return Remove_ReturnIfExists(list.TypedData<value_type>(), item.TypedConstData<item_type>());
}
//----------------------------------------------------------------------------
template <typename _VectorLike>
void TVectorLikeTraits<_VectorLike>::Reserve(const FAtom& list, size_t capacity) const {
    list.TypedData<value_type>().reserve(capacity);
}
//----------------------------------------------------------------------------
template <typename _VectorLike>
void TVectorLikeTraits<_VectorLike>::Clear(const FAtom& list) const {
    list.TypedData<value_type>().clear();
}
//----------------------------------------------------------------------------
template <typename _VectorLike>
void TVectorLikeTraits<_VectorLike>::Empty(const FAtom& list, size_t capacity) const {
    value_type& v = list.TypedData<value_type>();
    if (capacity) {
        v.clear();
        v.reserve(capacity);
    }
    else {
        v.clear_ReleaseMemory();
    }
}
//----------------------------------------------------------------------------
template <typename _VectorLike>
bool TVectorLikeTraits<_VectorLike>::ForEach(const FAtom& list, const foreach_fun& foreach) const {
    const PTypeTraits value_traits = MakeTraits<item_type>();

    for (auto& elt : list.TypedData<value_type>()) {
        if (not foreach(FAtom(&elt, value_traits)))
            return false;
    }

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// TAssociativeVectorTraits<_Key, _Value, _EqualTo, _Vector>
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
size_t TAssociativeVectorTraits<_Key, _Value, _EqualTo, _Vector>::Count(const FAtom& dico) const {
    return dico.TypedConstData<value_type>().size();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
bool TAssociativeVectorTraits<_Key, _Value, _EqualTo, _Vector>::IsEmpty(const FAtom& dico) const {
    return dico.TypedConstData<value_type>().empty();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
FAtom TAssociativeVectorTraits<_Key, _Value, _EqualTo, _Vector>::Find(const FAtom& dico, const FAtom& key) const {
    const value_type& d = dico.TypedConstData<value_type>();
    const auto it = d.find(key.TypedConstData<_Key>());
    return (it != d.end())
        ? FAtom(&it->second, MakeTraits<_Value>())
        : FAtom();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
FAtom TAssociativeVectorTraits<_Key, _Value, _EqualTo, _Vector>::AddDefault(const FAtom& dico, FAtom&& rkey) const {
    value_type& d = dico.TypedData<value_type>();
    _Value& value = d.Add(std::move(rkey.TypedData<_Key>()));
    return FAtom(&value, MakeTraits<_Value>());
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
FAtom TAssociativeVectorTraits<_Key, _Value, _EqualTo, _Vector>::AddDefault(const FAtom& dico, const FAtom& key) const {
    value_type& d = dico.TypedData<value_type>();
    _Value& value = d.Add(key.TypedConstData<_Key>());
    return FAtom(&value, MakeTraits<_Value>());
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
void TAssociativeVectorTraits<_Key, _Value, _EqualTo, _Vector>::AddCopy(const FAtom& dico, const FAtom& key, const FAtom& value) const {
    dico.TypedData<value_type>().Insert_AssertUnique(
        key.TypedConstData<_Key>(),
        value.TypedConstData<_Value>() );
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
void TAssociativeVectorTraits<_Key, _Value, _EqualTo, _Vector>::AddMove(const FAtom& dico, const FAtom& key, const FAtom& value) const {
    dico.TypedData<value_type>().Insert_AssertUnique(
        std::move(key.TypedData<_Key>()),
        std::move(value.TypedData<_Value>()) );
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
bool TAssociativeVectorTraits<_Key, _Value, _EqualTo, _Vector>::Remove(const FAtom& dico, const FAtom& key) const {
    return dico.TypedData<value_type>().Erase(key.TypedConstData<_Key>());
}

//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
void TAssociativeVectorTraits<_Key, _Value, _EqualTo, _Vector>::Reserve(const FAtom& dico, size_t capacity) const {
    dico.TypedData<value_type>().reserve(capacity);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
void TAssociativeVectorTraits<_Key, _Value, _EqualTo, _Vector>::Clear(const FAtom& dico) const {
    dico.TypedData<value_type>().clear();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
void TAssociativeVectorTraits<_Key, _Value, _EqualTo, _Vector>::Empty(const FAtom& dico, size_t capacity) const {
    value_type& d = dico.TypedData<value_type>();
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
bool TAssociativeVectorTraits<_Key, _Value, _EqualTo, _Vector>::ForEach(const FAtom& dico, const foreach_fun& foreach) const {
    const PTypeTraits key_traits = MakeTraits<_Key>();
    const PTypeTraits value_traits = MakeTraits<_Value>();

    for (const auto& it : dico.TypedData<value_type>()) {
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
size_t THashMapTraits<_Key, _Value, _Hasher, _EqualTo, _Allocator>::Count(const FAtom& dico) const {
    return dico.TypedConstData<value_type>().size();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
bool THashMapTraits<_Key, _Value, _Hasher, _EqualTo, _Allocator>::IsEmpty(const FAtom& dico) const {
    return dico.TypedConstData<value_type>().empty();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
FAtom THashMapTraits<_Key, _Value, _Hasher, _EqualTo, _Allocator>::Find(const FAtom& dico, const FAtom& key) const {
    const value_type& d = dico.TypedConstData<value_type>();
    const auto it = d.find(key.TypedConstData<_Key>());
    return (it != d.end())
        ? FAtom(&it->second, MakeTraits<_Value>())
        : FAtom();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
FAtom THashMapTraits<_Key, _Value, _Hasher, _EqualTo, _Allocator>::AddDefault(const FAtom& dico, FAtom&& rkey) const {
    value_type& d = dico.TypedData<value_type>();
    _Value& v = d.Add(std::move(rkey.TypedData<_Key>()));
    return FAtom(&v, MakeTraits<_Value>());
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
FAtom THashMapTraits<_Key, _Value, _Hasher, _EqualTo, _Allocator>::AddDefault(const FAtom& dico, const FAtom& key) const {
    value_type& d = dico.TypedData<value_type>();
    _Value& v = d.Add(key.TypedConstData<_Key>());
    return FAtom(&v, MakeTraits<_Value>());
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
void THashMapTraits<_Key, _Value, _Hasher, _EqualTo, _Allocator>::AddCopy(const FAtom& dico, const FAtom& key, const FAtom& value) const {
    dico.TypedData<value_type>().emplace_AssertUnique(
        key.TypedConstData<_Key>(),
        value.TypedConstData<_Value>() );
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
void THashMapTraits<_Key, _Value, _Hasher, _EqualTo, _Allocator>::AddMove(const FAtom& dico, const FAtom& key, const FAtom& value) const {
    dico.TypedData<value_type>().emplace_AssertUnique(
        std::move(key.TypedData<_Key>()),
        std::move(value.TypedData<_Value>()) );
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
bool THashMapTraits<_Key, _Value, _Hasher, _EqualTo, _Allocator>::Remove(const FAtom& dico, const FAtom& key) const {
    return dico.TypedData<value_type>().erase(key.TypedConstData<_Key>());
}

//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
void THashMapTraits<_Key, _Value, _Hasher, _EqualTo, _Allocator>::Reserve(const FAtom& dico, size_t capacity) const {
    dico.TypedData<value_type>().reserve(capacity);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
void THashMapTraits<_Key, _Value, _Hasher, _EqualTo, _Allocator>::Clear(const FAtom& dico) const {
    dico.TypedData<value_type>().clear();
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
void THashMapTraits<_Key, _Value, _Hasher, _EqualTo, _Allocator>::Empty(const FAtom& dico, size_t capacity) const {
    value_type& d = dico.TypedData<value_type>();
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
bool THashMapTraits<_Key, _Value, _Hasher, _EqualTo, _Allocator>::ForEach(const FAtom& dico, const foreach_fun& foreach) const {
    const PTypeTraits key_traits = MakeTraits<_Key>();
    const PTypeTraits value_traits = MakeTraits<_Value>();

    for (const auto& it : dico.TypedData<value_type>()) {
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
