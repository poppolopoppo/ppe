#pragma once

#include "Core_fwd.h"

#include "HAL/PlatformMemory.h"
#include "Memory/MemoryDomain.h"
#include "Memory/MemoryView.h"
#include "Meta/TypeTraits.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct NODISCARD FAllocatorBlock {
    void* Data{ nullptr };
    size_t SizeInBytes{ 0 };

    FAllocatorBlock() = default;

    PPE_FAKEBOOL_OPERATOR_DECL() { return (!!Data); }
    FRawMemory MakeView() const { return FRawMemory((u8*)Data, SizeInBytes); }

    FAllocatorBlock Reset() {
        const FAllocatorBlock cpy{ *this };
        *this = Null();
        return cpy;
    }

    static CONSTEXPR FAllocatorBlock Null() NOEXCEPT {
        return FAllocatorBlock{ nullptr, 0 };
    }

    template <typename T>
    static FAllocatorBlock From(TMemoryView<T> v) NOEXCEPT {
        return FAllocatorBlock{ v.data(), v.SizeInBytes() };
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Defines the concept of a PPE allocator
//----------------------------------------------------------------------------
class FGenericAllocator {
public:
    using propagate_on_container_copy_assignment = std::false_type;
    using propagate_on_container_move_assignment = std::false_type;
    using propagate_on_container_swap = std::false_type;

    using is_always_equal = std::false_type;

    using has_maxsize = std::false_type;
    using has_owns = std::false_type;
    using has_reallocate = std::false_type;
    using has_acquire = std::false_type;
    using has_steal = std::false_type;

    STATIC_CONST_INTEGRAL(size_t, Alignment, INDEX_NONE);

    static size_t MaxSize() NOEXCEPT = delete;
    static size_t SnapSize(size_t s) NOEXCEPT = delete;

    bool Owns(FAllocatorBlock b) const NOEXCEPT = delete;

    FAllocatorBlock Allocate(size_t s) = delete;
    void Deallocate(FAllocatorBlock b) = delete;

    void Reallocate(FAllocatorBlock& b, size_t s) = delete;

    bool Acquire(FAllocatorBlock b) NOEXCEPT = delete;
    bool Steal(FAllocatorBlock b) NOEXCEPT = delete;

#if USE_PPE_MEMORYDOMAINS
    using has_memory_tracking = std::false_type;

    FMemoryTracking& TrackingData() NOEXCEPT = delete;
    FGenericAllocator& AllocatorWithoutTracking() NOEXCEPT = delete;

#endif

    friend bool operator ==(const FGenericAllocator& lhs, const FGenericAllocator& rhs) NOEXCEPT = delete;
    friend bool operator !=(const FGenericAllocator& lhs, const FGenericAllocator& rhs) NOEXCEPT = delete;
};
//----------------------------------------------------------------------------
// Detects if given type if a valid allocator (ie has Allocate/Deallocate)
//----------------------------------------------------------------------------
namespace details {
template <typename T>
using if_has_allocate_ = decltype(std::declval<T&>().Allocate(size_t(0)));
template <typename T>
using if_has_deallocate_ = decltype(std::declval<T&>().Deallocate(std::declval<FAllocatorBlock>()));
} //!details
//----------------------------------------------------------------------------
template <typename T>
using is_allocator_t = std::bool_constant<
    Meta::has_defined_v<details::if_has_allocate_, T> &&
    Meta::has_defined_v<details::if_has_deallocate_, T>
    >;
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR bool is_allocator_v = is_allocator_t<T>::value;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// The traits will use default behaviors/values when missing in _Allocator
//----------------------------------------------------------------------------
namespace details {
template <typename _Allocator>
using if_propagate_on_container_copy_assignment_ = typename _Allocator::propagate_on_container_copy_assignment;
template <typename _Allocator>
using if_propagate_on_container_move_assignment_ = typename _Allocator::propagate_on_container_move_assignment;
template <typename _Allocator>
using if_propagate_on_container_swap_ = typename _Allocator::propagate_on_container_swap;
template <typename _Allocator>
using if_is_always_equal_ = typename _Allocator::is_always_equal;
template <typename _Allocator>
using if_has_maxsize_ = typename _Allocator::has_maxsize;
template <typename _Allocator>
using if_has_owns_ = typename _Allocator::has_owns;
template <typename _Allocator>
using if_has_reallocate_ = typename _Allocator::has_reallocate;
template <typename _Allocator>
using if_has_acquire_ = typename _Allocator::has_acquire;
template <typename _Allocator>
using if_has_steal_ = typename _Allocator::has_steal;
template <typename _Allocator>
using if_reallocate_can_fail_ = decltype(std::declval<_Allocator&>().Reallocate(
    std::declval<FAllocatorBlock&>(), size_t(0) ));
#if USE_PPE_MEMORYDOMAINS
template <typename _Allocator>
using if_has_memory_tracking_ = typename _Allocator::has_memory_tracking;
#endif
} //!details
//----------------------------------------------------------------------------
template <typename _Allocator>
struct TAllocatorTraits {
    using allocator_type = _Allocator;

    using propagate_on_container_copy_assignment = Meta::optional_definition_t<
        details::if_propagate_on_container_copy_assignment_, std::false_type, _Allocator >;
    using propagate_on_container_move_assignment = Meta::optional_definition_t<
        details::if_propagate_on_container_move_assignment_, std::false_type, _Allocator >;
    using propagate_on_container_swap = Meta::optional_definition_t<
        details::if_propagate_on_container_swap_, std::false_type, _Allocator >;

    using is_always_equal = Meta::optional_definition_t<
        details::if_is_always_equal_, std::false_type, _Allocator >;

    using has_maxsize = Meta::optional_definition_t<
        details::if_has_maxsize_, std::false_type, _Allocator >;
    using has_owns = Meta::optional_definition_t<
        details::if_has_owns_, std::false_type, _Allocator >;
    using has_reallocate = Meta::optional_definition_t<
        details::if_has_reallocate_, std::false_type, _Allocator >;
    using has_acquire = Meta::optional_definition_t<
        details::if_has_acquire_, std::false_type, _Allocator >;
    using has_steal = Meta::optional_definition_t<
        details::if_has_steal_, std::false_type, _Allocator >;

    STATIC_CONST_INTEGRAL(size_t, Alignment, _Allocator::Alignment);

    using reallocate_can_fail = std::is_same<
        Meta::optional_definition_t<details::if_reallocate_can_fail_, void, _Allocator>,
        bool >;

    static _Allocator& Get(_Allocator& self) NOEXCEPT { return self; }
    static const _Allocator& Get(const _Allocator& self) NOEXCEPT { return self; }

    static void Copy(_Allocator* dst, const _Allocator& src) {
        IF_CONSTEXPR(propagate_on_container_copy_assignment::value) {
            *dst = src;
        }
        else {
            UNUSED(dst);
            UNUSED(src);
        }
    }

    NODISCARD static _Allocator SelectOnCopy(const _Allocator& other) {
        IF_CONSTEXPR(propagate_on_container_copy_assignment::value) {
            return _Allocator{ other };
        }
        else {
            UNUSED(other);
            return Meta::MakeForceInit<_Allocator>();
        }
    }

    static void Move(_Allocator* dst, _Allocator&& src) {
        IF_CONSTEXPR(propagate_on_container_move_assignment::value) {
            *dst = std::move(src);
        }
        else {
            UNUSED(dst);
            UNUSED(src);
        }
    }

    NODISCARD static _Allocator SelectOnMove(_Allocator&& rvalue) {
        IF_CONSTEXPR(propagate_on_container_move_assignment::value) {
            return _Allocator{ std::move(rvalue) };
        }
        else {
            UNUSED(rvalue);
            return Meta::MakeForceInit<_Allocator>();
        }
    }

    static void Swap(_Allocator& lhs, _Allocator& rhs) {
        IF_CONSTEXPR(propagate_on_container_swap::value) {
            using std::swap;
            swap(lhs, rhs); // can be overloaded
        }
        else {
            UNUSED(lhs);
            UNUSED(rhs);
        }
    }

    NODISCARD static bool Equals(const _Allocator& lhs, const _Allocator& rhs) NOEXCEPT {
        IF_CONSTEXPR(is_always_equal::value) {
            UNUSED(lhs);
            UNUSED(rhs);
            return true;
        }
        else
            return (lhs == rhs);
    }

    template <typename _AllocatorOther>
    NODISCARD static bool Equals(const _Allocator& lhs, const _AllocatorOther& rhs) NOEXCEPT {
        IF_CONSTEXPR(Meta::has_equals_v<_Allocator, _AllocatorOther>)
            return (lhs == rhs);
        else {
            UNUSED(lhs);
            UNUSED(rhs);
            return false;
        }
    }

    NODISCARD static size_t MaxSize() NOEXCEPT {
        IF_CONSTEXPR(has_maxsize::value) {
            return _Allocator::MaxSize();
        }
        else {
            return (size_t(0) - size_t(1));
        }
    }

    NODISCARD static size_t SnapSize(size_t size) NOEXCEPT {
#if USE_PPE_ASSERT
        const size_t snapped = _Allocator::SnapSize(size);
        Assert(snapped >= size);
        Assert_NoAssume(_Allocator::SnapSize(snapped) == snapped);
        return snapped;
#else
        return _Allocator::SnapSize(size);
#endif
    }

    template <typename T>
    NODISCARD static size_t SnapSizeT(size_t count) NOEXCEPT {
        return (SnapSize(count * sizeof(T)) / sizeof(T));
    }

    NODISCARD static bool Owns(const _Allocator& a, FAllocatorBlock b) NOEXCEPT {
        IF_CONSTEXPR(has_owns::value) {
            return a.Owns(b);
        }
        else {
#if defined(BUILD_LINK_DYNAMIC) || defined(__clang__)
            UNUSED(a);
            UNUSED(b);
            AssertNotImplemented();
#else
            static_assert(false, "given allocator doesn't implement Owns() method");
#endif
        }
    }

    NODISCARD static FAllocatorBlock Allocate(_Allocator& a, size_t s) {
        Assert_NoAssume(s <= MaxSize());
        return (s ? a.Allocate(s) : FAllocatorBlock::Null());
    }

    template <typename T>
    NODISCARD static TMemoryView<T> AllocateT(_Allocator& a, size_t n) {
        const FAllocatorBlock b{ Allocate(a, n * sizeof(T)) };
        Assert_NoAssume(n * sizeof(T) <= b.SizeInBytes);
        return b.MakeView().Cast<T>();
    }

    template <typename T>
    NODISCARD static T* AllocateOneT(_Allocator& a) {
        return static_cast<T*>(Allocate(a, sizeof(T)).Data);
    }

    static void Deallocate(_Allocator& a, FAllocatorBlock b) {
        Assert_NoAssume(b.SizeInBytes <= MaxSize());
        if (b)
            a.Deallocate(b);
    }

    template <typename T>
    static void DeallocateT(_Allocator& a, TMemoryView<T> v) {
        Deallocate(a, FAllocatorBlock::From(v));
    }

    template <typename T>
    static void DeallocateT(_Allocator& a, T* p, size_t n) {
        Deallocate(a, FAllocatorBlock{ p,  n * sizeof(T) });
    }

    template <typename T>
    static void DeallocateOneT(_Allocator& a, T* p) {
        Deallocate(a, FAllocatorBlock{ p,  sizeof(T) });
    }

    NODISCARD static auto Reallocate(_Allocator& a, FAllocatorBlock& b, size_t s) {
        Assert_NoAssume(b || s);
        Assert_NoAssume(b.SizeInBytes <= MaxSize());
        Assert_NoAssume(s <= MaxSize());

        IF_CONSTEXPR(has_reallocate::value) {
            return a.Reallocate(b, s);
        }
        else {
            if ((!!b) & (!!s)) {
                const FAllocatorBlock r = a.Allocate(s);
                Assert_NoAssume(r);
                FPlatformMemory::MemcpyLarge(r.Data, b.Data, Min(s, b.SizeInBytes));
                a.Deallocate(b);
                b = r;
            }
            else {
                if (Likely(s)) {
                    Assert_NoAssume(not b);
                    b = a.Allocate(s);
                }
                else {
                    Assert_NoAssume(b);
                    a.Deallocate(b.Reset());
                }
            }
            return;
        }
    }

    // specialized this method to avoid over-copying when !has_reallocate
    template <typename T>
    NODISCARD static auto ReallocateT_AssumePOD(_Allocator& a, TMemoryView<T>& items, size_t oldSize, size_t newSize) {
        Assert(oldSize >= items.size());
        Assert_NoAssume(oldSize * sizeof(T) <= MaxSize());
        Assert_NoAssume(newSize * sizeof(T) <= MaxSize());

        FAllocatorBlock b{ items.data(), oldSize * sizeof(T) };
        const size_t s = newSize * sizeof(T);

        IF_CONSTEXPR(has_reallocate::value) {
            IF_CONSTEXPR(reallocate_can_fail::value) {
                Verify(a.Reallocate(b, s));
            }
            else {
                a.Reallocate(b, s);
            }
        }
        else {
            if ((!!b) & (!!s)) {
                const FAllocatorBlock r = a.Allocate(s);
                Assert_NoAssume(r);
                // *HERE* copy potentially less since we're giving the actual used size (which can be less reserved size)
                FPlatformMemory::MemcpyLarge(r.Data, b.Data, Min(s, /*b.SizeInBytes*/items.SizeInBytes()));
                a.Deallocate(b);
                b = r;
            }
            else {
                if (Likely(s)) {
                    Assert_NoAssume(not b);
                    b = a.Allocate(s);
                }
                else {
                    Assert_NoAssume(b);
                    a.Deallocate(b.Reset());
                }
            }
        }

        items = TMemoryView<T>(static_cast<T*>(b.Data), Min(newSize, items.size()));
    }

    NODISCARD static bool Acquire(_Allocator& a, FAllocatorBlock b) NOEXCEPT {
        Assert(b);
        Assert_NoAssume(b.SizeInBytes <= MaxSize());

        IF_CONSTEXPR(has_acquire::value)
            return a.Acquire(b);
        else {
            UNUSED(a);
            UNUSED(b);
            return false;
        }
    }

    NODISCARD static bool Steal(_Allocator& a, FAllocatorBlock b) NOEXCEPT {
        Assert(b);
        Assert_NoAssume(b.SizeInBytes <= MaxSize());

        IF_CONSTEXPR(has_steal::value)
            return a.Steal(b);
        else {
            UNUSED(a);
            UNUSED(b);
            return false;
        }
    }

    template <typename _AllocatorDst>
    NODISCARD static bool StealAndAcquire(_AllocatorDst* dst, _Allocator& src, FAllocatorBlock b) NOEXCEPT {
        Assert(dst);
        Assert_NoAssume(b.SizeInBytes <= MaxSize());

        using dst_t = TAllocatorTraits<_AllocatorDst>;
        Assert_NoAssume(b.SizeInBytes <= dst_t::MaxSize());

        IF_CONSTEXPR(dst_t::has_acquire::value && has_steal::value) {
            return (Steal(src, b) && dst_t::Acquire(*dst, b));
        }
        else {
            UNUSED(dst);
            UNUSED(src);
            UNUSED(b);
            return false;
        }
    }

#if USE_PPE_MEMORYDOMAINS
    using has_memory_tracking = Meta::optional_definition_t<
        details::if_has_memory_tracking_, std::false_type, _Allocator >;

    NODISCARD static FMemoryTracking* TrackingData(_Allocator& a) NOEXCEPT {
        IF_CONSTEXPR(has_memory_tracking::value) {
            return a.TrackingData();
        }
        else {
            return std::addressof(MEMORYDOMAIN_TRACKING_DATA(Unknown));
        }
    }

    NODISCARD static auto& AllocatorWithoutTracking(_Allocator& a) NOEXCEPT {
        IF_CONSTEXPR(has_memory_tracking::value) {
            return a.AllocatorWithoutTracking();
        }
        else {
            return a;
        }
    }

#else

    NODISCARD static auto& AllocatorWithoutTracking(_Allocator& a) NOEXCEPT {
        return a;
    }

#endif

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Propagate allocation on move, return false if the source block was copied
//----------------------------------------------------------------------------
template <typename _Allocator>
NODISCARD bool MoveAllocatorBlock(_Allocator* dst, _Allocator& src, FAllocatorBlock b) NOEXCEPT {
    Assert(dst);

    using traits_t = TAllocatorTraits<_Allocator>;
    IF_CONSTEXPR(traits_t::propagate_on_container_move_assignment::value) {
        // nothing to do : the allocator is trivially movable
        UNUSED(dst);
        UNUSED(src);
        UNUSED(b);

        traits_t::Move(dst, std::move(src));

        return true;
    }
    else {
        return (not b || traits_t::StealAndAcquire(dst, src, b));
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Helpers for Reallocate()
//----------------------------------------------------------------------------
template <typename _Allocator, typename T>
NODISCARD auto ReallocateAllocatorBlock_AssumePOD(_Allocator& a, TMemoryView<T>& items, size_t oldSize, size_t newSize) {
    return TAllocatorTraits<_Allocator>::ReallocateT_AssumePOD(a, items, oldSize, newSize);
}
//---------------------------------------------------------------------------
template <typename _Allocator, typename T>
void ReallocateAllocatorBlock_NonPOD(_Allocator& a, TMemoryView<T>& items, size_t oldSize, size_t newSize) {
    using traits_t = TAllocatorTraits<_Allocator>;

    const FAllocatorBlock o{ items.data(), oldSize * sizeof(T) };
    FAllocatorBlock b = traits_t::Allocate(a, newSize * sizeof(T));

    std::uninitialized_move(
        items.begin(), items.begin() + Min(newSize, items.size()),
        MakeCheckedIterator(static_cast<T*>(b.Data), newSize, 0));

    Meta::Destroy(items);

    traits_t::Deallocate(a, o);

    items = { static_cast<T*>(b.Data), Min(newSize, items.size()) };
}
//---------------------------------------------------------------------------
template <typename _Allocator, typename T>
NODISCARD auto ReallocateAllocatorBlock(_Allocator& a, TMemoryView<T>& items, size_t oldSize, size_t newSize) {
    IF_CONSTEXPR(Meta::has_trivial_move<T>::value)
        return ReallocateAllocatorBlock_AssumePOD(a, items, oldSize, newSize);
    else
        return ReallocateAllocatorBlock_NonPOD(a, items, oldSize, newSize);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Handles memory stealing from one allocator to another
// This is only handling the trivial case of stealing between 2 allocators of the same type
// It should be specialized for every wanted allocator combinations
//----------------------------------------------------------------------------
template <typename _Allocator>
NODISCARD bool StealAllocatorBlock(_Allocator* dst, _Allocator& src, FAllocatorBlock b) NOEXCEPT {
    using traits_t = TAllocatorTraits<_Allocator>;
    return traits_t::StealAndAcquire(dst, src, b);
}
//----------------------------------------------------------------------------
// Test statically if two allocators can steal/acquire from one to another
//----------------------------------------------------------------------------
namespace details {
template <typename _AllocatorDst, typename _AllocatorSrc,
    class = decltype(StealAllocatorBlock(std::declval<_AllocatorDst*>(), std::declval<_AllocatorSrc&>(), std::declval<FAllocatorBlock>())) >
std::true_type has_stealallocatorblock_(int);
template <typename _AllocatorDst, typename _AllocatorSrc>
std::false_type has_stealallocatorblock_(...);
} //!details
//----------------------------------------------------------------------------
template <typename _AllocatorDst, typename _AllocatorSrc>
using has_stealallocatorblock_t = decltype(details::has_stealallocatorblock_<_AllocatorDst, _AllocatorSrc>(0));
//----------------------------------------------------------------------------
template <typename _AllocatorDst, typename _AllocatorSrc>
CONSTEXPR bool has_stealallocatorblock_v = has_stealallocatorblock_t<_AllocatorDst, _AllocatorSrc>::value;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Memory tracking from allocators
//----------------------------------------------------------------------------
#if USE_PPE_MEMORYDOMAINS
template <typename _Allocator>
NODISCARD FMemoryTracking* AllocatorTrackingData(_Allocator& a) {
    return TAllocatorTraits<_Allocator>::TrackingData(a);
}
template <typename _AllocatorA, typename _AllocatorB>
NODISCARD FMemoryTracking* AllocatorTrackingData(_AllocatorA& a, _AllocatorB& b) {
    using a_traits = TAllocatorTraits<_AllocatorA>;
    using b_traits = TAllocatorTraits<_AllocatorB>;

    IF_CONSTEXPR(a_traits::has_memory_tracking::value) {
        IF_CONSTEXPR(b_traits::has_memory_tracking::value) {
            FMemoryTracking* const trackingA = a_traits::TrackingData(a);
            Assert_NoAssume(trackingA == b_traits::TrackingData(b));
            return trackingA;
        }
        else {
            return a_traits::TrackingData(a);
        }
    }
    else {
        return b_traits::TrackingData(b);
    }
}
#endif
//----------------------------------------------------------------------------
template <typename _Allocator>
NODISCARD static auto& AllocatorWithoutTracking(_Allocator& a) NOEXCEPT {
    return TAllocatorTraits<_Allocator>::AllocatorWithoutTracking(a);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
