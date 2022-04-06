#pragma once

#include "Core.h"

#include "Container/IntrusiveList.h"
#include "IO/TextWriter_fwd.h"
#include "Meta/PointerWFlags.h"
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

    enum EMode {
        Recursive   = 0, // will call _parent recursively for each alloc
        Isolated    = 1, // will ignore _parent when allocating
    };

    struct FSnapshot {
        i64 NumAllocs;
        i64 MinSize;
        i64 MaxSize;
        i64 TotalSize;
        i64 PeakAllocs;
        i64 PeakSize;
        i64 AccumulatedAllocs;
        i64 AccumulatedSize;
        i64 SmallAllocs;
    };

    struct PPE_CORE_API FCounters {
        counter_t NumAllocs{ 0 };
        counter_t MinSize{ CODE3264(UINT32_MAX, UINT64_MAX) };
        counter_t MaxSize{ 0 };
        counter_t TotalSize{ 0 };
        counter_t PeakAllocs{ 0 };
        counter_t PeakSize{ 0 };
        counter_t AccumulatedAllocs{ 0 };
        std::atomic<u64> AccumulatedSize{ 0 };
        counter_t SmallAllocs{ 0 };

        FCounters() = default;

        void Allocate(size_t s);
        void Deallocate(size_t s);
        void ReleaseBatch(size_t n, size_t s);

        FSnapshot Snapshot() const;
        FSnapshot Difference(const FCounters& o) const;
        void ResetAt(const FSnapshot& snapshot);
    };

    explicit FMemoryTracking(
        const char* optionalName = "unknown",
        FMemoryTracking* parent = nullptr,
        EMode mode = Recursive ) NOEXCEPT;

    const char* Name() const { return _name; }
    size_t Level() const { return _level; }

    FMemoryTracking* Parent() { return _parent.Get(); }
    const FMemoryTracking* Parent() const { return _parent.Get(); }

    EMode Mode() const { return static_cast<EMode>(_parent.Flag01()); }

    FSnapshot User() const { return _user.Snapshot(); }
    FSnapshot System() const { return _system.Snapshot(); }
    FSnapshot Wasted() const { return _system.Difference(_user); }

    bool empty() const { return (0 == _user.NumAllocs.load(std::memory_order_relaxed)); }

    bool IsChildOf(const FMemoryTracking& other) const;

    void Allocate(size_t userSize, size_t systemSize, const FMemoryTracking* child = nullptr) NOEXCEPT;
    void Deallocate(size_t userSize, size_t systemSize, const FMemoryTracking* child = nullptr) NOEXCEPT;

    void AllocateUser(size_t size, const FMemoryTracking* child = nullptr) NOEXCEPT;
    void DeallocateUser(size_t size, const FMemoryTracking* child = nullptr) NOEXCEPT;

    void AllocateSystem(size_t size, const FMemoryTracking* child = nullptr) NOEXCEPT;
    void DeallocateSystem(size_t size, const FMemoryTracking* child = nullptr) NOEXCEPT;

    void ReleaseBatch(size_t numAllocs, size_t userTotal, size_t systemTotal, const FMemoryTracking* child = nullptr) NOEXCEPT;

    void ReleaseBatchUser(size_t numAllocs, size_t totalSize, const FMemoryTracking* child = nullptr) NOEXCEPT;
    void ReleaseBatchSystem(size_t numAllocs, size_t totalSize, const FMemoryTracking* child = nullptr) NOEXCEPT;

    // used by linear heaps :
    void ReleaseAllUser(const FMemoryTracking* child = nullptr) NOEXCEPT;

    // should be called only *before* registration!
    void Reparent(const char* newName, FMemoryTracking* parent, EMode mode = Recursive) NOEXCEPT {
        _name = newName;
        _parent.Reset(parent, static_cast<uintptr_t>(mode));
        _level = (parent ? parent->_level + 1 : 0);
    }

    void MoveTo(FMemoryTracking* dst) NOEXCEPT;
    void Swap(FMemoryTracking& other) NOEXCEPT; // will swap statistics

    static FMemoryTracking& GpuMemory() NOEXCEPT;
    static FMemoryTracking& UsedMemory() NOEXCEPT;
    static FMemoryTracking& ReservedMemory() NOEXCEPT;
    static FMemoryTracking& PooledMemory() NOEXCEPT;
    static FMemoryTracking& UnaccountedMemory() NOEXCEPT;

    static FMemoryTracking* ThreadTrackingData() NOEXCEPT;
    static FMemoryTracking* SetThreadTrackingData(FMemoryTracking* trackingData) NOEXCEPT;

    struct FThreadScope : Meta::FNonCopyableNorMovable {
        FMemoryTracking& CurrentTrackingData;
        FMemoryTracking* PreviousTrackingData{ nullptr };

        explicit FThreadScope(FMemoryTracking& trackingData)
        :   CurrentTrackingData(trackingData)
        ,   PreviousTrackingData(SetThreadTrackingData(&CurrentTrackingData)) {
        }

        ~FThreadScope() {
            Verify(SetThreadTrackingData(PreviousTrackingData) == &CurrentTrackingData);
        }

        FMemoryTracking* operator ->() const NOEXCEPT {
            return std::addressof(CurrentTrackingData);
        }
    };

private:
    FCounters _user;
    FCounters _system;

    const char* _name;
    size_t _level;
    Meta::TPointerWFlags<FMemoryTracking> _parent;

public:
    TIntrusiveListNode<FMemoryTracking> Node;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
