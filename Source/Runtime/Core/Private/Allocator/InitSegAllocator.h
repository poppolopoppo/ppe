#pragma once

#include "Core_fwd.h"

#include "Meta/AlignedStorage.h"
#include "Meta/Assert.h"
#include "Meta/TypeInfo.h"
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

    static PPE_CORE_API NO_INLINE void Allocate(FAlloc& alloc) NOEXCEPT;

    using deleter_f = void(*)(FAlloc&) NOEXCEPT;

    class FAlloc : FNonCopyableNorMovable {
    public:
        const size_t Priority;
        const deleter_f Deleter;
        FAlloc* Next{ nullptr };
#if USE_PPE_PLATFORM_DEBUG
        const FConstChar DebugName;
        FAlloc(const size_t priority, deleter_f deleter, FConstChar debugName) NOEXCEPT
        :   Priority(priority)
        ,   Deleter(deleter)
        ,   DebugName(debugName) {
#else
        FAlloc(const size_t priority, deleter_f deleter) NOEXCEPT
        :   Priority(priority)
        ,   Deleter(deleter) {
#endif
            Assert_NoAssume(Priority);
            Assert_NoAssume(Deleter);
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
            return reinterpret_cast<Meta::TRemoveConst<T>*>(std::addressof(_data));
        }

        operator T& () NOEXCEPT { return (*Get()); }
        operator Meta::TAddConst<T>& () const NOEXCEPT { return (*Get()); }

        static void Destroy(FAlloc& alloc) NOEXCEPT {
            Meta::Destroy(static_cast<TAlloc<T>&>(alloc).Get());
        }

        template <typename... _Args>
        TAlloc(size_t priority, _Args&&... args) NOEXCEPT
#if USE_PPE_PLATFORM_DEBUG
        :   FAlloc(priority, &Destroy, Meta::type_info<Meta::TDecay<T>>.name) {
#else
        :   FAlloc(priority, &Destroy) {
#endif
            Meta::Construct(Get(), std::forward<_Args>(args)...);
        }
    };

private:
    FAtomicOrderedLock _barrier;

    FAlloc* _head{ nullptr };

#if USE_PPE_PLATFORM_DEBUG
    size_t _debugInitOrder{ 0 };
#endif
};
//----------------------------------------------------------------------------
template <typename T>
using TInitSegAlloc = typename FInitSegAllocator::TAlloc<T>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
