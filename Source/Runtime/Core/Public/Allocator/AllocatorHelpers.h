#pragma once

#include "Core_fwd.h"

#include "Allocator/AllocatorBase.h"
#include "Allocator/TrackingMalloc.h"
#include "Container/IntrusiveList.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Tries to allocate from _Primary, if failed use _Fallback instead
//----------------------------------------------------------------------------
template <typename _Primary, typename _Fallback>
class EMPTY_BASES TFallbackAllocator
:   private _Primary
,   private _Fallback {
    STATIC_ASSERT(_Primary::has_owns::value); // needed for Deallocate()
public:
    using primary_traits = TAllocatorTraits<_Primary>;
    using fallback_traits = TAllocatorTraits<_Fallback>;

#define FALLBACK_USING_DEF(_NAME, _OP) \
    using _NAME = typename std::bool_constant< \
        primary_traits::_NAME::value _OP \
        fallback_traits::_NAME::value >::type

    FALLBACK_USING_DEF(propagate_on_container_copy_assignment, and);
    FALLBACK_USING_DEF(propagate_on_container_move_assignment, and);
    FALLBACK_USING_DEF(propagate_on_container_swap, and);

    FALLBACK_USING_DEF(is_always_equal, and);

    FALLBACK_USING_DEF(has_maxsize, or);
    FALLBACK_USING_DEF(has_owns, and);
    FALLBACK_USING_DEF(has_reallocate, or);
    FALLBACK_USING_DEF(has_acquire, or);
    FALLBACK_USING_DEF(has_steal, or);

#undef FALLBACK_USING_DEF

    STATIC_CONST_INTEGRAL(size_t, Alignment, Min(primary_traits::Alignment, fallback_traits::Alignment));
    STATIC_ASSERT(Meta::IsAligned(Alignment, primary_traits::Alignment));
    STATIC_ASSERT(Meta::IsAligned(Alignment, fallback_traits::Alignment));

    TFallbackAllocator() = default;

    TFallbackAllocator(const _Primary& primary, const _Fallback& fallback)
    :   _Primary(primary)
    ,   _Fallback(fallback)
    {}
    TFallbackAllocator(_Primary&& rprimary, _Fallback&& rfallback)
    :   _Primary(std::move(rprimary))
    ,   _Fallback(std::move(rfallback))
    {}

    TFallbackAllocator(const TFallbackAllocator& other)
    :   _Primary(primary_traits::SelectOnCopy(other))
    ,   _Fallback(fallback_traits::SelectOnCopy(other))
    {}
    TFallbackAllocator& operator =(const TFallbackAllocator& other) {
        primary_traits::Copy(this, other);
        fallback_traits::Copy(this, other);
        return (*this);
    }

    TFallbackAllocator(TFallbackAllocator&& rvalue)
    :   _Primary(primary_traits::SelectOnMove(std::move(rvalue)))
    ,   _Fallback(fallback_traits::SelectOnMove(std::move(rvalue)))
    {}
    TFallbackAllocator& operator =(TFallbackAllocator&& rvalue) {
        primary_traits::Move(this, std::move(rvalue));
        fallback_traits::Move(this, std::move(rvalue));
        return (*this);
    }

    static size_t MaxSize() NOEXCEPT {
        return Min(primary_traits::MaxSize(), fallback_traits::MaxSize());
    }

    static size_t SnapSize(size_t s) NOEXCEPT {
        s = primary_traits::SnapSize(s);
        Assert_NoAssume(fallback_traits::SnapSize(s) == s);
        return s;
    }

    bool Owns(FAllocatorBlock b) const NOEXCEPT {
        return (primary_traits::Owns(*this, b) || fallback_traits::Owns(*this, b));
    }

    FAllocatorBlock Allocate(size_t s) {
        FAllocatorBlock b = primary_traits::Allocate(*this, s);
        if (Unlikely(not b))
            b = fallback_traits::Allocate(*this, s);
        return b;
    }

    void Deallocate(FAllocatorBlock b) {
        if (Likely(primary_traits::Owns(*this, b)))
            primary_traits::Deallocate(*this, b);
        else
            fallback_traits::Deallocate(*this, b);
    }

    void Reallocate(FAllocatorBlock& b, size_t s) {
        if (Likely(primary_traits::Owns(*this, b)))
            primary_traits::Reallocate(*this, b, s);
        else
            fallback_traits::Reallocate(*this, b, s);
    }

    bool Acquire(FAllocatorBlock b) NOEXCEPT {
        if (Unlikely(not primary_traits::Acquire(*this, b)))
            return fallback_traits::Acquire(*this, b);
        else
            return true;
    }

    bool Steal(FAllocatorBlock b) NOEXCEPT {
        if (Likely(primary_traits::Owns(*this, b)))
            return primary_traits::Steal(*this, b);
        else
            return fallback_traits::Steal(*this, b);
    }

    friend bool operator ==(const TFallbackAllocator& lhs, const TFallbackAllocator& rhs) NOEXCEPT {
        return (primary_traits::Equals(lhs, rhs) &&
                fallback_traits::Equals(lhs, rhs));
    }

    friend bool operator !=(const TFallbackAllocator& lhs, const TFallbackAllocator& rhs) NOEXCEPT {
        return (not operator ==(lhs, rhs));
    }

    friend void swap(TFallbackAllocator& lhs, TFallbackAllocator& rhs) NOEXCEPT {
        primary_traits::Swap(lhs, rhs);
        fallback_traits::Swap(lhs, rhs);
    }
};
//----------------------------------------------------------------------------
// Proxy allocator is wrapping an internal reference to another allocator
//----------------------------------------------------------------------------
template <typename _Allocator>
class TProxyAllocator : private FGenericAllocator {
public:
    using allocator_traits = TAllocatorTraits<_Allocator>;

#define PROXY_USING_DEF(_NAME) \
    using _NAME = typename allocator_traits::_NAME;

    PROXY_USING_DEF(propagate_on_container_copy_assignment);
    PROXY_USING_DEF(propagate_on_container_move_assignment);
    PROXY_USING_DEF(propagate_on_container_swap);

    PROXY_USING_DEF(is_always_equal);

    PROXY_USING_DEF(has_maxsize);
    PROXY_USING_DEF(has_owns);
    PROXY_USING_DEF(has_reallocate);
    PROXY_USING_DEF(has_acquire);
    PROXY_USING_DEF(has_steal);

#undef PROXY_USING_DEF

    STATIC_CONST_INTEGRAL(size_t, Alignment, allocator_traits::Alignment);

    _Allocator* AllocRef;

    explicit TProxyAllocator(_Allocator* a) NOEXCEPT
    :   AllocRef(a)
    {}
    explicit TProxyAllocator(Meta::FForceInit) NOEXCEPT
    :   AllocRef(nullptr)
    {}

    static size_t MaxSize() {
        return allocator_traits::MaxSize();
    }

    static size_t SnapSize(size_t s) NOEXCEPT {
        return allocator_traits::SnapSize(s);
    }

    bool Owns(FAllocatorBlock b) const NOEXCEPT {
        return allocator_traits::Owns(*AllocRef, b);
    }

    FAllocatorBlock Allocate(size_t s) {
        return allocator_traits::Allocate(*AllocRef, s);
    }

    void Deallocate(FAllocatorBlock b) {
        allocator_traits::Deallocate(*AllocRef, b);
    }

    void Reallocate(FAllocatorBlock& b, size_t s) {
        allocator_traits::Reallocate(*AllocRef, b, s);
    }

    bool Acquire(FAllocatorBlock b) NOEXCEPT {
        return allocator_traits::Acquire(*AllocRef, b);
    }

    bool Steal(FAllocatorBlock b) NOEXCEPT {
        return allocator_traits::Steal(*AllocRef, b);
    }

    friend bool operator ==(const TProxyAllocator& lhs, const TProxyAllocator& rhs) NOEXCEPT {
        return (lhs.AllocRef == rhs.AllocRef);
    }

    friend bool operator !=(const TProxyAllocator& lhs, const TProxyAllocator& rhs) NOEXCEPT {
        return (not operator ==(lhs, rhs));
    }

    friend void swap(TProxyAllocator& lhs, TProxyAllocator& rhs) NOEXCEPT {
        std::swap(lhs.AllocRef, rhs.AllocRef);
    }
};
//----------------------------------------------------------------------------
// Segregates allocations based on their size
//----------------------------------------------------------------------------
template <size_t _Threshold, typename _Under, typename _Above>
class EMPTY_BASES TSegregatorAllocator
:   private _Under
,   private _Above {
public:
    using under_traits = TAllocatorTraits<_Under>;
    using above_traits = TAllocatorTraits<_Above>;

#define SEGREGATOR_USING_DEF(_NAME, _OP) \
    using _NAME = typename std::bool_constant< \
        under_traits::_NAME::value _OP \
        above_traits::_NAME::value >::type

    SEGREGATOR_USING_DEF(propagate_on_container_copy_assignment, and);
    SEGREGATOR_USING_DEF(propagate_on_container_move_assignment, and);
    SEGREGATOR_USING_DEF(propagate_on_container_swap, and);

    SEGREGATOR_USING_DEF(is_always_equal, and);

    using has_maxsize = typename above_traits::has_maxsize;
    SEGREGATOR_USING_DEF(has_owns, and);
    SEGREGATOR_USING_DEF(has_reallocate, or);
    SEGREGATOR_USING_DEF(has_acquire, or);
    SEGREGATOR_USING_DEF(has_steal, or);

#undef SEGREGATOR_USING_DEF

    STATIC_CONST_INTEGRAL(size_t, Threshold, _Threshold);

    STATIC_CONST_INTEGRAL(size_t, Alignment, Min(under_traits::Alignment, above_traits::Alignment));
    STATIC_ASSERT(Meta::IsAligned(Alignment, under_traits::Alignment));
    STATIC_ASSERT(Meta::IsAligned(Alignment, above_traits::Alignment));

    TSegregatorAllocator() = default;

    TSegregatorAllocator(const _Under& under, const _Above& above)
    :   _Under(under)
    ,   _Above(above)
    {}
    TSegregatorAllocator(_Under&& runder, _Above&& rabove)
    :   _Under(std::move(runder))
    ,   _Above(std::move(rabove))
    {}

    TSegregatorAllocator(const TSegregatorAllocator& other)
    :   _Under(under_traits::SelectOnCopy(other))
    ,   _Above(above_traits::SelectOnCopy(other))
    {}
    TSegregatorAllocator& operator =(const TSegregatorAllocator& other) {
        under_traits::Copy(this, other);
        above_traits::Copy(this, other);
        return (*this);
    }

    TSegregatorAllocator(TSegregatorAllocator&& rvalue)
    :   _Under(under_traits::SelectOnMove(std::move(rvalue)))
    ,   _Above(above_traits::SelectOnMove(std::move(rvalue)))
    {}
    TSegregatorAllocator& operator =(TSegregatorAllocator&& rvalue) {
        under_traits::Move(this, std::move(rvalue));
        above_traits::Move(this, std::move(rvalue));
        return (*this);
    }

    static size_t MaxSize() NOEXCEPT {
        return above_traits::MaxSize();
    }

    static size_t SnapSize(size_t s) NOEXCEPT {
        if (Likely(s <= _Threshold))
            return under_traits::SnapSize(s);
        else
            return above_traits::SnapSize(s);
    }

    bool Owns(FAllocatorBlock b) const NOEXCEPT {
        if (Likely(b.SizeInBytes <= _Threshold))
            return under_traits::Owns(*this, b);
        else
            return above_traits::Owns(*this, b);
    }

    FAllocatorBlock Allocate(size_t s) {
        if (Likely(s <= _Threshold))
            return under_traits::Allocate(*this, s);
        else
            return above_traits::Allocate(*this, s);
    }

    void Deallocate(FAllocatorBlock b) {
        if (b.SizeInBytes <= _Threshold)
            under_traits::Deallocate(*this, b);
        else
            above_traits::Deallocate(*this, b);
    }

    void Reallocate(FAllocatorBlock& b, size_t s) {
        if ((s <= _Threshold) & (b.SizeInBytes <= _Threshold))
            under_traits::Reallocate(*this, b, s);
        else if ((s > _Threshold) & (b.SizeInBytes > _Threshold))
            above_traits::Reallocate(*this, b, s);
        else {
            const FAllocatorBlock n = Allocate(s);
            FPlatformMemory::MemcpyLarge(n.Data, b.Data, Min(b.SizeInBytes, s));
            Deallocate(b);
            b = n;
        }
    }

    bool Acquire(FAllocatorBlock b) NOEXCEPT {
        if (Likely(b.SizeInBytes <= _Threshold))
            return under_traits::Acquire(*this, b);
        else
            return above_traits::Acquire(*this, b);
    }

    bool Steal(FAllocatorBlock b) NOEXCEPT {
        if (Likely(b.SizeInBytes <= _Threshold))
            return under_traits::Steal(*this, b);
        else
            return above_traits::Steal(*this, b);
    }

    friend bool operator ==(const TSegregatorAllocator& lhs, const TSegregatorAllocator& rhs) NOEXCEPT {
        return (under_traits::Equals(lhs, rhs) &&
                above_traits::Equals(lhs, rhs) );
    }

    friend bool operator !=(const TSegregatorAllocator& lhs, const TSegregatorAllocator& rhs) NOEXCEPT {
        return (not operator ==(lhs, rhs));
    }

    friend void swap(TSegregatorAllocator& lhs, TSegregatorAllocator& rhs) NOEXCEPT {
        under_traits::Swap(lhs, rhs);
        above_traits::Swap(lhs, rhs);
    }

private: // segregators are self-composable, use this helper to chain
    // Ex:
    //      TSegregatorAllocator<64, FStacklocalAllocator, FMallocator>
    //          ::bellow_t<32, TFreeList<...> >
    //          ::bellow_t<16, TBitmapAllocator<> >
    //          [...]
    //

    template <size_t _Bellow, typename _Other>
    struct make_bellow_t {
        STATIC_ASSERT(_Bellow < _Threshold);
        using type = TSegregatorAllocator<_Bellow, _Other, TSegregatorAllocator>;
    };

public:
    template <size_t _Bellow, typename _Other>
    using bellow_t = typename make_bellow_t<_Bellow, _Other>::type;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
