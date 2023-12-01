#pragma once

#include "Memory/SharedBuffer.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline FBufferOwner::FBufferOwner(void* data, u64 sizeInBytes, EBufferOwnerFlags flags)
:   _sizeInBytes(sizeInBytes) {
    _dataWFlags.Reset(data, u8(flags));
}
//----------------------------------------------------------------------------
inline FBufferOwner::FBufferOwner(FAllocatorBlock alloc, EBufferOwnerFlags flags)
:   FBufferOwner(alloc.Data, alloc.SizeInBytes, flags) {
}
//----------------------------------------------------------------------------
inline FBufferOwner::~FBufferOwner() {
    AssertRelease_NoAssume(_dataWFlags.Ptr<void>() == nullptr && _sizeInBytes == 0);
}
//----------------------------------------------------------------------------
inline void* FBufferOwner::Data() {
    Materialize();
    return _dataWFlags.Ptr<void>();
}
//----------------------------------------------------------------------------
inline u64 FBufferOwner::SizeInBytes() {
    Materialize();
    return _sizeInBytes;
}
//----------------------------------------------------------------------------
inline bool FBufferOwner::IsUniqueOwnedMutable() const {
    return (FWeakRefCountable::RefCount() == 1 &&
        (BitAnd(Flags(), EBufferOwnerFlags::Owned | EBufferOwnerFlags::Immutable) == EBufferOwnerFlags::Owned));
}
//----------------------------------------------------------------------------
inline FRawMemory FBufferOwner::MakeView() {
    Materialize();
    return MakeRawView(_dataWFlags.Ptr<u8>(), _sizeInBytes);
}
//----------------------------------------------------------------------------
inline void FBufferOwner::Materialize() {
    if (not IsMaterialized()) {
        MaterializeBuffer();
        Assert_NoAssume(IsMaterialized());
    }
}
//----------------------------------------------------------------------------
inline void FBufferOwner::OnStrongRefCountReachZero() {
    FreeBuffer();
    SetBuffer(nullptr, 0);

    PPE::OnStrongRefCountReachZero(this);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
template <typename _Deleter>
class TBufferOwnerDeleter final : public FBufferOwner {
public:
    explicit TBufferOwnerDeleter(void* data, u64 sizeInBytes, _Deleter&& deleter)
    :   FBufferOwner(data, sizeInBytes, EBufferOwnerFlags::Owned | EBufferOwnerFlags::Materialized)
    ,   _deleter(std::move(deleter)) {
    }

    explicit TBufferOwnerDeleter(const void* data, u64 sizeInBytes, _Deleter&& deleter)
    :   TBufferOwnerDeleter(data, sizeInBytes, std::forward<_Deleter>(deleter)) {
        AppendFlags(EBufferOwnerFlags::Immutable);
    }

private:
    virtual void FreeBuffer() override final {
        const FRawMemory view = MakeView();
        _deleter(view.data(), view.SizeInBytes());
    }

    Meta::TDecay<_Deleter> _deleter;
};
template <typename _Deleter>
TBufferOwnerDeleter(void* data, u64 sizeInBytes, _Deleter&& deleter) -> TBufferOwnerDeleter<_Deleter>;
template <typename _Deleter>
TBufferOwnerDeleter(const void* data, u64 sizeInBytes, _Deleter&& deleter) -> TBufferOwnerDeleter<_Deleter>;
//----------------------------------------------------------------------------
} //!details
//----------------------------------------------------------------------------
template <typename _Deleter,
    decltype(std::declval<_Deleter>()(std::declval<void*>()))* >
FUniqueBuffer FUniqueBuffer::TakeOwn(void* data, u64 sizeInBytes, _Deleter&& deleter) NOEXCEPT {
    auto sizedDeleter = [deleter{std::forward<_Deleter>(deleter)}](void* data, u64 sizeInBytes) -> void {
        Unused(sizeInBytes);
        return deleter(data);
    };
    using owner_type = details::TBufferOwnerDeleter<Meta::TDecay<decltype(sizedDeleter)>>;
    return FUniqueBuffer(NEW_REF(SharedBuffer, owner_type, data, sizeInBytes,
        std::move(sizedDeleter)));
}
//----------------------------------------------------------------------------
template <typename _SizedDeleter,
    decltype(std::declval<_SizedDeleter>()(std::declval<void*>(), std::declval<u64>()))* >
FUniqueBuffer FUniqueBuffer::TakeOwn(void* data, u64 sizeInBytes, _SizedDeleter&& deleter) NOEXCEPT {
    using owner_type = details::TBufferOwnerDeleter<_SizedDeleter>;
    return FUniqueBuffer(NEW_REF(SharedBuffer, owner_type, data, sizeInBytes,
        std::forward<_SizedDeleter>(deleter)));
}
//----------------------------------------------------------------------------
template <typename _Deleter,
    decltype(std::declval<_Deleter>()(std::declval<void*>()))* >
FSharedBuffer FSharedBuffer::TakeOwn(const void* data, u64 sizeInBytes, _Deleter&& deleter) NOEXCEPT {
    auto sizedDeleter = [deleter{std::forward<_Deleter>(deleter)}](void* data, u64 sizeInBytes) -> void {
        Unused(sizeInBytes);
        return deleter(data);
    };
    using owner_type = details::TBufferOwnerDeleter<Meta::TDecay<decltype(sizedDeleter)>>;
    return FSharedBuffer(NEW_REF(SharedBuffer, owner_type, data, sizeInBytes, std::move(sizedDeleter)));
}
//----------------------------------------------------------------------------
template <typename _SizedDeleter,
    decltype(std::declval<_SizedDeleter>()(std::declval<void*>(), std::declval<u64>()))* >
FSharedBuffer FSharedBuffer::TakeOwn(const void* data, u64 sizeInBytes, _SizedDeleter&& deleter) NOEXCEPT {
    using owner_type = details::TBufferOwnerDeleter<_SizedDeleter>;
    return FSharedBuffer(NEW_REF(SharedBuffer, owner_type, data, sizeInBytes,
        std::forward<_SizedDeleter>(deleter)));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
class TBufferOwnerRawStorage final : public FBufferOwner {
public:
    using storage_type = TRawStorage<T, _Allocator>;

    explicit TBufferOwnerRawStorage(storage_type&& rstorage)
    :   FBufferOwner()
    ,   _storage(std::move(rstorage)) {
        SetBuffer(_storage.data(), _storage.SizeInBytes());
        AppendFlags(EBufferOwnerFlags::Materialized | EBufferOwnerFlags::Owned);
    }

private:
    virtual void FreeBuffer() override final {
        _storage.clear_ReleaseMemory();
    }

    storage_type _storage;
};
//----------------------------------------------------------------------------
} //!details
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
NODISCARD inline FSharedBuffer MakeSharedRawStorage(TRawStorage<T, _Allocator>&& rstorage) {
    using owner_type = details::TBufferOwnerRawStorage<T, _Allocator>;
    return FSharedBuffer(NEW_REF(SharedBuffer, owner_type, std::move(rstorage)));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
