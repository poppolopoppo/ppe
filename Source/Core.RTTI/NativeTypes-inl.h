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
void TBaseTypeTraits<T, _Parent>::Create(void* data) const {
    Assert(data);

    Meta::Construct(reinterpret_cast<T*>(data), Meta::ForceInit);
}
//----------------------------------------------------------------------------
template <typename T, typename _Parent>
void TBaseTypeTraits<T, _Parent>::CreateCopy(void* data, const void* other) const {
    Assert(data);
    Assert(other);

    Meta::Construct(reinterpret_cast<T*>(data), *reinterpret_cast<const T*>(other));
}
//----------------------------------------------------------------------------
template <typename T, typename _Parent>
void TBaseTypeTraits<T, _Parent>::CreateMove(void* data, void* rvalue) const {
    Assert(data);
    Assert(rvalue);

    Meta::Construct(reinterpret_cast<T*>(data), std::move(*reinterpret_cast<T*>(rvalue)));
}
//----------------------------------------------------------------------------
template <typename T, typename _Parent>
void TBaseTypeTraits<T, _Parent>::Destroy(void* data) const {
    Assert(data);

    Meta::Destroy(reinterpret_cast<T*>(data));
}
//----------------------------------------------------------------------------
template <typename T, typename _Parent>
bool TBaseTypeTraits<T, _Parent>::Equals(const void* lhs, const void* rhs) const {
    Assert(lhs);
    Assert(rhs);

    return (*reinterpret_cast<const T*>(lhs) == *reinterpret_cast<const T*>(rhs));
}
//----------------------------------------------------------------------------
template <typename T, typename _Parent>
void TBaseTypeTraits<T, _Parent>::Copy(const void* src, void* dst) const {
    Assert(src);
    Assert(dst);

    *reinterpret_cast<T*>(dst) = *reinterpret_cast<const T*>(src);
}
//----------------------------------------------------------------------------
template <typename T, typename _Parent>
void TBaseTypeTraits<T, _Parent>::Move(void* src, void* dst) const {
    Assert(src);
    Assert(dst);

    *reinterpret_cast<T*>(dst) = std::move(*reinterpret_cast<T*>(src));
}
//----------------------------------------------------------------------------
template <typename T, typename _Parent>
void TBaseTypeTraits<T, _Parent>::Swap(void* lhs, void* rhs) const  {
    Assert(lhs);
    Assert(rhs);

    using std::swap;
    swap(*reinterpret_cast<T*>(lhs), *reinterpret_cast<T*>(rhs));
}
//----------------------------------------------------------------------------
template <typename T, typename _Parent>
void* TBaseTypeTraits<T, _Parent>::Cast(void* data, const PTypeTraits& dst) const {
    Assert(data);

    return (*dst == *this ? data : nullptr);
}
//----------------------------------------------------------------------------
template <typename T, typename _Parent>
hash_t TBaseTypeTraits<T, _Parent>::HashValue(const void* data) const {
    Assert(data);

    return hash_tuple(TypeId(), *reinterpret_cast<const T*>(data));
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
ETypeFlags TBasePairTraits<_First, _Second>::TypeFlags() const {
    ONE_TIME_INITIALIZE(const ETypeFlags, GCachedTypeFlags, (
        ETypeFlags::Pair + (
            (ETypeFlags::POD & MakeTraits<_First>()->TypeFlags()) &&
            (ETypeFlags::POD & MakeTraits<_Second>()->TypeFlags())
                ? ETypeFlags::POD : ETypeFlags(0) )
    ));
    return GCachedTypeFlags;
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
        TBasePairTraits<_First, _Second>::TypeFlags(),
        SizeInBytes()
    );
}
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
bool TBasePairTraits<_First, _Second>::IsDefaultValue(const void* data) const {
    return (First(const_cast<void*>(data)).IsDefaultValue() &&
            Second(const_cast<void*>(data)).IsDefaultValue() );
}
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
void TBasePairTraits<_First, _Second>::ResetToDefaultValue(void* data) const {
    First(data).ResetToDefaultValue();
    Second(data).ResetToDefaultValue();
}
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
bool TBasePairTraits<_First, _Second>::PromoteCopy(const void* src, const FAtom& dst) const {
    Assert(src);
    Assert(dst);

    if (*dst.Traits() == *this) {
        Copy(src, dst.Data());
        return true;
    }
    else if (const IPairTraits* const dstPair = dst.Traits()->AsPair()) {
        return (First(const_cast<void*>(src)).PromoteCopy(dstPair->First(dst.Data())) &&
                Second(const_cast<void*>(src)).PromoteCopy(dstPair->Second(dst.Data())) );
    }

    return false;
}
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
bool TBasePairTraits<_First, _Second>::PromoteMove(void* src, const FAtom& dst) const {
    Assert(src);
    Assert(dst);

    if (*dst.Traits() == *this) {
        Move(src, dst.Data());
        return true;
    }
    else if (const IPairTraits* const dstPair = dst.Traits()->AsPair()) {
        return (First(src).PromoteMove(dstPair->First(dst.Data())) &&
                Second(src).PromoteMove(dstPair->Second(dst.Data())) );
    }

    return false;
}
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
bool TBasePairTraits<_First, _Second>::DeepEquals(const void* lhs, const void* rhs) const {
    Assert(lhs);
    Assert(rhs);

    return (First(const_cast<void*>(lhs)).DeepEquals(First(const_cast<void*>(rhs))) &&
            Second(const_cast<void*>(lhs)).DeepEquals(Second(const_cast<void*>(rhs))) );
}
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
void TBasePairTraits<_First, _Second>::DeepCopy(const void* src, void* dst) const {
    First(const_cast<void*>(src)).DeepCopy(First(dst));
    Second(const_cast<void*>(src)).DeepCopy(Second(dst));
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
ETypeFlags TBaseListTraits<T>::TypeFlags() const {
    return ETypeFlags::List;
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
        TBaseListTraits<T>::TypeFlags(),
        SizeInBytes()
    );
}
//----------------------------------------------------------------------------
template <typename T>
bool TBaseListTraits<T>::IsDefaultValue(const void* data) const {
    return IsEmpty(data);
}
//----------------------------------------------------------------------------
template <typename T>
void TBaseListTraits<T>::ResetToDefaultValue(void* data) const {
    Clear(data);
}
//----------------------------------------------------------------------------
template <typename T>
bool TBaseListTraits<T>::PromoteCopy(const void* src, const FAtom& dst) const {
    Assert(src);
    Assert(dst);

    if (*dst.Traits() == *this) {
        Copy(src, dst.Data());
        return true;
    }
    else if (const IListTraits* const dstList = dst.Traits()->AsList()) {
        const size_t n = Count(src);
        dstList->Empty(dst.Data(), n);

        void* const pdst = dst.Data();

        return ForEach(const_cast<void*>(src), [pdst, dstList](const FAtom& it) {
            const FAtom last = dstList->AddDefault(pdst);
            return (it.PromoteCopy(last));
        });
    }

    return false;
}
//----------------------------------------------------------------------------
template <typename T>
bool TBaseListTraits<T>::PromoteMove(void* src, const FAtom& dst) const {
    Assert(src);
    Assert(dst);

    if (*dst.Traits() == *this) {
        Move(src, dst.Data());
        return true;
    }
    else if (const IListTraits* const dstList = dst.Traits()->AsList()) {
        const size_t n = Count(src);
        dstList->Empty(dst.Data(), n);

        void* const pdst = dst.Data();

        const bool succeed = ForEach(src, [pdst, dstList](const FAtom& it) {
            const FAtom last = dstList->AddDefault(pdst);
            return (it.PromoteMove(last));
        });

        if (succeed) {
            Clear(src);
            return true;
        }
    }

    return false;
}
//----------------------------------------------------------------------------
template <typename T>
bool TBaseListTraits<T>::DeepEquals(const void* lhs, const void* rhs) const {
    const size_t n = Count(lhs);
    if (Count(rhs) != n)
        return false;

    const PTypeTraits value_traits = MakeTraits<T>();

    forrange(i, 0, n) {
        const FAtom a = At(const_cast<void*>(lhs), i);
        const FAtom b = At(const_cast<void*>(rhs), i);

        if (not value_traits->DeepEquals(a.Data(), b.Data()))
            return false;
    }

    return true;
}
//----------------------------------------------------------------------------
template <typename T>
void TBaseListTraits<T>::DeepCopy(const void* src, void* dst) const {
    const size_t n = Count(src);

    Clear(dst);
    Reserve(dst, n);

    const PTypeTraits value_traits = MakeTraits<T>();

    forrange(i, 0, n) {
        const FAtom s = At(const_cast<void*>(src), i);
        const FAtom d = AddDefault(dst);

        value_traits->DeepCopy(s.Data(), d.Data());
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
bool TBaseDicoTraits<_Key, _Value>::PromoteCopy(const void* src, const FAtom& dst) const {
    Assert(src);
    Assert(dst);

    if (*dst.Traits() == *this) {
        Copy(src, dst.Data());
        return true;
    }
    else if (const IDicoTraits* const dstDico = dst.Traits()->AsDico()) {
        const size_t n = Count(src);
        dstDico->Empty(dst.Data(), n);

        STACKLOCAL_ATOM(promotedKey, dstDico->KeyTraits());

        return ForEach(const_cast<void*>(src), [&promotedKey, &dst, dstDico](const FAtom& key, const FAtom& value) {
            if (not key.PromoteCopy(promotedKey))
                return false;

            const FAtom promotedValue = dstDico->AddDefault(dst.Data(), promotedKey);
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

    if (*dst.Traits() == *this) {
        Copy(src, dst.Data());
        return true;
    }
    else if (const IDicoTraits* const dstDico = dst.Traits()->AsDico()) {
        const size_t n = Count(src);
        dstDico->Empty(dst.Data(), n);

        STACKLOCAL_ATOM(promotedKey, dstDico->KeyTraits());

        const bool succeed = ForEach(src, [&promotedKey, &dst, dstDico](const FAtom& key, const FAtom& value) {
            if (not key.PromoteMove(promotedKey))
                return false;

            const FAtom promotedValue = dstDico->AddDefault(dst.Data(), promotedKey);
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
        const FAtom promotedValue = AddDefault(dst, promotedKey);
        value.DeepCopy(promotedValue);
        return true;
    });
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// TPairTraits<_First, _Second>
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
FAtom TPairTraits<_First, _Second>::First(void* data) const {
    Assert(data);

    return FAtom(&reinterpret_cast<value_type*>(data)->first, MakeTraits<_First>());
}
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
FAtom TPairTraits<_First, _Second>::Second(void* data) const {
    Assert(data);

    return FAtom(&reinterpret_cast<value_type*>(data)->second, MakeTraits<_Second>());
}
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
void TPairTraits<_First, _Second>::SetFirstCopy(void* data, const FAtom& other) const {
    Assert(data);
    Assert(other);
    Assert(other.Traits() == MakeTraits<_First>());

    MakeTraits<_First>()->Copy(
        other.Data(),
        &reinterpret_cast<value_type*>(data)->first );
}
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
void TPairTraits<_First, _Second>::SetFirstMove(void* data, const FAtom& rvalue) const {
    Assert(data);
    Assert(rvalue);
    Assert(rvalue.Traits() == MakeTraits<_First>());

    MakeTraits<_First>()->Move(
        rvalue.Data(),
        std::move(&reinterpret_cast<value_type*>(data)->first) );
}
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
void TPairTraits<_First, _Second>::SetSecondCopy(void* data, const FAtom& other) const {
    Assert(data);
    Assert(other);
    Assert(other.Traits() == MakeTraits<_Second>());

    MakeTraits<_Second>()->Copy(
        other.Data(),
        &reinterpret_cast<value_type*>(data)->second );
}
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
void TPairTraits<_First, _Second>::SetSecondMove(void* data, const FAtom& rvalue) const {
    Assert(data);
    Assert(rvalue);
    Assert(rvalue.Traits() == MakeTraits<_Second>());

    MakeTraits<_Second>()->Move(
        rvalue.Data(),
        &reinterpret_cast<value_type*>(data)->second );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// TVectorLikeTraits<_VectorLike>
//----------------------------------------------------------------------------
template <typename _VectorLike>
size_t TVectorLikeTraits<_VectorLike>::Count(const void* data) const {
    Assert(data);

    return reinterpret_cast<const value_type*>(data)->size();
}
//----------------------------------------------------------------------------
template <typename _VectorLike>
bool TVectorLikeTraits<_VectorLike>::IsEmpty(const void* data) const {
    Assert(data);

    return reinterpret_cast<const value_type*>(data)->empty();
}
//----------------------------------------------------------------------------
template <typename _VectorLike>
FAtom TVectorLikeTraits<_VectorLike>::At(void* data, size_t index) const {
    Assert(data);

    auto& item = reinterpret_cast<value_type*>(data)->at(index);
    return FAtom(&item, MakeTraits<item_type>());
}
//----------------------------------------------------------------------------
template <typename _VectorLike>
size_t TVectorLikeTraits<_VectorLike>::Find(const void* data, const FAtom& item) const {
    Assert(data);
    Assert(item);

    return IndexOf(*reinterpret_cast<const value_type*>(data), item.TypedData<item_type>());
}
//----------------------------------------------------------------------------
template <typename _VectorLike>
FAtom TVectorLikeTraits<_VectorLike>::AddDefault(void* data) const {
    Assert(data);

    auto& item = reinterpret_cast<value_type*>(data)->push_back_Default();
    return FAtom(&item, MakeTraits<item_type>());
}
//----------------------------------------------------------------------------
template <typename _VectorLike>
void TVectorLikeTraits<_VectorLike>::AddCopy(void* data, const FAtom& item) const {
    Assert(data);
    Assert(item);

    reinterpret_cast<value_type*>(data)->push_back(item.TypedConstData<item_type>());
}
//----------------------------------------------------------------------------
template <typename _VectorLike>
void TVectorLikeTraits<_VectorLike>::AddMove(void* data, const FAtom& item) const {
    Assert(data);
    Assert(item);

    reinterpret_cast<value_type*>(data)->push_back(std::move(item.TypedData<item_type>()));
}
//----------------------------------------------------------------------------
template <typename _VectorLike>
void TVectorLikeTraits<_VectorLike>::Erase(void* data, size_t index) const {
    Assert(data);

    value_type& v = *reinterpret_cast<value_type*>(data);
    v.erase(v.begin() + index);
}
//----------------------------------------------------------------------------
template <typename _VectorLike>
bool TVectorLikeTraits<_VectorLike>::Remove(void* data, const FAtom& item) const {
    Assert(data);
    Assert(item);

    return Remove_ReturnIfExists(
        *reinterpret_cast<value_type*>(data),
        item.TypedConstData<item_type>() );
}
//----------------------------------------------------------------------------
template <typename _VectorLike>
void TVectorLikeTraits<_VectorLike>::Reserve(void* data, size_t capacity) const {
    Assert(data);

    reinterpret_cast<value_type*>(data)->reserve(capacity);
}
//----------------------------------------------------------------------------
template <typename _VectorLike>
void TVectorLikeTraits<_VectorLike>::Clear(void* data) const {
    Assert(data);

    reinterpret_cast<value_type*>(data)->clear();
}
//----------------------------------------------------------------------------
template <typename _VectorLike>
void TVectorLikeTraits<_VectorLike>::Empty(void* data, size_t capacity) const {
    Assert(data);

    value_type& v = *reinterpret_cast<value_type*>(data);
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
bool TVectorLikeTraits<_VectorLike>::ForEach(void* data, const foreach_fun& foreach) const {
    Assert(data);

    const PTypeTraits value_traits = MakeTraits<item_type>();
    for (auto& elt : *reinterpret_cast<value_type*>(data)) {
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
FAtom TAssociativeVectorTraits<_Key, _Value, _EqualTo, _Vector>::AddDefault(void* data, FAtom&& rkey) const {
    Assert(data);

    value_type& d = (*reinterpret_cast<value_type*>(data));
    _Value& value = d.Add(std::move(rkey.TypedData<_Key>()));
    return FAtom(&value, MakeTraits<_Value>());
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
FAtom TAssociativeVectorTraits<_Key, _Value, _EqualTo, _Vector>::AddDefault(void* data, const FAtom& key) const {
    Assert(data);

    value_type& d = (*reinterpret_cast<value_type*>(data));
    _Value& value = d.Add(key.TypedConstData<_Key>());
    return FAtom(&value, MakeTraits<_Value>());
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
void TAssociativeVectorTraits<_Key, _Value, _EqualTo, _Vector>::AddCopy(void* data, const FAtom& key, const FAtom& value) const {
    Assert(data);

    reinterpret_cast<value_type*>(data)->Insert_AssertUnique(
        key.TypedConstData<_Key>(),
        value.TypedConstData<_Value>() );
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _EqualTo, typename _Vector>
void TAssociativeVectorTraits<_Key, _Value, _EqualTo, _Vector>::AddMove(void* data, const FAtom& key, const FAtom& value) const {
    Assert(data);

    reinterpret_cast<value_type*>(data)->Insert_AssertUnique(
        std::move(key.TypedData<_Key>()),
        std::move(value.TypedData<_Value>()) );
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
FAtom THashMapTraits<_Key, _Value, _Hasher, _EqualTo, _Allocator>::AddDefault(void* data, FAtom&& rkey) const {
    Assert(data);

    value_type& d = (*reinterpret_cast<value_type*>(data));
    _Value& v = d.Add(std::move(rkey.TypedData<_Key>()));
    return FAtom(&v, MakeTraits<_Value>());
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
FAtom THashMapTraits<_Key, _Value, _Hasher, _EqualTo, _Allocator>::AddDefault(void* data, const FAtom& key) const {
    Assert(data);

    value_type& d = (*reinterpret_cast<value_type*>(data));
    _Value& v = d.Add(key.TypedConstData<_Key>());
    return FAtom(&v, MakeTraits<_Value>());
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
void THashMapTraits<_Key, _Value, _Hasher, _EqualTo, _Allocator>::AddCopy(void* data, const FAtom& key, const FAtom& value) const {
    Assert(data);

    reinterpret_cast<value_type*>(data)->emplace_AssertUnique(
        key.TypedConstData<_Key>(),
        value.TypedConstData<_Value>() );
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value, typename _Hasher, typename _EqualTo, typename _Allocator>
void THashMapTraits<_Key, _Value, _Hasher, _EqualTo, _Allocator>::AddMove(void* data, const FAtom& key, const FAtom& value) const {
    Assert(data);

    reinterpret_cast<value_type*>(data)->emplace_AssertUnique(
        std::move(key.TypedData<_Key>()),
        std::move(value.TypedData<_Value>()) );
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
