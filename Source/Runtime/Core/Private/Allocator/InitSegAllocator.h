#pragma once

#include "Core_fwd.h"

#include "Diagnostic/Logger.h"
#include "Meta/AlignedStorage.h"
#include "Meta/InPlace.h"
#include "Thread/AtomicSpinLock.h"

namespace PPE {
EXTERN_LOG_CATEGORY(PPE_CORE_API, InitSeg)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FInitSegAllocator : Meta::FNonCopyableNorMovable {
    class FAlloc;
    static FInitSegAllocator GInitSegAllocator_;

    FInitSegAllocator();

    static NO_INLINE void Allocate(FAlloc& alloc) NOEXCEPT;
    static NO_INLINE void Deallocate(FAlloc& alloc) NOEXCEPT;

    using deleter_f = void(*)(FAlloc&) NOEXCEPT;

    class FAlloc : FNonCopyableNorMovable {
    public:
        deleter_f const Deleter;
        FAlloc* Next{ nullptr };

        explicit FAlloc(deleter_f deleter) NOEXCEPT
        :   Deleter(deleter) {
            Assert_NoAssume(deleter);
            Allocate(*this);
        }
    };

public:
    ~FInitSegAllocator();

    template <typename T>
    class TAlloc : FAlloc {
        friend class FInitSegAllocator;
        POD_STORAGE(T) _data;
    public:

        Meta::TRemoveConst<T>* Get() NOEXCEPT {
            return reinterpret_cast<Meta::TRemoveConst<T>*>(&_data);
        }

        operator T& () NOEXCEPT { return (*Get()); }
        operator Meta::TAddConst<T>& () const NOEXCEPT { return (*Get()); }

        static void Destroy(FAlloc& alloc) NOEXCEPT {
            Meta::Destroy(static_cast<TAlloc<T>&>(alloc).Get());
        }

        template <typename... _Args>
        TAlloc(_Args&&... args) NOEXCEPT
        :   FAlloc(&Destroy) {
            Meta::Construct(Get(), std::forward<_Args>(args)...);
        }
    };

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
