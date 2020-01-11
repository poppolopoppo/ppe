#pragma once

#include "Core.h"

#include "Container/IntrusiveList.h"
#include "IO/TextWriter_fwd.h"
#include "Meta/TypeTraits.h"

namespace PPE {
template <typename T>
class TMemoryView;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_CORE_API FMemoryTracking {
public:
    using counter_t = std::atomic<size_t>;

    struct FSnapshot {
        i64 NumAllocs;
        i64 MinSize;
        i64 MaxSize;
        i64 TotalSize;
        i64 PeakAllocs;
        i64 PeakSize;
        i64 AccumulatedAllocs;
        i64 AccumulatedSize;
    };

    explicit FMemoryTracking(
        const char* optionalName = "unknown",
        FMemoryTracking* parent = nullptr );

    const char* Name() const { return _name; }
    size_t Level() const { return _level; }

    FMemoryTracking* Parent() { return _parent; }
    const FMemoryTracking* Parent() const { return _parent; }

    FSnapshot User() const { return _user.Snapshot(); }
    FSnapshot System() const { return _system.Snapshot(); }
    FSnapshot Wasted() const { return _system.Substract(_user); }

    bool empty() const { return (0 == _user.NumAllocs); }

    bool IsChildOf(const FMemoryTracking& other) const;

    void Allocate(size_t userSize, size_t systemSize) NOEXCEPT;
    void Deallocate(size_t userSize, size_t systemSize) NOEXCEPT;

    void AllocateUser(size_t size) NOEXCEPT;
    void DeallocateUser(size_t size) NOEXCEPT;

    void AllocateSystem(size_t size) NOEXCEPT;
    void DeallocateSystem(size_t size) NOEXCEPT;

    void ReleaseBatch(size_t numAllocs, size_t userTotal, size_t systemTotal) NOEXCEPT;

    void ReleaseBatchUser(size_t numAllocs, size_t totalSize) NOEXCEPT;
    void ReleaseBatchSystem(size_t numAllocs, size_t totalSize) NOEXCEPT;

    // used by linear heaps :
    void ReleaseAllUser() NOEXCEPT;

    static FMemoryTracking& UsedMemory();
    static FMemoryTracking& ReservedMemory();
    static FMemoryTracking& PooledMemory();

private:
    struct FCounters_ {
        counter_t NumAllocs{ 0 };
        counter_t MinSize{ CODE3264(UINT32_MAX, UINT64_MAX) };
        counter_t MaxSize{ 0 };
        counter_t TotalSize{ 0 };
        counter_t PeakAllocs{ 0 };
        counter_t PeakSize{ 0 };
        counter_t AccumulatedAllocs{ 0 };
        std::atomic<u64> AccumulatedSize{ 0 };

        FCounters_() = default;

        void Allocate(size_t s);
        void Deallocate(size_t s);
        void ReleaseBatch(size_t n, size_t s);

        FSnapshot Snapshot() const;
        FSnapshot Substract(const FCounters_& o) const;
    };

    FCounters_ _user;
    FCounters_ _system;

    const char* _name;
    size_t _level;
    FMemoryTracking* _parent;

public:
    TIntrusiveListNode<FMemoryTracking> Node;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
