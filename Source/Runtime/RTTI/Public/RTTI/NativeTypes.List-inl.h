#pragma once

#include "RTTI/NativeTypes.h"

#include "Container/AssociativeVector.h"
#include "Container/Vector.h"

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// TBaseListTraits<T>
//----------------------------------------------------------------------------
template <typename T>
class TBaseListTraits : public IListTraits {
protected:
    CONSTEXPR TBaseListTraits(size_t sizeInBytes)
        : IListTraits(
            MakeListTypeId(MakeTraits<T>()),
            MakeListTypeFlags(MakeTraits<T>()),
            sizeInBytes )
    {}

public: // ITypeTraits
    virtual FStringView TypeName() const override final;

    virtual bool IsDefaultValue(const void* data) const override final;
    virtual void ResetToDefaultValue(void* data) const override final;

    virtual bool Equals(const void* lhs, const void* rhs) const override final;
    virtual hash_t HashValue(const void* data) const override final;

    virtual bool DeepEquals(const void* lhs, const void* rhs) const override final;
    virtual void DeepCopy(const void* src, void* dst) const override final;

    virtual bool PromoteCopy(const void* src, const FAtom& dst) const override /*final*/;
    virtual bool PromoteMove(void* src, const FAtom& dst) const override /*final*/;

public: // IListTraits
    virtual PTypeTraits ValueTraits() const override final { return MakeTraits<T>(); }
};
//----------------------------------------------------------------------------
template <typename T>
FStringView TBaseListTraits<T>::TypeName() const {
    ONE_TIME_INITIALIZE(const FString, GCachedTypeName, MakeListTypeName(
        MakeTraits<T>()
    ));
    return GCachedTypeName.MakeView();
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
bool TBaseListTraits<T>::Equals(const void* lhs, const void* rhs) const {
    Assert(lhs);
    Assert(rhs);

    const size_t n = Count(lhs);
    if (Count(rhs) != n)
        return false;

    forrange(i, 0, n) {
        const FAtom lhsIt = At((void*)lhs, i);
        const FAtom rhsIt = At((void*)rhs, i);

        if (not lhsIt.Equals(rhsIt))
            return false;
    }

    return true;
}
//----------------------------------------------------------------------------
template <typename T>
hash_t TBaseListTraits<T>::HashValue(const void* data) const {
    Assert(data);

    hash_t h{ TypeId() };

    ForEach((void*)data, [&h](const FAtom& it) {
        hash_combine(h, it.HashValue());
        return true;
    });

    return h;
}
//----------------------------------------------------------------------------
template <typename T>
bool TBaseListTraits<T>::PromoteCopy(const void* src, const FAtom& dst) const {
    Assert(src);
    Assert(dst);

    if (const IListTraits* const dstList = dst.Traits()->AsList()) {
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

    if (const IListTraits* const dstList = dst.Traits()->AsList()) {
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
// TVectorLikeTraits<_VectorLike>
//----------------------------------------------------------------------------
template <typename _VectorLike>
class TVectorLikeTraits final : public TBaseTypeTraits< _VectorLike, TBaseListTraits<typename _VectorLike::value_type> > {
    using item_type = typename _VectorLike::value_type;
    using base_traits = TBaseTypeTraits< _VectorLike, TBaseListTraits<item_type> >;
    using typename base_traits::value_type;
    using typename base_traits::pointer;
    using typename base_traits::const_pointer;

public: // IListTraits
    using typename base_traits::foreach_fun;

    CONSTEXPR TVectorLikeTraits() : base_traits(sizeof(_VectorLike)) {}

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
PTypeTraits Traits(Meta::TType< TVector<T, _Allocator> >) noexcept {
    return PTypeTraits::Make< TVectorLikeTraits< TVector<T, _Allocator> > >();
}
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

    const PTypeTraits item_traits = MakeTraits<typename _VectorLike::value_type>();
    Assert(item_traits->TypeId() == item.TypeId());

    size_t i = 0;
    for (auto& val : *((value_type*)data)) {
        const FAtom it(&val, item_traits);

        if (it.Equals(item))
            return i;

        i++;
    }

    return INDEX_NONE;
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

    const PTypeTraits item_traits = MakeTraits<typename _VectorLike::value_type>();
    Assert(item_traits->TypeId() == item.TypeId());

    value_type& vector = (*reinterpret_cast<value_type*>(data));

    foreachitem(it, vector) {
        const FAtom value(std::addressof(*it), item_traits);

        if (value.Equals(item)) {
            vector.erase(it);
            return true;
        }
    }

    return false;
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
} //!namespace RTTI
} //!namespace PPE
