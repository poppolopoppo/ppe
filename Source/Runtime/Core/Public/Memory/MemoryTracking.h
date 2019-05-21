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
        size_t MinSize;
        size_t MaxSize;
        size_t PeakSize;
        size_t TotalSize;
    };

    explicit FMemoryTracking(
        const char* optionalName = "unknown",
        FMemoryTracking* parent = nullptr );

    const char* Name() const { return _name; }
    size_t Level() const { return _level; }

    FMemoryTracking* Parent() { return _parent; }
    const FMemoryTracking* Parent() const { return _parent; }

    size_t NumAllocs() const { return _numAllocs; }
    size_t PeakAllocs() const { return _peakAllocs; }

    FSnapshot User() const { return _user.Snapshot(); }
    FSnapshot System() const { return _system.Snapshot(); }
    FSnapshot Wasted() const { return _system.Substract(_user); }

    bool IsChildOf(const FMemoryTracking& other) const;

    void Allocate(size_t userSize, size_t systemSize) NOEXCEPT;
    void Deallocate(size_t userSize, size_t systemSize) NOEXCEPT;

    // used by linear heaps :
    void ReleaseAll() NOEXCEPT;
    void ReleaseBatch(size_t numAllocs, size_t userTotal, size_t systemTotal) NOEXCEPT;

    static FMemoryTracking& UsedMemory();
    static FMemoryTracking& ReservedMemory();
    static FMemoryTracking& PooledMemory();

private:
    using counter_t = std::atomic<size_t>;

    struct FCounters_ {
        counter_t MinSize{ CODE3264(UINT32_MAX, UINT64_MAX) };
        counter_t MaxSize{ 0 };
        counter_t PeakSize{ 0 };
        counter_t TotalSize{ 0 };

        FCounters_() = default;

        void Allocate(size_t s);
        void Deallocate(size_t s);
        void ReleaseBatch(size_t s);

        FSnapshot Snapshot() const;
        FSnapshot Substract(const FCounters_& o) const;
    };

    counter_t _numAllocs;
    counter_t _peakAllocs;

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
