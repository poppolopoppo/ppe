#pragma once

#include "Core/Meta/AlignedStorage.h"
#include "Core/Meta/TypeTraits.h"

#include <memory>

namespace Core {
namespace Meta {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
struct TInSituPtr {
#ifdef ARCH_X86
    STATIC_CONST_INTEGRAL(intptr_t, NullMagick, 0xDEADF001ul);
#else
    STATIC_CONST_INTEGRAL(intptr_t, NullMagick, 0xDEADF001DEADF001ull);
#endif
    union {
        POD_STORAGE(T) InSitu;
        intptr_t VTable = NullMagick;
    };

    bool Valid() const { return (VTable != NullMagick); }
    operator const void* () const { return (Valid() ? this : nullptr); }

    T* get() {
        Assert(Valid());
        return reinterpret_cast<T*>(std::addressof(InSitu));
    }
    const T* get() const { return const_cast<TInSituPtr*>(this)->get(); }

    T& operator *() { return *get(); }
    const T& operator *() const { return *get(); }

    T* operator ->() { return get(); }
    const T* operator ->() const { return get(); }

    template <typename U, typename... _Args>
    U* Create(_Args&&... args) {
        STATIC_ASSERT(std::is_base_of<T, U>::value);
        STATIC_ASSERT(sizeof(U) == sizeof(T));
        Assert(not Valid());
        U* const result = new ((void*)std::addressof(InSitu)) U{ std::forward<_Args>(args)... };
        Assert(Valid());
        return result;
    }

    void Destroy() {
        Assert(Valid());
        get()->~T();
        VTable = NullMagick
    }

    template <typename U, typename... _Args>
    static TInSituPtr Make(_Args&&... args) {
        TInSituPtr p;
        p.Create<U>(std::forward<_Args>(args)...);
        return p;
    }

    inline friend bool operator ==(const TInSituPtr& lhs, const TInSituPtr& rhs) {
        STATIC_ASSERT(sizeof(lhs) == sizeof(intptr_t));
        STATIC_ASSERT(sizeof(rhs) == sizeof(intptr_t));
        return (lhs.VTable == rhs.VTable);
    }
    inline friend bool operator !=(const TInSituPtr& lhs, const TInSituPtr& rhs) {
        return not operator ==(lhs, rhs);
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace Core
