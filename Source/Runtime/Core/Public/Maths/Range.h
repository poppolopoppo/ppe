#pragma once

#include "Core_fwd.h"

#include "Maths/MathHelpers.h"
#include "Memory/MemoryView.h"
#include "Meta/Iterator.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
struct TRange;
using FRange32i = TRange<i32>;
using FRange32u = TRange<u32>;
using FRange32f = TRange<float>;
using FBytesRange = TRange<size_t>;
//----------------------------------------------------------------------------
template <typename T>
struct TRange {
    using value_type = T;
    STATIC_ASSERT(std::is_arithmetic_v<value_type>);

    STATIC_CONST_INTEGRAL(value_type, MaxValue, TNumericLimits<value_type>::MaxValue());
    STATIC_CONST_INTEGRAL(value_type, MinValue, TNumericLimits<value_type>::Lowest());
    STATIC_CONST_INTEGRAL(value_type, ZeroValue, TNumericLimits<value_type>::Zero());

    value_type First{ MaxValue };
    value_type Last{ MinValue };

    TRange() = default;

    TRange(const TRange&) = default;
    TRange& operator =(const TRange&) = default;

    TRange(TRange&&) = default;
    TRange& operator =(TRange&&) = default;

    CONSTEXPR TRange(value_type first, value_type last) : First(first), Last(last) {}

    CONSTEXPR static TRange Zero() { return { ZeroValue, ZeroValue }; }

    CONSTEXPR value_type Extent() const { return (Last - First); }
    CONSTEXPR bool Empty() const { return not Any(First < Last); }
    CONSTEXPR bool Whole() const { return All(First == 0) && All(Last == MaxValue); }

    NODISCARD CONSTEXPR bool Contains(const TRange& other) const {
        return All((First <= other.First) && (other.Last <= Last));
    }

    NODISCARD CONSTEXPR bool Overlaps(const TRange& other) const {
        return (not All((Last < other.First) || (other.Last < First)));
    }

    CONSTEXPR void Reset() {
        *this = TRange{};
        Assert_NoAssume(Empty());
    }

    CONSTEXPR TRange& Add(value_type value) {
        First = Blend(First + value, MaxValue, not value or First < First + value);
        Last = Blend(Last + value, MaxValue, not value or Last < Last + value);
        return (*this);
    }

    CONSTEXPR TRange& Append(const TRange& other) {
        if (Empty())
            *this = other;
        else
            SelfUnion(other);
        return (*this);
    }

    NODISCARD CONSTEXPR TRange operator +(T value) const { return (TRange(*this) += value); }
    CONSTEXPR TRange& operator +=(T value) { return Add(value); }

    NODISCARD CONSTEXPR TRange Union(const TRange& other) const { return TRange(*this).SelfUnion(other); }
    CONSTEXPR TRange& SelfUnion(const TRange& other) {
        //Assert(Overlaps(other));
        First = Min(First, other.First);
        Last = Max(Last, other.Last);
        return (*this);
    }

    NODISCARD CONSTEXPR TRange Intersect(const TRange& other) const { return TRange(*this).SelfIntersect(other); }
    CONSTEXPR TRange& SelfIntersect(const TRange& other) {
        First = Max(First, other.First);
        Last = Min(Last, other.Last);
        if (Empty()) *this = TRange{};
        return (*this);
    }

    CONSTEXPR static bool Overlaps(T firstA, T lastA, T firstB, T lastB) {
        return TRange{ firstA, lastA }.Overlaps({ firstB, lastB });
    }

    template <typename U, Meta::TEnableIf<std::is_integral_v<T>>* = nullptr>
    CONSTEXPR TMemoryView<U> MakeView(TMemoryView<U> view) const {
        return view.SubRange(checked_cast<size_t>(First), checked_cast<size_t>(Extent()));
    }

    CONSTEXPR bool operator ==(const TRange& other) const { return (All((First == other.First) && (Last == other.Last))); }
    CONSTEXPR bool operator !=(const TRange& other) const { return (not operator ==(other)); }

    CONSTEXPR friend hash_t hash_value(const TRange& range) {
        return hash_size_t_constexpr(range.First, range.Last);
    }

    struct FIterator : Meta::TIterator<value_type, std::random_access_iterator_tag> {
        using parent_type = Meta::TIterator<value_type, std::random_access_iterator_tag>;

        using typename parent_type::iterator_category;
        using typename parent_type::value_type;
        using typename parent_type::pointer;
        using typename parent_type::reference;

        using difference_type = decltype(std::declval<T>() - std::declval<T>());

        FIterator() = default;

        CONSTEXPR FIterator(const FIterator& ) = default;
        CONSTEXPR FIterator& operator =(const FIterator& ) = default;

        CONSTEXPR FIterator(FIterator&& ) = default;
        CONSTEXPR FIterator& operator =(FIterator&& ) = default;

        CONSTEXPR value_type operator *() const { return _value; }

        CONSTEXPR FIterator& Increment(difference_type n = value_type(1)) {
            _value = Blend( // check for overflow
                Min(_value + _increment * n, _owner->Last),
                _owner->Last, _value < _value + _increment * n);
            return (*this);
        }
        CONSTEXPR FIterator& Decrement(difference_type n = value_type(1)) {
            _value = Blend( // check of underflow
                Max(_value - _increment * n, _owner->First),
                _owner->First, _value - _increment * n < _value );
            return (*this);
        }

        CONSTEXPR FIterator& operator ++() /* prefix */ { return Increment(); }
        CONSTEXPR FIterator& operator --() /* prefix */ { return Decrement(); }

        CONSTEXPR FIterator operator ++(int) /* postfix */ { auto tmp{*this}; Increment(); return tmp; }
        CONSTEXPR FIterator operator --(int) /* postfix */ { auto tmp{*this}; Decrement(); return tmp; }

        CONSTEXPR FIterator& operator +=(difference_type n) { Increment(n); return (*this); }
        CONSTEXPR FIterator& operator -=(difference_type n) { Decrement(n); return (*this); }

        CONSTEXPR FIterator operator +(difference_type n) { return FIterator{*this}.Increment(n); }
        CONSTEXPR FIterator operator -(difference_type n) { return FIterator{*this}.Decrement(n); }

        CONSTEXPR value_type operator [](difference_type n) { return *(*this + n); }

        bool operator ==(const FIterator& other) const { Assert(_owner == other._owner); return All(_value == other._value); }
        bool operator !=(const FIterator& other) const { return (not operator ==(other)); }

        bool operator < (const FIterator& other) const { Assert(_owner == other._owner); return All(_value <  other._value); }
        bool operator >=(const FIterator& other) const { return (not operator < (other)); }

        bool operator <=(const FIterator& other) const { Assert(_owner == other._owner); return All(_value <= other._value); }
        bool operator > (const FIterator& other) const { return (not operator < (other)); }

    private:
        const TRange* _owner;
        value_type _value;
        value_type _increment;

        friend struct TRange;
        CONSTEXPR FIterator(const TRange* owner, value_type value, value_type increment = value_type(1))
        :   _owner(owner)
        ,   _value(value)
        ,   _increment(increment) {
            Assert(_owner);
            Assert(!(_value < _owner->First));
            Assert(!(_owner->Last < _value));
        }
    };

    CONSTEXPR FIterator begin() const { return FIterator(this, First); }
    CONSTEXPR FIterator end() const { return FIterator(this, Last); }

    CONSTEXPR TIterable<FIterator> Each(value_type increment = value_type(1)) const {
        return MakeIterable(
            FIterator{ this, First, increment },
            FIterator{ this, Last, increment } );
    }

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
