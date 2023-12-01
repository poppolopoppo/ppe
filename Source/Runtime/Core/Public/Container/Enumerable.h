#pragma once

#include "Core_fwd.h"

#include "Meta/Iterator.h"

/**
 * Iterator type erasure for TIterable
 * ex:
 *      int Sum(TEnumerable<int>& seq) {
 *          int result = 0;
 *          while (seq.Next()) result += *seq;
 *          return result;
 *      }
 *      [...]
 *
 *      TVector<int> v{ 1, 2, 3, 4, 5 };
 *      auto odd = v.Iterable().FilterBy([](int x) { return !!(x&1); });
 *      int oddSum = Sum(odd);
 */

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class TEnumerable {
public:
    using value_type = T;
    using pointer = T*;
    using reference = T&;

    using FEnumerableFunc = void (*)(TEnumerable<T> self, bool (*each)(void* userData, reference it), void* userData);

    CONSTEXPR TEnumerable(void* internal, FEnumerableFunc enumerate) NOEXCEPT
    :   _internal(internal)
    ,   _enumerate(enumerate) {
        Assert_NoAssume(_internal);
        Assert_NoAssume(_enumerate);
    }

    template <typename _Allocator>
    CONSTEXPR TEnumerable(TVector<T, _Allocator>& vector)
    :   TEnumerable(const_cast<void*>(&vector), [](TEnumerable<T> self, bool (*each)(void* userData, reference it), void* userData){
        auto& view = (*static_cast<TVector<T, _Allocator>*>(self._internal));
        for (reference it : view) {
            if (not each(userData, it))
                return;
        }
    })
    {}

    template <typename _Allocator>
    CONSTEXPR TEnumerable(const TVector<Meta::TRemoveConst<T>, _Allocator>& vector)
    :   TEnumerable(const_cast<void*>(&vector), [](TEnumerable<T> self, bool (*each)(void* userData, reference it), void* userData){
        auto& view = (*static_cast<const TVector<Meta::TRemoveConst<T>, _Allocator>*>(self._internal));
        for (reference it : view) {
            if (not each(userData, it))
                return;
        }
    })
    {}

    CONSTEXPR TEnumerable(const TMemoryView<T>& view)
    :   TEnumerable(const_cast<void*>(&view), [](TEnumerable<T> self, bool (*each)(void* userData, reference it), void* userData){
        const auto& view = (*static_cast<const TMemoryView<T>*>(self._internal));
        for (reference it : view) {
            if (not each(userData, it))
                return;
        }
    })
    {}

    template <typename _It, Meta::TEnableIf<std::is_same_v<typename Meta::TIteratorTraits<_It>::value_type, value_type>>* = nullptr>
    CONSTEXPR TEnumerable(const TIterable<_It>& iterable) NOEXCEPT
    :   TEnumerable(const_cast<void*>(&iterable), [](TEnumerable<T> self, bool (*each)(void* userData, reference it), void* userData){
        const auto& iterable = (*static_cast<const TIterable<_It>*>(self._internal));
        for (reference it : iterable) {
            if (not each(userData, it))
                return;
        }
    })
    {}

    TEnumerable(const TEnumerable&) = default;
    TEnumerable& operator =(const TEnumerable&) = default;

    TEnumerable(TEnumerable&&) = default;
    TEnumerable& operator =(TEnumerable&&) = default;

    CONSTEXPR void Each(bool (*each)(void* userData, reference it), void* userData = nullptr) {
        Assert(_internal && _enumerate);
        _enumerate(*this, each, userData);
    }

    friend CONSTEXPR bool operator ==(const TEnumerable& lhs, const TEnumerable& rhs) {
        return (lhs._internal == rhs._internal && lhs._enumerate == rhs._enumerate);
    }
    friend CONSTEXPR bool operator !=(const TEnumerable& lhs, const TEnumerable& rhs) {
        return (not operator ==(lhs, rhs));
    }

    friend CONSTEXPR void swap(TEnumerable& lhs, TEnumerable& rhs) NOEXCEPT {
        std::swap(lhs._internal, rhs._internal);
        std::swap(lhs._enumerate, rhs._enumerate);
    }

private:
    void* _internal{ nullptr };
    FEnumerableFunc _enumerate{ nullptr };
};
//----------------------------------------------------------------------------
template <typename T>
TEnumerable(const TMemoryView<T>&) -> TEnumerable<Meta::TAddConst<T>>;
template <typename T, typename _Allocator>
TEnumerable(TVector<T, _Allocator>&) -> TEnumerable<T>;
template <typename T, typename _Allocator>
TEnumerable(const TVector<T, _Allocator>&) -> TEnumerable<Meta::TAddConst<T>>;
template <typename _It>
TEnumerable(const TIterable<_It>&) -> TEnumerable<typename TIterable<_It>::value_type>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
