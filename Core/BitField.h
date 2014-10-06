#pragma once

#include "Cast.h"

#include <type_traits>

namespace Core {
namespace Meta {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
struct BitCount {
    enum : size_t { value = (sizeof(T) << 3) };
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Index, size_t _Count>
struct BitField {
    static_assert(_Count > 0, "_Count must be superior to 0");
    static_assert(_Index + _Count <= BitCount<T>::value, "overflow");

    enum : T {
        Index = _Index,
        Count = _Count,
        End = _Index + _Count,
        MaxValue = ((T(1) << _Count) - 1),
        Mask = (MaxValue << _Index),
    };

    FORCE_INLINE static T Clear(T value) { return (value & ~Mask); }
    FORCE_INLINE static T Format(T value) { return ((value << Index) & Mask); }

    static T Set(T flags, T value);
    static T Get(T flags);

    static T Inc(T flags, T value);

    FORCE_INLINE static void InplaceSet(T& flags, T value) { flags = Set(flags, value); }

    template <typename U>
    static T Set(T flags, U value) { return Set(flags, checked_cast<T>(value)); }
    template <typename U>
    static U Get(T flags) { return checked_cast<U>(Get(flags)); }

    template <typename U>
    static void InplaceSet(T& flags, U value) { flags = Set<U>(flags, value); }
    template <typename U>
    static void InplaceBitAnd(T& flags, U value) { flags = Set<U>(flags, (Get<U>(flags) & value)); }
    template <typename U>
    static void InplaceBitOr(T& flags, U value) { flags = Set<U>(flags, (Get<U>(flags) | value)); }
    template <typename U>
    static void InplaceBitXor(T& flags, U value) { flags = Set<U>(flags, (Get<U>(flags) ^ value)); }
};
//----------------------------------------------------------------------------
template <typename T, size_t _Index, size_t _Count>
FORCE_INLINE T BitField<T, _Index, _Count>::Set(T flags, T value) {
    Assert(Get(Format(value)) == value);
    return (Clear(flags) | Format(value));
}
//----------------------------------------------------------------------------
template <typename T, size_t _Index, size_t _Count>
FORCE_INLINE T BitField<T, _Index, _Count>::Get(T flags) {
    return ((flags & Mask) >> Index);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Index, size_t _Count>
FORCE_INLINE T BitField<T, _Index, _Count>::Inc(T flags, T value) {
    Assert(Get(Format(value)) == value);
    return (Clear(flags) | Format(Get(flags) + value));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Index>
struct BitField<T, _Index, 1> {
    static_assert(_Index + 1 <= BitCount<T>::value, "overflow");

    enum : T {
        Index = _Index,
        Count = 1,
        End = _Index + 1,
        MaxValue = 1,
        Mask = (MaxValue << _Index),
    };

    FORCE_INLINE static T Clear(T value) { return (value & ~Mask); }
    FORCE_INLINE static T Format(bool value) { return ((T)value & 1) << Index; }

    static T Set(T flags, bool value);
    static bool Get(T flags);

    FORCE_INLINE static void InplaceSet(T& flags, bool value) { flags = Set(flags, value); }

    FORCE_INLINE static T True(T flags) { return flags | Mask; }
    FORCE_INLINE static T False(T flags) { return flags & ~Mask; }

    FORCE_INLINE static void InplaceTrue(T& flags) { flags |= Mask; }
    FORCE_INLINE static void InplaceFalse(T& flags) { flags &= ~Mask; }
};
//----------------------------------------------------------------------------
template <typename T, size_t _Index>
FORCE_INLINE T BitField<T, _Index, 1>::Set(T flags, bool value) {
    Assert(Get(Format(value)) == value);
    return (Clear(flags) | Format(value));
}
//----------------------------------------------------------------------------
template <typename T, size_t _Index>
FORCE_INLINE bool BitField<T, _Index, 1>::Get(T flags) {
    return ((flags & Mask) ? true : false);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
struct Bit {
    enum { Capacity = BitCount<T>::value };

    template <size_t _Index, typename _Field>
    struct Field {
        typedef BitField<T, _Index, BitCount<_Field>::value> type;
    };

    template <size_t _Count>
    struct First {
        typedef BitField<T, 0, _Count> type;
    };

    template <typename _BitField>
    struct After {
        template <size_t _Count>
        struct Field {
            typedef BitField<T, _BitField::End, _Count> type;
        };
        struct Remain {
            typedef BitField<T, _BitField::End, Capacity - _BitField::End> type;
        };
    };
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace Core
