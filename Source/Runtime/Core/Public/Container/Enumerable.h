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
    typedef T value_type;

    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;

    using FConsumeFunc = void (*)(void*, pointer*) NOEXCEPT;

    TEnumerable() = default;

    CONSTEXPR TEnumerable(void* userData, FConsumeFunc consume)
    :   _userData(userData)
    ,   _consume(consume) {
        Assert_NoAssume(userData);
        Assert_NoAssume(_consume);
    }

    template <typename _It, class = Meta::TEnableIf<
        std::is_same_v<value_type, typename TIterable<_It>::value_type> > >
    CONSTEXPR TEnumerable(TIterable<_It>& iterable)
    :   _userData(std::addressof(iterable))
    ,   _consume([](void* userData, pointer* current) NOEXCEPT {
        return static_cast<TIterable<_It>*>(userData)->Consume(current);
    })
    {}

    TEnumerable(const TEnumerable&) = default;
    TEnumerable& operator =(const TEnumerable&) = default;

    TEnumerable(TEnumerable&&) = default;
    TEnumerable& operator =(TEnumerable&&) = default;

    CONSTEXPR void* UserData() const { return _userData; }

    CONSTEXPR reference Get() const {
        Assert(_current);
        return _current;
    }
    CONSTEXPR pointer GetIFP() const {
        return _current;
    }

    CONSTEXPR reference operator *() const { return Get(); }
    CONSTEXPR pointer operator ->() const { return std::addressof(Get()); }

    CONSTEXPR bool Next() NOEXCEPT {
        return _consume(_userData, &_current);
    }

    friend CONSTEXPR bool operator ==(const TEnumerable& lhs, const TEnumerable& rhs) {
        return (lhs._userData == rhs._userData && lhs._consume == rhs._consume);
    }
    friend CONSTEXPR bool operator !=(const TEnumerable& lhs, const TEnumerable& rhs) {
        return (not operator ==(lhs, rhs));
    }

    friend CONSTEXPR void swap(TEnumerable& lhs, TEnumerable& rhs) NOEXCEPT {
        std::swap(lhs._userData, rhs._userData);
        std::swap(lhs._consume, rhs._consume);
    }

private:
    pointer* _current{ nullptr };
    void* _userData;
    FConsumeFunc _consume;
};
//----------------------------------------------------------------------------
template <typename _It>
TEnumerable< typename TIterable<_It>::value_type > MakeEnumerable(TIterable<_It>& iterable) {
    return { iterable };
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
