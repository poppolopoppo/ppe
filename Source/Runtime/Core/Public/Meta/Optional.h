#pragma once

#include "Meta/Assert.h"
#include "Meta/TypeTraits.h"

#if _HAS_CXX17
#   include <optional>
#endif

namespace PPE {
namespace Meta {
//---------------------------------------------------------------------------- 
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// TODO: use C++17 std::optional when available
#if _HAS_CXX17
template <typename T>
using TOptional = std::optional<T>;
#else
template <typename T, typename = TEnableIf< TIsPod<T>::value > >
struct TOptional {
    TOptional() = default;

    TOptional(const TOptional&) = default;
    TOptional& operator =(const TOptional&) = default;

    TOptional(TOptional&&) = default;
    TOptional& operator =(TOptional&&) = default;

    TOptional(T value) { operator =(value); }
    TOptional& operator =(T value) { emplace(value); return (*this); }

    T operator *() const { Assert(_hasValue); return *data_(); }
    const T* operator ->() const { Assert(_hasValue); return data_(); }

    operator bool() const { return _hasValue; }

    bool has_value() const { return _hasValue; }
    void reset() { _hasValue = false; }
    void emplace(T value) {
        _hasValue = true;
        *data_() = value;
    }

    void swap(TOptional& other) {
        std::swap(_hasValue);
        std::swap(*data_(), *other.data_());
    }

private:
    typename std::aligned_storage<sizeof(T), alignof(T)>::type _data;
    bool _hasValue = false;

    FORCE_INLINE T* data_() { return reinterpret_cast<T*>(&_data); }
    FORCE_INLINE const T* data_() const { return reinterpret_cast<const T*>(&_data); }
};
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace PPE

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
hash_t hash_value(const Meta::TOptional<T>& optional) {
    STATIC_ASSERT(Meta::TIsPod<T>::value);
    return hash_as_pod(*optional);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE