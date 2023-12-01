#pragma once

#include "Core_fwd.h"

#include "Allocator/AllocatorBlock.h"
#include "HAL/PlatformMemory.h"
#include "Memory/MemoryDomain.h"
#include "Meta/TypeTraits.h"
#include "Thread/ThreadSafe_fwd.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Defines the concept of a PPE allocator
//----------------------------------------------------------------------------
class FAllocatorPolicy {
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

    size_t MaxSize() const NOEXCEPT = delete;
    size_t SnapSize(size_t s) const NOEXCEPT = delete;

    bool Owns(FAllocatorBlock b) const NOEXCEPT = delete;

    FAllocatorBlock Allocate(size_t s) = delete;
    void Deallocate(FAllocatorBlock b) = delete;

    void Reallocate(FAllocatorBlock& b, size_t s) = delete;

    bool Acquire(FAllocatorBlock b) NOEXCEPT = delete;
    bool Steal(FAllocatorBlock b) NOEXCEPT = delete;

#if USE_PPE_MEMORYDOMAINS
    using has_memory_tracking = std::false_type;

    FMemoryTracking& TrackingData() NOEXCEPT = delete;
    FAllocatorPolicy& AllocatorWithoutTracking() NOEXCEPT = delete;

#endif

    friend bool operator ==(const FAllocatorPolicy& lhs, const FAllocatorPolicy& rhs) NOEXCEPT = delete;
    friend bool operator !=(const FAllocatorPolicy& lhs, const FAllocatorPolicy& rhs) NOEXCEPT = delete;
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
            Unused(dst);
            Unused(src);
        }
    }

    NODISCARD static _Allocator SelectOnCopy(const _Allocator& other) {
        IF_CONSTEXPR(propagate_on_container_copy_assignment::value) {
            return _Allocator{ other };
        }
        else {
            Unused(other);
            return Meta::MakeForceInit<_Allocator>();
        }
    }

    static void Move(_Allocator* dst, _Allocator&& src) {
        IF_CONSTEXPR(propagate_on_container_move_assignment::value) {
            *dst = std::move(src);
        }
        else {
            Unused(dst);
            Unused(src);
        }
    }

    NODISCARD static _Allocator SelectOnMove(_Allocator&& rvalue) {
        IF_CONSTEXPR(propagate_on_container_move_assignment::value) {
            return _Allocator{ std::move(rvalue) };
        }
        else {
            Unused(rvalue);
            return Meta::MakeForceInit<_Allocator>();
        }
    }

    static void Swap(_Allocator& lhs, _Allocator& rhs) {
        IF_CONSTEXPR(propagate_on_container_swap::value) {
            using std::swap;
            swap(lhs, rhs); // can be overloaded
        }
        else {
            Unused(lhs);
            Unused(rhs);
        }
    }

    NODISCARD static bool Equals(const _Allocator& lhs, const _Allocator& rhs) NOEXCEPT {
        IF_CONSTEXPR(is_always_equal::value) {
            Unused(lhs);
            Unused(rhs);
            return true;
        }
        else
            return (lhs == rhs);
    }

    template <typename _AllocatorOther>
    NODISCARD static bool Equals(const _Allocator& lhs, const _AllocatorOther& rhs) NOEXCEPT {
        IF_CONSTEXPR(Meta::has_trivial_equal_v<_Allocator, _AllocatorOther>)
            return (lhs == rhs);
        else {
            Unused(lhs);
            Unused(rhs);
            return false;
        }
    }

    NODISCARD static size_t MaxSize(const _Allocator& a) NOEXCEPT {
        IF_CONSTEXPR(has_maxsize::value) {
            return a.MaxSize();
        }
        else {
            return (static_cast<size_t>(0) - static_cast<size_t>(1));
        }
    }

    NODISCARD static size_t SnapSize(const _Allocator& a, size_t size) NOEXCEPT {
#if USE_PPE_ASSERT
        const size_t snapped = a.SnapSize(size);
        Assert(snapped >= size);
        Assert_NoAssume(a.SnapSize(snapped) == snapped);
        Assert_NoAssume(!!size || !snapped); // SnapSize(0) == 0
        return snapped;
#else
        return a.SnapSize(size);
#endif
    }

    template <typename T>
    NODISCARD static size_t SnapSizeT(const _Allocator& a, size_t count) NOEXCEPT {
        return (SnapSize(a, count * sizeof(T)) / sizeof(T));
    }

    NODISCARD static bool Owns(const _Allocator& a, FAllocatorBlock b) NOEXCEPT {
        IF_CONSTEXPR(has_owns::value) {
            return a.Owns(b);
        }
        else {
            Unused(a);
            Unused(b);
            AssertNotImplemented();
        }
    }

    NODISCARD static FAllocatorBlock Allocate(_Allocator& a, size_t s) {
        Assert_NoAssume(s <= MaxSize(a));
        return (s ? a.Allocate(s) : Default);
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
        Assert_NoAssume(b.SizeInBytes <= MaxSize(a));
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
        Assert_NoAssume(b.SizeInBytes <= MaxSize(a));
        Assert_NoAssume(s <= MaxSize(a));

        IF_CONSTEXPR(has_reallocate::value) {
            return a.Reallocate(b, s);
        }
        else {
            if (Unlikely(b.SizeInBytes == s)) {
                return;
            }

            if ((!!b) && (!!s)) {
                Assert_NoAssume(SnapSize(a, s) != SnapSize(a, b.SizeInBytes));
                const FAllocatorBlock r = a.Allocate(s);
                Assert_NoAssume(r);
                FPlatformMemory::MemcpyLarge(r.Data, b.Data, Min(s, b.SizeInBytes));
                a.Deallocate(b);
                b = r;
            }
            else if (Likely(s)) {
                Assert_NoAssume(not b);
                b = a.Allocate(s);
            }
            else {
                Assert_NoAssume(b);
                a.Deallocate(b.Reset());
            }
        }
    }

    NODISCARD static bool Acquire(_Allocator& a, FAllocatorBlock b) NOEXCEPT {
        Assert(b);
        Assert_NoAssume(b.SizeInBytes <= MaxSize(a));

        IF_CONSTEXPR(has_acquire::value)
            return a.Acquire(b);
        else {
            Unused(a);
            Unused(b);
            return false;
        }
    }

    NODISCARD static bool Steal(_Allocator& a, FAllocatorBlock b) NOEXCEPT {
        Assert(b);
        Assert_NoAssume(b.SizeInBytes <= MaxSize(a));

        IF_CONSTEXPR(has_steal::value)
            return a.Steal(b);
        else {
            Unused(a);
            Unused(b);
            return false;
        }
    }

    template <typename _AllocatorDst>
    NODISCARD static bool StealAndAcquire(_AllocatorDst* dst, _Allocator& src, FAllocatorBlock b) NOEXCEPT {
        Assert(dst);
        Assert_NoAssume(b.SizeInBytes <= MaxSize(src));

        using dst_t = TAllocatorTraits<_AllocatorDst>;
        Assert_NoAssume(b.SizeInBytes <= dst_t::MaxSize(*dst));

        IF_CONSTEXPR(dst_t::has_acquire::value && has_steal::value) {
            return (Steal(src, b) && dst_t::Acquire(*dst, b));
        }
        else {
            Unused(dst);
            Unused(src);
            Unused(b);
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
template <typename _Allocator>
struct TAllocatorTraits<TPtrRef<_Allocator>> {
    using allocator_type = _Allocator;
    using allocator_traits = TAllocatorTraits<_Allocator>;

    using propagate_on_container_copy_assignment = std::true_type;
    using propagate_on_container_move_assignment = std::true_type;
    using propagate_on_container_swap = std::true_type;

    using is_always_equal = typename allocator_traits::is_always_equal;

    using has_maxsize = typename allocator_traits::has_maxsize;
    using has_owns = typename allocator_traits::has_owns;
    using has_reallocate = typename allocator_traits::has_reallocate;
    using has_acquire = typename allocator_traits::has_acquire;
    using has_steal = typename allocator_traits::has_steal;

    STATIC_CONST_INTEGRAL(size_t, Alignment, allocator_traits::Alignment);

    using reallocate_can_fail = typename allocator_traits::reallocate_can_fail;

    static _Allocator& Get(const TPtrRef<_Allocator>& self) NOEXCEPT { return *self; }
    static const _Allocator& Get(const TPtrRef<const _Allocator>& self) NOEXCEPT { return *self; }

    static void Copy(TPtrRef<_Allocator> dst, TPtrRef<const _Allocator>& src) {
        allocator_traits::Copy(dst, src);
    }
    NODISCARD static TPtrRef<_Allocator> SelectOnCopy(const TPtrRef<_Allocator>& other) {
        return other; // copy reference
    }
    static void Move(TPtrRef<_Allocator> dst, TPtrRef<_Allocator>& src) {
        allocator_traits::Move(dst, std::move(*src));
    }
    NODISCARD static TPtrRef<_Allocator> SelectOnMove(const TPtrRef<_Allocator>& other) {
        return other; // copy reference
    }
    static void Swap(TPtrRef<_Allocator>& lhs, TPtrRef<_Allocator>& rhs) {
        swap(lhs, rhs); // swap references
    }
    NODISCARD static bool Equals(const TPtrRef<const _Allocator>& lhs, const TPtrRef<const _Allocator>& rhs) NOEXCEPT {
        return (lhs == rhs); // compare by address instead of value
    }
    template <typename _AllocatorOther>
    NODISCARD static bool Equals(const TPtrRef<const _Allocator>& lhs, const TPtrRef<_AllocatorOther>& rhs) NOEXCEPT {
        return allocator_traits::Equals(*lhs, *rhs); // compare by value
    }
    NODISCARD static size_t MaxSize(const TPtrRef<const _Allocator>& a) NOEXCEPT {
        return allocator_traits::MaxSize(*a);
    }
    NODISCARD static size_t SnapSize(const TPtrRef<const _Allocator>& a, size_t size) NOEXCEPT {
        return allocator_traits::SnapSize(*a, size);
    }
    template <typename T>
    NODISCARD static size_t SnapSizeT(const TPtrRef<const _Allocator>& a, size_t count) NOEXCEPT {
        return allocator_traits::template SnapSizeT<T>(*a, count);
    }
    NODISCARD static bool Owns(const TPtrRef<const _Allocator>& a, FAllocatorBlock b) NOEXCEPT {
        return allocator_traits::Owns(*a, b);
    }
    NODISCARD static FAllocatorBlock Allocate(const TPtrRef<_Allocator>& a, size_t s) {
        return allocator_traits::Allocate(*a, s);
    }
    template <typename T>
    NODISCARD static TMemoryView<T> AllocateT(const TPtrRef<_Allocator>& a, size_t n) {
        return allocator_traits::template AllocateT<T>(*a, n);
    }
    template <typename T>
    NODISCARD static T* AllocateOneT(const TPtrRef<_Allocator>& a) {
        return allocator_traits::template AllocateOneT<T>(*a);
    }
    static void Deallocate(const TPtrRef<_Allocator>& a, FAllocatorBlock b) {
        allocator_traits::Deallocate(*a, b);
    }
    template <typename T>
    static void DeallocateT(const TPtrRef<_Allocator>& a, TMemoryView<T> v) {
        return allocator_traits::template DeallocateT<T>(*a, v);
    }
    template <typename T>
    static void DeallocateT(const TPtrRef<_Allocator>& a, T* p, size_t n) {
        return allocator_traits::template DeallocateT<T>(*a, p, n);
    }
    template <typename T>
    static void DeallocateOneT(const TPtrRef<_Allocator>& a, T* p) {
        allocator_traits::DeallocateOneT(*a, p);
    }
    NODISCARD static auto Reallocate(const TPtrRef<_Allocator>& a, FAllocatorBlock& b, size_t s) {
        return allocator_traits::Reallocate(*a, b, s);
    }
    NODISCARD static bool Acquire(const TPtrRef<_Allocator>& a, FAllocatorBlock b) NOEXCEPT {
        return allocator_traits::Acquire(*a, b);
    }
    NODISCARD static bool Steal(const TPtrRef<_Allocator>& a, FAllocatorBlock b) NOEXCEPT {
        return allocator_traits::Steal(*a, b);
    }
    template <typename _AllocatorDst>
    NODISCARD static bool StealAndAcquire(_AllocatorDst* dst, const TPtrRef<_Allocator>& src, FAllocatorBlock b) NOEXCEPT {
        return allocator_traits::StealAndAcquire(dst, *src, b);
    }
#if USE_PPE_MEMORYDOMAINS
    using has_memory_tracking = typename allocator_traits::has_memory_tracking;
    NODISCARD static FMemoryTracking* TrackingData(const TPtrRef<_Allocator>& a) NOEXCEPT {
        return allocator_traits::TrackingData(*a);
    }
    NODISCARD static auto& AllocatorWithoutTracking(const TPtrRef<_Allocator>& a) NOEXCEPT {
        return allocator_traits::AllocatorWithoutTracking(a);
    }
#else
    NODISCARD static auto& AllocatorWithoutTracking(const TPtrRef<_Allocator>& a) NOEXCEPT {
        return allocator_traits::AllocatorWithoutTracking(*a);
    }
#endif
};
//----------------------------------------------------------------------------
template <typename _Allocator, EThreadBarrier _Barrier>
struct TAllocatorTraits<TThreadSafe<_Allocator, _Barrier>> {
    using allocator_type = TThreadSafe<_Allocator, _Barrier>;
    using allocator_traits = TAllocatorTraits<_Allocator>;

    using propagate_on_container_copy_assignment = typename allocator_traits::propagate_on_container_copy_assignment;
    using propagate_on_container_move_assignment = typename allocator_traits::propagate_on_container_move_assignment;
    using propagate_on_container_swap = typename allocator_traits::propagate_on_container_swap;

    using is_always_equal = typename allocator_traits::is_always_equal;

    using has_maxsize = typename allocator_traits::has_maxsize;
    using has_owns = typename allocator_traits::has_owns;
    using has_reallocate = typename allocator_traits::has_reallocate;
    using has_acquire = typename allocator_traits::has_acquire;
    using has_steal = typename allocator_traits::has_steal;

    STATIC_CONST_INTEGRAL(size_t, Alignment, allocator_traits::Alignment);

    using reallocate_can_fail = typename allocator_traits::reallocate_can_fail;

    static _Allocator& Get(allocator_type& self) NOEXCEPT { return *self.LockExclusive(); }
    static const _Allocator& Get(const allocator_type& self) NOEXCEPT { return *self.LockShared(); }

    static void Copy(allocator_type* dst, const allocator_type& src) {
        allocator_traits::Copy(dst->LockExclusive().Get(), src.LockShared());
    }
    NODISCARD static const allocator_type& SelectOnCopy(const allocator_type& other) {
        return other; // pass-through
    }
    static void Move(allocator_type* dst, allocator_type& src) {
        allocator_traits::Move(dst->LockExclusive().Get(), std::move(*src.LockExclusive()));
    }
    NODISCARD static allocator_type&& SelectOnMove(allocator_type&& rvalue) {
        return std::move(rvalue); // pass-through
    }
    static void Swap(allocator_type& lhs, allocator_type& rhs) {
        swap(*lhs.LockExclusive(), *rhs.LockExclusive()); // swap content
    }
    NODISCARD static bool Equals(const allocator_type& lhs, const allocator_type& rhs) NOEXCEPT {
        return (*lhs.LockShared() == *rhs.LockShared()); // compare by address instead of value
    }
    template <typename _AllocatorOther>
    NODISCARD static bool Equals(const allocator_type& lhs, const _AllocatorOther& rhs) NOEXCEPT {
        return allocator_traits::Equals(*lhs.LockShared(), *rhs); // compare by value
    }
    NODISCARD static size_t MaxSize(const allocator_type& a) NOEXCEPT {
        return allocator_traits::MaxSize(*a.LockShared());
    }
    NODISCARD static size_t SnapSize(const allocator_type& a, size_t size) NOEXCEPT {
        return allocator_traits::SnapSize(*a.LockShared(), size);
    }
    template <typename T>
    NODISCARD static size_t SnapSizeT(const allocator_type& a, size_t count) NOEXCEPT {
        return allocator_traits::template SnapSizeT<T>(*a.LockShared(), count);
    }
    NODISCARD static bool Owns(const allocator_type& a, FAllocatorBlock b) NOEXCEPT {
        return allocator_traits::Owns(*a.LockShared(), b);
    }
    NODISCARD static FAllocatorBlock Allocate(allocator_type& a, size_t s) {
        return allocator_traits::Allocate(*a.LockExclusive(), s);
    }
    template <typename T>
    NODISCARD static TMemoryView<T> AllocateT(allocator_type& a, size_t n) {
        return allocator_traits::template AllocateT<T>(*a.LockExclusive(), n);
    }
    template <typename T>
    NODISCARD static T* AllocateOneT(allocator_type& a) {
        return allocator_traits::template AllocateOneT<T>(*a.LockExclusive());
    }
    static void Deallocate(allocator_type& a, FAllocatorBlock b) {
        allocator_traits::Deallocate(*a.LockExclusive(), b);
    }
    template <typename T>
    static void DeallocateT(allocator_type& a, TMemoryView<T> v) {
        return allocator_traits::template DeallocateT<T>(*a.LockExclusive(), v);
    }
    template <typename T>
    static void DeallocateT(allocator_type& a, T* p, size_t n) {
        return allocator_traits::template DeallocateT<T>(*a.LockExclusive(), p, n);
    }
    template <typename T>
    static void DeallocateOneT(allocator_type& a, T* p) {
        allocator_traits::DeallocateOneT(*a.LockExclusive(), p);
    }
    NODISCARD static auto Reallocate(allocator_type& a, FAllocatorBlock& b, size_t s) {
        return allocator_traits::Reallocate(*a.LockExclusive(), b, s);
    }
    NODISCARD static bool Acquire(allocator_type& a, FAllocatorBlock b) NOEXCEPT {
        return allocator_traits::Acquire(*a.LockExclusive(), b);
    }
    NODISCARD static bool Steal(allocator_type& a, FAllocatorBlock b) NOEXCEPT {
        return allocator_traits::Steal(*a.LockExclusive(), b);
    }
    template <typename _AllocatorDst>
    NODISCARD static bool StealAndAcquire(_AllocatorDst* dst, allocator_type& src, FAllocatorBlock b) NOEXCEPT {
        return allocator_traits::StealAndAcquire(dst, *src.LockExclusive(), b);
    }
#if USE_PPE_MEMORYDOMAINS
    using has_memory_tracking = typename allocator_traits::has_memory_tracking;
    NODISCARD static FMemoryTracking* TrackingData(allocator_type& a) NOEXCEPT {
        return allocator_traits::TrackingData(*a.LockExclusive());
    }
    NODISCARD static auto& AllocatorWithoutTracking(allocator_type& a) NOEXCEPT {
        return allocator_traits::AllocatorWithoutTracking(a.LockExclusive());
    }
#else
    NODISCARD static auto& AllocatorWithoutTracking(allocator_type& a) NOEXCEPT {
        return allocator_traits::AllocatorWithoutTracking(*a.LockExclusive());
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
        Unused(dst);
        Unused(src);
        Unused(b);

        traits_t::Move(dst, std::move(src));

        return true;
    }
    else {
        return (not b || traits_t::StealAndAcquire(dst, src, b));
    }
}
//----------------------------------------------------------------------------
// Helpers for Reallocate()
//----------------------------------------------------------------------------
template <typename _Allocator, typename T>
NODISCARD auto ReallocateAllocatorBlock_AssumePOD(_Allocator& a, TMemoryView<T>& items, size_t oldSize, size_t newSize) {
    STATIC_ASSERT(Meta::has_trivial_move<T>::value);
    STATIC_ASSERT(Meta::has_trivial_destructor<T>::value);
    using traits_t = TAllocatorTraits<_Allocator>;

    Assert(oldSize >= items.size());
    Assert_NoAssume(oldSize * sizeof(T) <= traits_t::MaxSize(a));
    Assert_NoAssume(newSize * sizeof(T) <= traits_t::MaxSize(a));

    FAllocatorBlock b{ items.data(), oldSize * sizeof(T) };
    const size_t s = newSize * sizeof(T);
    if (b.SizeInBytes == s)
        return;

#if 1 // specialized to exploit user oldSize, see *HERE* bellow
    IF_CONSTEXPR(traits_t::has_reallocate::value) {
        IF_CONSTEXPR(traits_t::reallocate_can_fail::value) {
            VerifyRelease(traits_t::Reallocate(a, b, s));
        }
        else {
            traits_t::Reallocate(a, b, s);
        }
    }
    else {
        if ((!!b) & (!!s)) {
            Assert_NoAssume(traits_t::SnapSize(a, s) != traits_t::SnapSize(a, b.SizeInBytes));
            const FAllocatorBlock r = traits_t::Allocate(a, s);
            Assert_NoAssume(r);
            // *HERE* copy potentially less since we're giving the actual user size (which can be less than reserved size)
            FPlatformMemory::MemcpyLarge(r.Data, b.Data, Min(s, /*b.SizeInBytes*/items.SizeInBytes()));
            traits_t::Deallocate(a, b);
            b = r;
        }
        else {
            if (Likely(s)) {
                Assert_NoAssume(not b);
                b = traits_t::Allocate(a, s);
            }
            else {
                Assert_NoAssume(b);
                traits_t::Deallocate(a, b.Reset());
            }
        }
    }
#else
    traits_t::Reallocate(a, b, newSize);
#endif

    if (oldSize = items.size(); newSize > oldSize)
        newSize = oldSize;
    items = TMemoryView<T>(static_cast<T*>(b.Data), newSize);
}
//---------------------------------------------------------------------------
template <typename _Allocator, typename T>
void ReallocateAllocatorBlock_NonPOD(_Allocator& a, TMemoryView<T>& items, size_t oldSize, size_t newSize) {
    Assert(oldSize != newSize);
    using traits_t = TAllocatorTraits<_Allocator>;

    const FAllocatorBlock o{ items.data(), oldSize * sizeof(T) };
    if (Likely(o.SizeInBytes != newSize * sizeof(T))) {
        FAllocatorBlock b = traits_t::Allocate(a, newSize * sizeof(T));

        std::uninitialized_move(
            items.begin(), items.begin() + Min(newSize, items.size()),
            MakeCheckedIterator(static_cast<T*>(b.Data), newSize, 0));

        Meta::Destroy(items);

        traits_t::Deallocate(a, o);

        items = { static_cast<T*>(b.Data), Min(newSize, items.size()) };
    }
}
//---------------------------------------------------------------------------
template <typename _Allocator, typename T>
NODISCARD auto ReallocateAllocatorBlock(_Allocator& a, TMemoryView<T>& items, size_t oldSize, size_t newSize) {
    IF_CONSTEXPR(Meta::is_pod_v<T> || (Meta::has_trivial_move<T>::value && Meta::has_trivial_destructor<T>::value))
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
