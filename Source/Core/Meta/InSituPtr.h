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
    STATIC_CONST_INTEGRAL(size_t, NullMagick, 0xDEADF001);
#else
    STATIC_CONST_INTEGRAL(size_t, NullMagick, 0xDEADF001DEADF001);
#endif
    union {
        size_t Magick = NullMagick;
        POD_STORAGE(T) Inplace;
    };

    bool Valid() const { return (Magick != NullMagick); }
    operator void* () const { return (Valid() ? this : nullptr); }

    T* get() {
        Assert(Valid());
        return reinterpret_cast<T*>(std::addressof(Inplace));
    }
    const T* get() const { return const_cast<TInSituPtr>(this)->get(); }

    T& operator *() { return *get(); }
    const T& operator *() const { return *get(); }

    T* operator ->() { return get(); }
    const T* operator ->() const { return get(); }

    template <typename U, typename... _Args>
    TEnableIf< std::is_base_of<T, U>::value, U* > Create(_Args&&... args) {
        STATIC_ASSERT(sizeof(U) == sizeof(T));
        Assert(not Valid());
        U* const result = new ((void*)std::addressof(Inplace)) U{ std::forward<_Args>(args)... };
        Assert(Valid());
        return result;
    }

    void Destroy() {
        get()->~T();
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace Core
