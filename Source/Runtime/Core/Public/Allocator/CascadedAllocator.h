#pragma once

#include "Core_fwd.h"

#include "Allocator/AllocatorBase.h"
#include "Container/IntrusiveList.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Allocate a new _Cascade with _Batch when allocation fails
//----------------------------------------------------------------------------
template <typename _Cascade, typename _Batch>
class TCascadedAllocator : private _Batch {
public:
    using cascade_traits = TAllocatorTraits<_Cascade>;
    using batch_traits = TAllocatorTraits<_Batch>;

#define CASCADED_USING_DEF(_NAME) \
    using _NAME = typename batch_traits::_NAME

    using propagate_on_container_copy_assignment = std::false_type;
    CASCADED_USING_DEF(propagate_on_container_move_assignment);
    CASCADED_USING_DEF(propagate_on_container_swap);

    CASCADED_USING_DEF(is_always_equal);

    using has_maxsize = std::bool_constant<
        batch_traits::has_maxsize::value ||
        cascade_traits::has_maxsize::value >;

    CASCADED_USING_DEF(has_owns);
    CASCADED_USING_DEF(has_reallocate);
    CASCADED_USING_DEF(has_acquire);
    CASCADED_USING_DEF(has_steal);

#undef CASCADED_USING_DEF

    STATIC_ASSERT(cascade_traits::has_owns::value); // needed for Deallocate()
    STATIC_CONST_INTEGRAL(size_t, Alignment, batch_traits::Alignment);

    TCascadedAllocator() = default;
    ~TCascadedAllocator() {
        ReleaseAllCascades();
    }

    explicit TCascadedAllocator(const _Batch& batch)
    :   _Batch(batch)
    {}
    explicit TCascadedAllocator(_Batch&& rbatch)
    :   _Batch(std::move(rbatch))
    {}

    // copy will keep owned memory intact
    TCascadedAllocator(const TCascadedAllocator& other)
    :   _Batch(batch_traits::SelectOnCopy(other))
    {}
    TCascadedAllocator& operator =(const TCascadedAllocator& other) {
        batch_traits::Copy(this, other);
        return (*this);
    }

    // move will steal owned memory
    TCascadedAllocator(TCascadedAllocator&& rvalue)
    :   _Batch(batch_traits::SelectOnMove(std::move(rvalue)))
    ,   Cascades(rvalue.Cascades) {
        rvalue.Cascades = nullptr;
        batch_traits::Move(this, std::move(rvalue));
    }
    TCascadedAllocator& operator =(TCascadedAllocator&& rvalue) {
        if (Cascades)
            ReleaseAllCascades();
        Cascades = rvalue.Cascades;
        rvalue.Cascades = nullptr;
        batch_traits::Move(this, std::move(rvalue));
        return (*this);
    }

    static size_t SnapSize(size_t s) NOEXCEPT {
        return cascade_traits::SnapSize(s);
    }

    static size_t MaxSize() NOEXCEPT {
        return Min(batch_traits::MaxSize(), cascade_traits::MaxSize());
    }

    bool Owns(FAllocatorBlock b) const NOEXCEPT {
        for (FCascade* c = Cascades; c; c = c->Node.Next) {
            if (cascade_traits::Owns(c->Alloc, b))
                return true;
        }
        return false;
    }

    FAllocatorBlock Allocate(size_t s) {
        for (FCascade* c = Cascades; c; c = c->Node.Next) {
            const FAllocatorBlock b = cascade_traits::Allocate(c->Alloc, s);
            if (Likely(b)) {
                list_traits::PokeHead(&Cascades, nullptr, c);
                return b;
            }
        }
        return cascade_traits::Allocate(NewCascade()->Alloc, s);
    }

    void Deallocate(FAllocatorBlock b) {
        for (FCascade* c = Cascades; c; c = c->Node.Next) {
            if (cascade_traits::Owns(c->Alloc, b)) {
                list_traits::PokeHead(&Cascades, nullptr, c);
                cascade_traits::Deallocate(c->Alloc, b);
                return;
            }
        }
        AssertNotReached();
    }

    void Reallocate(FAllocatorBlock& b, size_t s) {
        for (FCascade* c = Cascades; c; c = c->Node.Next) {
            if (cascade_traits::Owns(c->Alloc, b)) {
                cascade_traits::Reallocate(c->Alloc, b, s);
                return;
            }
        }
        AssertNotReached();
    }

    bool Acquire(FAllocatorBlock b) NOEXCEPT {
        for (FCascade* c = Cascades; c; c = c->Node.Next) {
            if (cascade_traits::Acquire(c->Alloc, b))
                return true;
        }
        return false;
    }

    bool Steal(FAllocatorBlock b) NOEXCEPT {
        for (FCascade* c = Cascades; c; c = c->Node.Next) {
            if (cascade_traits::Owns(c->Alloc, b))
                return cascade_traits::Steal(c->Alloc, b);
        }
        AssertNotReached();
    }

    friend bool operator ==(const TCascadedAllocator& lhs, TCascadedAllocator& rhs) NOEXCEPT {
        return batch_traits::Equals(lhs, rhs);
    }

    friend bool operator !=(const TCascadedAllocator& lhs, TCascadedAllocator& rhs) NOEXCEPT {
        return (not operator ==(lhs, rhs));
    }

    friend void swap(TCascadedAllocator& lhs, TCascadedAllocator& rhs) NOEXCEPT {
        IF_CONSTEXPR(propagate_on_container_swap::value) {
            std::swap(lhs.Cascades, rhs.Cascades);
            batch_traits::Swap(lhs, rhs);
        }
    }

protected: // cascades
    struct FCascade {
        _Cascade Alloc;
        TIntrusiveListNode<FCascade> Node;
    };

    FCascade* Cascades{ nullptr };

    typedef details::TIntrusiveListAccessor<FCascade, &FCascade::Node> list_traits;

    FCascade* NewCascade() {
        FCascade* const c = INPLACE_NEW(
            batch_traits::Allocate(*this, sizeof(FCascade)).Data,
            FCascade) {};
        list_traits::PushHead(&Cascades, nullptr, c);
        return c;
    }

    void ReleaseCascade(FCascade* c) {
        list_traits::Erase(&Cascades, nullptr, c);
        Meta::Destroy(c);
        batch_traits::Deallocate(*this, FAllocatorBlock{ c, sizeof(FCascade) });
    }

    void ReleaseAllCascades() {
        while (FCascade * const c = list_traits::PopHead(&Cascades, nullptr)) {
            Meta::Destroy(c);
            batch_traits::Deallocate(*this, FAllocatorBlock{ c, sizeof(FCascade) });
        }
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
