#pragma once

#include "Core_fwd.h"

#include "Memory/MemoryView.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// packed memory view using an offset relative to this instead of a pointer
// - storing an offset instead of a pointer makes it twice smaller than TMemoryView<>
// - can be reallocated without having to update internal storage pointer
// - as a consequence it is also trivially serializable
//----------------------------------------------------------------------------
template <typename T>
struct TRelativeView {
    using value_type = T;
    using pointer = T*;
    using reference = T&;
    using difference_type = intptr_t;
    using memory_view = TMemoryView<T>;

    struct iterator { // custom iterator to work with offsets instead of pointers
        using iterator_category = std::random_access_iterator_tag;

        TPtrRef<const TRelativeView> owner;
        u32 index{ 0 };

        CONSTEXPR iterator& operator++() /* prefix */ { ++index; return *this; }
        CONSTEXPR iterator& operator--() /* prefix */ { --index; return *this; }

        CONSTEXPR iterator operator++(int) /* postfix */ { const iterator prev(*this); ++index; return prev; }
        CONSTEXPR iterator operator--(int) /* postfix */ { const iterator prev(*this); --index; return prev; }

        CONSTEXPR iterator& operator+=(difference_type n) { index += n; return *this; }
        CONSTEXPR iterator& operator-=(difference_type n) { index -= n; return *this; }

        NODISCARD CONSTEXPR iterator operator+(difference_type n) const { return iterator(owner, index + n); }
        NODISCARD CONSTEXPR iterator operator-(difference_type n) const { return iterator(owner, index - n); }

        NODISCARD CONSTEXPR reference operator*() const { return owner->at(index); }
        NODISCARD CONSTEXPR pointer operator->() const { return (&owner->at(index)); }

        NODISCARD CONSTEXPR reference operator[](difference_type n) const { return owner->at(index + n); }

        NODISCARD CONSTEXPR difference_type operator-(const iterator& other) const { return checked_cast<difference_type>(index - other.index); }

        NODISCARD CONSTEXPR bool operator==(const iterator& other) const { Assert(owner.get() == other.owner.get()); return (index == other.index); }
        NODISCARD CONSTEXPR bool operator!=(const iterator& other) const { return not operator ==(other); }

        NODISCARD CONSTEXPR bool operator< (const iterator& other) const { Assert(owner.get() == other.owner.get()); return (index < other.index); }
        NODISCARD CONSTEXPR bool operator>=(const iterator& other) const { return not operator < (other); }
    };

    i32 Offset{ 0 };
    u32 Count{ 0 };

    TRelativeView() = default;

    explicit TRelativeView(memory_view view) NOEXCEPT {
        reset(view);
    }

    //// disable copy/move for safety ///////////////////////////////
    TRelativeView(const TRelativeView& other) = delete;
    TRelativeView& operator =(const TRelativeView& other) = delete;

    TRelativeView(TRelativeView&& rvalue) = delete;
    TRelativeView& operator =(TRelativeView&& rvalue) = delete;
    /////////////////////////////////////////////////////////////////

    NODISCARD bool empty() const { return (Count == 0); }
    NODISCARD u32 size() const { return Count; }

    NODISCARD pointer data() const { return bit_cast<pointer>(bit_cast<intptr_t>(this) + static_cast<intptr_t>(Offset)); }
    NODISCARD reference at(size_t index) const {
        Assert_NoAssume(index < Count);
        return data()[index];
    }

    NODISCARD operator memory_view () const { return MakeView(); }
    NODISCARD memory_view MakeView() const { return { data(), Count }; }

    NODISCARD iterator begin() const { return { this, 0 }; }
    NODISCARD iterator end() const { return { this, Count }; }

    void reset() {
        Offset = 0;
        Count = 0;
    }
    void reset(TMemoryView<T> view) {
        Offset = (view.empty() ? 0 : checked_cast<i32>(bit_cast<intptr_t>(view.data()) - bit_cast<intptr_t>(this)));
        Count = checked_cast<u32>(view.size());
    }
};
//----------------------------------------------------------------------------
PPE_ASSUME_TEMPLATE_AS_POD(TRelativeView<T>, typename T)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
