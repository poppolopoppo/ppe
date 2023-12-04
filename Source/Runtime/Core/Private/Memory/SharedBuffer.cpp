// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Memory/SharedBuffer.h"

#include "Allocator/TrackingMalloc.h"
#include "HAL/PlatformMemory.h"
#include "Memory/MemoryTracking.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
class FBufferOwnerHeap_ final : public FBufferOwner {
public:
    explicit FBufferOwnerHeap_(u64 sizeInBytes)
#if USE_PPE_MEMORYDOMAINS
    :   _trackingData(FMemoryTracking::ThreadTrackingDataOr(MEMORYDOMAIN_TRACKING_DATA(SharedBuffer)))
    {
        const FMemoryTracking::FThreadScope trackingScope(_trackingData);

        const FAllocatorBlock alloc = PPE::malloc_for_new(checked_cast<size_t>(sizeInBytes));

        SetFlags(EBufferOwnerFlags::Materialized | EBufferOwnerFlags::Owned);
        SetBuffer(alloc.Data, sizeInBytes/* keep user size here */);

        _allocSizeForDebug = alloc.SizeInBytes;
        trackingScope->Allocate(sizeInBytes, _allocSizeForDebug);
    }
#else
    :   FBufferOwner(PPE::malloc(checked_cast<size_t>(sizeInBytes)), sizeInBytes,
            EBufferOwnerFlags::Materialized | EBufferOwnerFlags::Owned)
    {}
#endif

private:
    virtual void FreeBuffer() override final {
#if USE_PPE_MEMORYDOMAINS
        const FMemoryTracking::FThreadScope trackingScope(_trackingData);

        FAllocatorBlock alloc = FAllocatorBlock::From(FBufferOwner::MakeView());
        trackingScope->Deallocate(alloc.SizeInBytes, _allocSizeForDebug);

        alloc.SizeInBytes = _allocSizeForDebug;
        PPE::free_for_delete(alloc);

#else
        PPE::free(FBufferOwner::Data());
#endif
    }

#if USE_PPE_MEMORYDOMAINS
    FMemoryTracking& _trackingData;
    size_t _allocSizeForDebug{ 0 };
#endif
};
//----------------------------------------------------------------------------
class FBufferOwnerView_ final : public FBufferOwner {
public:
    FBufferOwnerView_(void* data, u64 sizeInBytes) NOEXCEPT
    :   FBufferOwner(data, sizeInBytes, EBufferOwnerFlags::Materialized) {

    }
    FBufferOwnerView_(const void* data, u64 sizeInBytes) NOEXCEPT
    :   FBufferOwnerView_(const_cast<void*>(data), sizeInBytes) {
        AppendFlags(EBufferOwnerFlags::Immutable);
    }

private:
    virtual void FreeBuffer() override final {
        NOOP();
    }
};
//----------------------------------------------------------------------------
class FBufferOwnerOuterView_ final : public FBufferOwner {
public:
    template <typename _OuterBuffer, decltype(FSharedBuffer(std::declval<_OuterBuffer>()))* = nullptr>
    FBufferOwnerOuterView_(void* data, u64 sizeInBytes, _OuterBuffer&& outerBuffer) NOEXCEPT
    :   FBufferOwner(data, sizeInBytes, EBufferOwnerFlags::Immutable | EBufferOwnerFlags::Materialized)
    ,   _outerBuffer(std::forward<_OuterBuffer>(outerBuffer)) {
        Assert_NoAssume(MakeRawView(FBufferOwner::Data(), FBufferOwner::SizeInBytes()).IsSubRangeOf(_outerBuffer.MakeView()));

        if (_outerBuffer.IsOwned())
            AppendFlags(EBufferOwnerFlags::Owned);
    }

private:
    virtual void FreeBuffer() override final {
        _outerBuffer.Reset();
    }

    FSharedBuffer _outerBuffer;
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FUniqueBuffer::FUniqueBuffer(FBufferOwner* owner) NOEXCEPT
:   _owner(owner) {

}
//----------------------------------------------------------------------------
FUniqueBuffer::FUniqueBuffer(PBufferOwner&& sharedOwner) NOEXCEPT
:   _owner(std::move(sharedOwner)) {

}
//----------------------------------------------------------------------------
void FUniqueBuffer::Materialize() {
    if (FBufferOwner* const pOwner = _owner.get(); !!pOwner)
        pOwner->Materialize();
}
//----------------------------------------------------------------------------
void FUniqueBuffer::Reset() {
    _owner.reset();
}
//----------------------------------------------------------------------------
FUniqueBuffer FUniqueBuffer::MakeOwned() && {
    return (IsOwned() ? std::move(*this) : Clone(MakeView()));
}
//----------------------------------------------------------------------------
FSharedBuffer FUniqueBuffer::MoveToShared() {
    return FSharedBuffer(MakeBufferOwner(std::move(*this)));
}
//----------------------------------------------------------------------------
FUniqueBuffer FUniqueBuffer::Allocate(u64 sizeInBytes) {
    return FUniqueBuffer{ NEW_REF(SharedBuffer, FBufferOwnerHeap_, sizeInBytes) };
}
//----------------------------------------------------------------------------
FUniqueBuffer FUniqueBuffer::Clone(FRawMemoryConst view) {
    FUniqueBuffer buffer = Allocate(view.SizeInBytes());
#if 0
    view.CopyTo(buffer.MakeView());
#else
    Assert_NoAssume(buffer.SizeInBytes() == view.SizeInBytes());
    FPlatformMemory::Memcpy(buffer.Data(), view.data(), view.SizeInBytes());
#endif
    return buffer;
}
//----------------------------------------------------------------------------
FUniqueBuffer FUniqueBuffer::Clone(const void* data, u64 sizeInBytes) {
    return Clone(MakeRawView(data, checked_cast<size_t>(sizeInBytes)));
}
//----------------------------------------------------------------------------
FUniqueBuffer FUniqueBuffer::MakeView(FRawMemory mutableView) {
    return MakeView(mutableView.data(), mutableView.SizeInBytes());
}
//----------------------------------------------------------------------------
FUniqueBuffer FUniqueBuffer::MakeView(void* data, u64 sizeInBytes) {
    return FUniqueBuffer(NEW_REF(SharedBuffer, FBufferOwnerView_, data, sizeInBytes));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FSharedBuffer::FSharedBuffer(FBufferOwner* owner) NOEXCEPT
:   _owner(owner) {
}
//----------------------------------------------------------------------------
FSharedBuffer::FSharedBuffer(PBufferOwner&& sharedOwner) NOEXCEPT
:   _owner(std::move(sharedOwner)) {
}
//----------------------------------------------------------------------------
FSharedBuffer::FSharedBuffer(const WBufferOwner& weakOwner) NOEXCEPT
:   _owner(weakOwner.Pin()) {
}
//----------------------------------------------------------------------------
void FSharedBuffer::Materialize() {
    if (FBufferOwner* const pOwner = _owner.get(); !!pOwner)
        pOwner->Materialize();
}
//----------------------------------------------------------------------------
void FSharedBuffer::Reset() {
    _owner.reset();
}
//----------------------------------------------------------------------------
FSharedBuffer FSharedBuffer::MakeOwned() const & {
    return (IsOwned() ? *this : Clone(MakeView()));
}
//----------------------------------------------------------------------------
FSharedBuffer FSharedBuffer::MakeOwned() && {
    return (IsOwned() ? std::move(*this) : Clone(MakeView()));
}
//----------------------------------------------------------------------------
FUniqueBuffer FSharedBuffer::MoveToUnique() {
    PBufferOwner existingOwner = MakeBufferOwner(std::move(*this));
    if (not existingOwner || existingOwner->IsUniqueOwnedMutable())
        return FUniqueBuffer(std::move(existingOwner));

    return FUniqueBuffer::Clone(existingOwner->Data(), existingOwner->SizeInBytes());
}
//----------------------------------------------------------------------------
FSharedBuffer FSharedBuffer::Clone(FRawMemoryConst view) {
    return FUniqueBuffer::Clone(view).MoveToShared();
}
//----------------------------------------------------------------------------
FSharedBuffer FSharedBuffer::Clone(const void* data, u64 sizeInBytes) {
    return FUniqueBuffer::Clone(data, sizeInBytes).MoveToShared();
}
//----------------------------------------------------------------------------
FSharedBuffer FSharedBuffer::MakeView(FRawMemoryConst view) {
    return MakeView(view.data(), view.SizeInBytes());
}
//----------------------------------------------------------------------------
FSharedBuffer FSharedBuffer::MakeView(const void* data, u64 sizeInBytes) {
    return FSharedBuffer(NEW_REF(SharedBuffer, FBufferOwnerView_, data, sizeInBytes));
}
//----------------------------------------------------------------------------
FSharedBuffer FSharedBuffer::MakeView(FRawMemoryConst view, FSharedBuffer&& outer) {
    return MakeSharedViewWithOuter_(view.data(), view.SizeInBytes(), std::move(outer));
}
//----------------------------------------------------------------------------
FSharedBuffer FSharedBuffer::MakeView(FRawMemoryConst view, const FSharedBuffer& outer) {
    return MakeSharedViewWithOuter_(view.data(), view.SizeInBytes(), outer);
}
//----------------------------------------------------------------------------
FSharedBuffer FSharedBuffer::MakeView(const void* data, u64 sizeInBytes, FSharedBuffer&& outer) {
    return MakeSharedViewWithOuter_(data, sizeInBytes, std::move(outer));
}
//----------------------------------------------------------------------------
FSharedBuffer FSharedBuffer::MakeView(const void* data, u64 sizeInBytes, const FSharedBuffer& outer) {
    return MakeSharedViewWithOuter_(data, sizeInBytes, outer);
}
//----------------------------------------------------------------------------
FSharedBuffer FSharedBuffer::MakeView(u64 offset, u64 sizeInBytes, FSharedBuffer&& outer) {
    const FRawMemoryConst view = outer.MakeView().SubRange(checked_cast<size_t>(offset), checked_cast<size_t>(sizeInBytes));
    return MakeSharedViewWithOuter_(view.data(), view.SizeInBytes(), std::move(outer));
}
//----------------------------------------------------------------------------
FSharedBuffer FSharedBuffer::MakeView(u64 offset, u64 sizeInBytes, const FSharedBuffer& outer) {
    const FRawMemoryConst view = outer.MakeView().SubRange(checked_cast<size_t>(offset), checked_cast<size_t>(sizeInBytes));
    return MakeSharedViewWithOuter_(view.data(), view.SizeInBytes(), outer);
}
//----------------------------------------------------------------------------
FSharedBuffer FSharedBuffer::MakeView(const FUniqueBuffer& outer) {
    return FSharedBuffer::MakeView(outer.MakeView());
}
//----------------------------------------------------------------------------
FSharedBuffer FSharedBuffer::MakeView(u64 offset, u64 sizeInBytes, const FUniqueBuffer& outer) {
    return FSharedBuffer::MakeView(outer.MakeView()
        .SubRange(checked_cast<size_t>(offset), checked_cast<size_t>(sizeInBytes)));
}
//----------------------------------------------------------------------------
template <typename _OuterBuffer>
NODISCARD FSharedBuffer FSharedBuffer::MakeSharedViewWithOuter_(const void* data, u64 sizeInBytes, _OuterBuffer&& outer) {
    if (not outer.IsValid())
        return FSharedBuffer::MakeView(data, sizeInBytes);

    if (const FRawMemoryConst outerView = outer.MakeView(); outerView.data() == data && outerView.SizeInBytes() == sizeInBytes)
        return std::forward<_OuterBuffer>(outer);

    return FSharedBuffer(NEW_REF(SharedBuffer, FBufferOwnerOuterView_,
        const_cast<void*>(data), sizeInBytes, std::forward<_OuterBuffer>(outer)));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWeakSharedBuffer::FWeakSharedBuffer(const FSharedBuffer& buffer) NOEXCEPT
:   _owner(MakeBufferOwner(buffer)) {
}
//----------------------------------------------------------------------------
FWeakSharedBuffer& FWeakSharedBuffer::operator=(const FSharedBuffer& buffer) NOEXCEPT {
    _owner.reset(MakeBufferOwner(buffer));
    return (*this);
}
//----------------------------------------------------------------------------
void FWeakSharedBuffer::Reset() {
    _owner.reset();
}
//----------------------------------------------------------------------------
FSharedBuffer FWeakSharedBuffer::Pin() const NOEXCEPT {
    return FSharedBuffer(_owner);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
