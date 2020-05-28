#pragma once

#include "Core_fwd.h"

#include "Meta/AlignedStorage.h"
#include "Thread/AtomicSpinLock.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FInitSegAllocator : Meta::FNonCopyableNorMovable {
public:
    FInitSegAllocator();
    ~FInitSegAllocator();

    using deleter_f = void(*)(void*) NOEXCEPT;

    struct FAlloc {
        void* Data;
        FAlloc* Next;
        deleter_f Deleter;
        CONSTEXPR explicit FAlloc(void* data, deleter_f deleter) NOEXCEPT
        :   Data(data)
        ,   Next(nullptr)
        ,   Deleter(deleter)
        {}
    };

    template <typename T>
    struct TAlloc : FAlloc {
        POD_STORAGE(T) Storage;
        operator T& () NOEXCEPT {
            return (*reinterpret_cast<T*>(&Storage));
        }
        operator const T& () const NOEXCEPT {
            return (*reinterpret_cast<const T*>(&Storage));
        }
        TAlloc() NOEXCEPT
        :   FAlloc(&Storage, [](void* p) NOEXCEPT {
            Meta::Destroy(reinterpret_cast<T*>(p));
        }) {
            Meta::Construct(reinterpret_cast<T*>(&Storage));
            Get().Allocate(*this);
        }
    };

    void Allocate(FAlloc& alloc) NOEXCEPT;

    static FInitSegAllocator& Get() NOEXCEPT;

private:
    FAtomicOrderedLock _barrier;
    FAlloc* _head{ nullptr };
    FAlloc* _tail{ nullptr };
};
//----------------------------------------------------------------------------
template <typename T>
using TInitSegAlloc = typename FInitSegAllocator::TAlloc<T>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
