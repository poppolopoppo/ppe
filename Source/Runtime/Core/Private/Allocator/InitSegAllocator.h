#pragma once

#include "Core_fwd.h"

#include "Diagnostic/Logger.h"
#include "Meta/AlignedStorage.h"
#include "Meta/TypeHash.h"
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

    NO_INLINE static void Allocate(FAlloc& alloc) NOEXCEPT;

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

        Meta::TRemoveConst<T>* get() NOEXCEPT {
            return (Meta::TRemoveConst<T>*)(&_data);
        }
        operator T& () NOEXCEPT {
            return (*reinterpret_cast<T*>(&_data));
        }
        operator Meta::TAddConst<T>& () const NOEXCEPT {
            return (*reinterpret_cast<Meta::TAddConst<T>*>(&_data));
        }

        static void Destroy(FAlloc& alloc) NOEXCEPT {
            Meta::Destroy(static_cast<TAlloc<T>&>(alloc).get());
        }

        template <typename... _Args>
        TAlloc(_Args&&... args) NOEXCEPT
        :   FAlloc(&Destroy) {
            Assert_NoAssume(Meta::IsAligned(alignof(T), get()));
            Meta::Construct(get(), std::forward<_Args>(args)...);
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
