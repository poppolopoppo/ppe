#pragma once

#include "RTTI/NativeTypes.h"

#include "Container/Vector.h"

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Item>
static CONSTEXPR const PTypeInfos MakeListTypeInfos = []() CONSTEXPR NOEXCEPT -> FTypeInfos {
    return FTypeInfos::CombineTypes(
        FTypeId(ETypeFlags::List),
        FTypeInfos::BasicInfos<T>(ETypeFlags::List),
        MakeTypeInfos<_Item>()
    );
};
//----------------------------------------------------------------------------
// TBaseListTraits<T>
//----------------------------------------------------------------------------
template <typename T>
class TBaseListTraits : public IListTraits {
protected:
    using IListTraits::IListTraits;

public: // ITypeTraits
    virtual FStringView TypeName() const override final;

public: // IListTraits
    virtual PTypeTraits ValueTraits() const NOEXCEPT override final { return MakeTraits<T>(); }
};
//----------------------------------------------------------------------------
template <typename T>
FStringView TBaseListTraits<T>::TypeName() const {
    ONE_TIME_INITIALIZE(const FStringView, GTypeName, MakeListTypeName(
        MakeTraits<T>()
    ));
    return GTypeName;
}
//----------------------------------------------------------------------------
// TListTraits<T>
//----------------------------------------------------------------------------
template <typename _List, typename T>
class TListTraits : public TBaseTypeTraits< _List, TBaseListTraits<T> > {
protected:
    using base_traits = TBaseTypeTraits< _List, TBaseListTraits<T> >;

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
        return MakeCommonType<_List>(other);
    }

protected:
    static _List& Unwrap(void* data) NOEXCEPT {
        Assert(data);
        return (*static_cast<_List*>(data));
    }
    static const _List& Unwrap(const void* data) NOEXCEPT {
        Assert(data);
        return (*static_cast<const _List*>(data));
    }
};
//----------------------------------------------------------------------------
template <typename _List, typename T>
bool TListTraits<_List, T>::IsDefaultValue(const void* data) const NOEXCEPT {
    return Unwrap(data).empty();
}
//----------------------------------------------------------------------------
template <typename _List, typename T>
void TListTraits<_List, T>::ResetToDefaultValue(void* data) const {
    Unwrap(data).clear();
}
//----------------------------------------------------------------------------
template <typename _List, typename T>
bool TListTraits<_List, T>::Equals(const void* lhs, const void* rhs) const NOEXCEPT {
    const _List& a = Unwrap(lhs);
    const _List& b = Unwrap(rhs);

    if (a.size() != b.size())
        return false;

    const PTypeTraits item = MakeTraits<T>();

    return std::equal(
        a.begin(), a.end(),
        b.begin(), b.end(),
        [item](const T& it, const T& jt) NOEXCEPT{
            return item->Equals(&it, &jt);
        });
}
//----------------------------------------------------------------------------
template <typename _List, typename T>
hash_t TListTraits<_List, T>::HashValue(const void* data) const NOEXCEPT {
    Assert(data);

    hash_t h{ ITypeTraits::TypeId() };

    const PTypeTraits item = MakeTraits<T>();

    for (const auto& it : Unwrap(data))
        hash_combine(h, item->HashValue(&it));

    return h;
}
//----------------------------------------------------------------------------
template <typename _List, typename T>
bool TListTraits<_List, T>::PromoteCopy(const void* src, const FAtom& dst) const {
    Assert(src);
    Assert(dst);

    if (base_traits::BasePromoteCopy(src, dst))
        return true;

    if (const IListTraits* const plist = dst.Traits()->AsList()) {
        const _List& srcT = Unwrap(src);

        const PTypeTraits item = MakeTraits<T>();

        plist->Empty(dst.Data(), srcT.size());

        for (const auto& it : srcT) {
            const FAtom last = plist->AddDefault(dst.Data());

            if (not item->PromoteCopy(&it, last))
                return false;
        }

        return true;
    }

    return false;
}
//----------------------------------------------------------------------------
template <typename _List, typename T>
bool TListTraits<_List, T>::PromoteMove(void* src, const FAtom& dst) const NOEXCEPT {
    Assert(src);
    Assert(dst);

    if (base_traits::BasePromoteMove(src, dst))
        return true;

    if (const IListTraits* const plist = dst.Traits()->AsList()) {
        _List& srcT = Unwrap(src);

        const PTypeTraits item = MakeTraits<T>();

        plist->Empty(dst.Data(), srcT.size());

        for (auto& it : srcT) {
            const FAtom last = plist->AddDefault(dst.Data());
            if (not item->PromoteMove(&it, last))
                return false;
        }

        srcT.clear();

        return true;
    }

    return false;
}
//----------------------------------------------------------------------------
template <typename _List, typename T>
bool TListTraits<_List, T>::DeepEquals(const void* lhs, const void* rhs) const {
    const _List& a = Unwrap(lhs);
    const _List& b = Unwrap(rhs);

    if (a.size() != b.size())
        return false;

    const PTypeTraits item = MakeTraits<T>();

    return std::equal(
        a.begin(), a.end(),
        b.begin(), b.end(),
        [item](const T& it, const T& jt) NOEXCEPT{
            return item->DeepEquals(&it, &jt);
        });
}
//----------------------------------------------------------------------------
template <typename _List, typename T>
void TListTraits<_List, T>::DeepCopy(const void* src, void* dst) const {
    const _List& srcT = Unwrap(src);
    _List& dstT = Unwrap(dst);

    dstT.clear();
    dstT.reserve(srcT.size());

    const PTypeTraits item = MakeTraits<T>();

    typename _List::value_type cpy;
    for (const auto& it : srcT) {
        item->DeepCopy(&it, &cpy);
        dstT.push_back(std::move(cpy));
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// TVectorTraits<T, _Allocator>
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
class TVectorTraits final : public TListTraits< TVector<T, _Allocator>, T > {
    using item_type = T;
    using base_traits = TListTraits< TVector<T, _Allocator>, T >;
    using typename base_traits::value_type;
    using typename base_traits::pointer;
    using typename base_traits::const_pointer;

    using base_traits::Unwrap;

public: // IListTraits
    using typename base_traits::foreach_fun;

    using base_traits::base_traits;

    virtual size_t Count(const void* data) const NOEXCEPT override final;
    virtual bool IsEmpty(const void* data) const NOEXCEPT override final;

    virtual FAtom At(void* data, size_t index) const NOEXCEPT override final;
    virtual size_t Find(const void* data, const FAtom& item) const NOEXCEPT override final;

    virtual FAtom AddDefault(void* data) const override final;
    virtual void AddCopy(void* data, const FAtom& item) const override final;
    virtual void AddMove(void* data, const FAtom& item) const override final;
    virtual void Erase(void* data, size_t index) const override final;
    virtual bool Remove(void* data, const FAtom& item) const override final;

    virtual void Reserve(void* data, size_t capacity) const override final;
    virtual void Clear(void* data) const override final;
    virtual void Empty(void* data, size_t capacaity) const override final;

    virtual bool ForEach(void* data, const foreach_fun& foreach) const NOEXCEPT override final;
};
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
CONSTEXPR PTypeInfos RTTI_TypeInfos(TTypeTag< TVector<T, _Allocator> >) {
    return MakeListTypeInfos< TVector<T, _Allocator>, T >;
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
CONSTEXPR PTypeTraits RTTI_Traits(TTypeTag< TVector<T, _Allocator> >) {
    return MakeStaticType< TVectorTraits<T, _Allocator>, TVector<T, _Allocator> >();
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
size_t TVectorTraits<T, _Allocator>::Count(const void* data) const NOEXCEPT {
    Assert(data);

    return Unwrap(data).size();
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool TVectorTraits<T, _Allocator>::IsEmpty(const void* data) const NOEXCEPT {
    Assert(data);

    return Unwrap(data).empty();
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
FAtom TVectorTraits<T, _Allocator>::At(void* data, size_t index) const NOEXCEPT {
    Assert(data);

    auto& item = Unwrap(data).at(index);
    return FAtom(&item, MakeTraits<item_type>());
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
size_t TVectorTraits<T, _Allocator>::Find(const void* data, const FAtom& item) const NOEXCEPT {
    Assert(data);
    Assert(item);

    const PTypeTraits item_traits = MakeTraits<item_type>();
    Assert_NoAssume(item_traits->TypeId() == item.TypeId());

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
template <typename T, typename _Allocator>
FAtom TVectorTraits<T, _Allocator>::AddDefault(void* data) const {
    Assert(data);

    auto& item = Unwrap(data).push_back_Default();
    return FAtom(&item, MakeTraits<item_type>());
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TVectorTraits<T, _Allocator>::AddCopy(void* data, const FAtom& item) const {
    Assert(data);
    Assert(item);

    Unwrap(data).push_back(item.TypedConstData<item_type>());
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TVectorTraits<T, _Allocator>::AddMove(void* data, const FAtom& item) const {
    Assert(data);
    Assert(item);

    Unwrap(data).push_back(std::move(item.TypedData<item_type>()));
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TVectorTraits<T, _Allocator>::Erase(void* data, size_t index) const {
    Assert(data);

    value_type& v = Unwrap(data);
    v.erase(v.begin() + index);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool TVectorTraits<T, _Allocator>::Remove(void* data, const FAtom& item) const {
    Assert(data);
    Assert(item);

    const PTypeTraits item_traits = MakeTraits<item_type>();
    Assert_NoAssume(item_traits->TypeId() == item.TypeId());

    value_type& vector = Unwrap(data);

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
template <typename T, typename _Allocator>
void TVectorTraits<T, _Allocator>::Reserve(void* data, size_t capacity) const {
    Assert(data);

    Unwrap(data).reserve(capacity);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TVectorTraits<T, _Allocator>::Clear(void* data) const {
    Assert(data);

    Unwrap(data).clear();
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void TVectorTraits<T, _Allocator>::Empty(void* data, size_t capacity) const {
    Assert(data);

    value_type& v = Unwrap(data);
    if (capacity) {
        v.clear();
        v.reserve(capacity);
    }
    else {
        v.clear_ReleaseMemory();
    }
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool TVectorTraits<T, _Allocator>::ForEach(void* data, const foreach_fun& foreach) const NOEXCEPT {
    Assert(data);

    const PTypeTraits value_traits = MakeTraits<item_type>();
    for (auto& elt : Unwrap(data)) {
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
