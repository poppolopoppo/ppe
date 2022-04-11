#pragma once

#include "Core.h"

#include "Memory/MemoryDomain.h"
#include "Memory/RefPtr.h"
#include "Thread/AtomicPool.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, class = TEnableIfRefCountable<T> >
struct TAtomicRefPtrPoolTraits {
    //STATIC_ASSERT(Meta::has_defined_v<FRefCountable::THasOnStrongRefCountReachZero, T>);

    static u32 IndexInPool(T* p) NOEXCEPT {
        return p->IndexInPool();
    }

    static void OnStrongRefCountReachZero(T* p) NOEXCEPT {
        p->OnStrongRefCountReachZero();
    }
};
//----------------------------------------------------------------------------
template <ARG0_IF_MEMORYDOMAINS(typename _Domain COMMA) typename T, size_t _Chunks = 1, typename _Traits = TAtomicRefPtrPoolTraits<T> >
class TAtomicRefPtrPool : TAtomicPool<TPtrRef<T>, _Chunks, u32> {
    using parent_type = TAtomicPool<TPtrRef<T>, _Chunks, u32>;
    using traits_type = _Traits;
public:
    using index_type = typename parent_type::index_type;
    using parent_type::Capacity;

    TAtomicRefPtrPool() = default;

    using parent_type::NumFreeBlocks;
    using parent_type::NumCreatedBlocks;
    using parent_type::NumCreatedAndFreeBlocks;

    template <typename... _Args>
    NODISCARD TRefPtr<T> Allocate(_Args&&... args) NOEXCEPT {
        const TPtrRef<T> p = *parent_type::template Allocate([&](TPtrRef<T>* p, index_type id) {
#if USE_PPE_MEMORYDOMAINS
            TRefPtr<T> refPtr = NewRef<T>(_Domain::TrackingData(), std::forward<_Args>(args)..., id);
#else
            TRefPtr<T> refPtr = NewRef<T>(std::forward<_Args>(args)..., id);
#endif
            *p = RemoveRef_AssertReachZero_KeepAlive(refPtr);
#if USE_PPE_SAFEPTR
            AddSafeRef(p->get());
#endif
            Assert_NoAssume(traits_type::IndexInPool(p->get()) == id);
        });
        Assert_NoAssume(p->RefCount() == 0);
        return TRefPtr<T>{ p.get() };
    }

    // with this variant you *NEED* to keep track of the block id inside the allocated block
    void Release(TPtrRef<T> p) NOEXCEPT {
        Assert(p);
        Assert_NoAssume(p->RefCount() == 0);
        const index_type id = traits_type::IndexInPool(p);
        TPtrRef<T>* const pp = parent_type::BlockAddress(id);
        Assert_NoAssume(pp->get() == p.get());
        parent_type::Release(pp);
    }

    // will trigger standard delete upon clear
    void Clear_ReleaseMemory() {
        parent_type::Clear_ReleaseMemory([](TPtrRef<T>* p, index_type id) {
            Assert(*p);
            Assert_NoAssume((*p)->RefCount() == 0);
            Assert_NoAssume(traits_type::IndexInPool(p->get()) == id);
            UNUSED(id);
#if USE_PPE_SAFEPTR
            RemoveSafeRef(p->get());
#endif
            PPE::OnStrongRefCountReachZero(p->get()); // standard delete, by-pass recycling
            p->reset();
        });
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
