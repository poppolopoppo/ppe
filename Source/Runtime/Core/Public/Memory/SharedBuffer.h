#pragma once

#include "Core_fwd.h"

#include "Memory/SharedBuffer_fwd.h"

#include "Memory/MemoryView.h"
#include "Memory/RefPtr.h"
#include "Memory/WeakPtr.h"
#include "Meta/PointerWFlags.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EBufferOwnerFlags : u8 {
    Owned           = 1 << 0,
    Immutable       = 1 << 1,
    Materialized    = 1 << 2,

    Unknown = 0,
};
ENUM_FLAGS(EBufferOwnerFlags);
//----------------------------------------------------------------------------
class FBufferOwner : public FWeakRefCountable, Meta::FNonCopyable {
public:
    FBufferOwner() = default;

    inline FBufferOwner(FAllocatorBlock alloc, EBufferOwnerFlags flags);
    inline FBufferOwner(void* data, u64 sizeInBytes, EBufferOwnerFlags flags);

    FBufferOwner(FBufferOwner&& rvalue) NOEXCEPT {
        using std::swap;
        swap(_dataWFlags, rvalue._dataWFlags);
        swap(_sizeInBytes, rvalue._sizeInBytes);
    }

    inline virtual ~FBufferOwner();

    NODISCARD inline void* Data();
    NODISCARD inline u64 SizeInBytes();
    NODISCARD EBufferOwnerFlags Flags() const { return EBufferOwnerFlags(_dataWFlags.Counter()); }

    NODISCARD bool IsOwned() const { return (Flags() ^ EBufferOwnerFlags::Owned); }
    NODISCARD bool IsImmutable() const { return (Flags() ^ EBufferOwnerFlags::Immutable); }
    NODISCARD bool IsMaterialized() const { return (Flags() ^ EBufferOwnerFlags::Materialized); }
    NODISCARD inline bool IsUniqueOwnedMutable() const;

    NODISCARD inline FRawMemory MakeView();

    void Materialize();
    void OnStrongRefCountReachZero();

protected:
    virtual void MaterializeBuffer() {
        AppendFlags(EBufferOwnerFlags::Materialized);
    }
    virtual void FreeBuffer() = 0;

    void SetBuffer(void* data, u64 sizeInBytes) {
        _dataWFlags.SetPtr(data);
        _sizeInBytes = sizeInBytes;
    }

    void SetFlags(EBufferOwnerFlags flags) { _dataWFlags.SetCounter(static_cast<u8>(flags)); }
    void AppendFlags(EBufferOwnerFlags flags) {  SetFlags(Flags() + flags); }
    void RemoveFlags(EBufferOwnerFlags flags) { SetFlags(Flags() - flags); }

private:
    friend class FUniqueBuffer;
    friend class FSharedBuffer;
    friend class FWeakSharedBuffer;

    Meta::FHeapPtrWCounter _dataWFlags{ 0 };
    u64 _sizeInBytes{ 0 };
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FUniqueBuffer {
public:
    FUniqueBuffer() = default;

    FUniqueBuffer(const FUniqueBuffer&) = default;
    FUniqueBuffer& operator =(const FUniqueBuffer&) = default;

    FUniqueBuffer(FUniqueBuffer&&) = default;
    FUniqueBuffer& operator =(FUniqueBuffer&&) = default;

    PPE_CORE_API explicit FUniqueBuffer(FBufferOwner* owner) NOEXCEPT;

    NODISCARD void* Data() const { return (_owner ? _owner->Data() : nullptr); }
    NODISCARD u64 SizeInBytes() const { return (_owner ? _owner->SizeInBytes() : 0); }

    NODISCARD bool IsValid() const { return _owner.valid(); }
    NODISCARD bool IsOwned() const { return (not _owner || _owner->IsOwned()); }
    NODISCARD bool IsImmutable() const { return (not _owner || _owner->IsImmutable()); }
    NODISCARD bool IsMaterialized() const { return (not _owner || _owner->IsMaterialized()); }

    PPE_FAKEBOOL_OPERATOR_DECL() { return IsValid(); }

    PPE_CORE_API void Materialize();
    PPE_CORE_API void Reset();

    NODISCARD PPE_CORE_API FUniqueBuffer MakeOwned() &&/* only on ref values */;
    NODISCARD PPE_CORE_API FSharedBuffer MoveToShared();

    NODISCARD FRawMemory MakeView() { return MakeRawView(Data(), checked_cast<size_t>(SizeInBytes())); }
    NODISCARD FRawMemoryConst MakeView() const { return MakeRawView(static_cast<const void*>(Data()), checked_cast<size_t>(SizeInBytes())); }

    NODISCARD operator FRawMemory() { return MakeView(); }
    NODISCARD operator FRawMemoryConst() const { return MakeView(); }

    NODISCARD friend hash_t hash_value(const FUniqueBuffer& buffer) NOEXCEPT {
        return hash_value(buffer._owner);
    }

public:
    NODISCARD PPE_CORE_API static FUniqueBuffer Allocate(u64 sizeInBytes);

    NODISCARD PPE_CORE_API static FUniqueBuffer Clone(FRawMemoryConst view);
    NODISCARD PPE_CORE_API static FUniqueBuffer Clone(const void* data, u64 sizeInBytes);

    NODISCARD PPE_CORE_API static FUniqueBuffer MakeView(FRawMemory mutableView);
    NODISCARD PPE_CORE_API static FUniqueBuffer MakeView(void* data, u64 sizeInBytes);

    template <typename _Deleter,
        decltype(std::declval<_Deleter>()(std::declval<void*>()))* = nullptr>
    NODISCARD inline static FUniqueBuffer TakeOwn(void* data, u64 sizeInBytes, _Deleter&& deleter) NOEXCEPT;
    template <typename _SizedDeleter,
        decltype(std::declval<_SizedDeleter>()(std::declval<void*>(), std::declval<u64>()))* = nullptr>
    NODISCARD inline static FUniqueBuffer TakeOwn(void* data, u64 sizeInBytes, _SizedDeleter&& deleter) NOEXCEPT;

private:
    friend class FSharedBuffer;

    PPE_CORE_API explicit FUniqueBuffer(PBufferOwner&& sharedOwner) NOEXCEPT;

    NODISCARD friend PBufferOwner MakeBufferOwner(FUniqueBuffer&& buffer) { return std::move(buffer._owner); }
    NODISCARD friend const PBufferOwner& MakeBufferOwner(const FUniqueBuffer& buffer) { return buffer._owner; }

    PBufferOwner _owner;
};
//----------------------------------------------------------------------------
class FSharedBuffer {
public:
    FSharedBuffer() = default;

    FSharedBuffer(const FSharedBuffer&) = default;
    FSharedBuffer& operator =(const FSharedBuffer&) = default;

    FSharedBuffer(FSharedBuffer&&) = default;
    FSharedBuffer& operator =(FSharedBuffer&&) = default;

    PPE_CORE_API explicit FSharedBuffer(FBufferOwner* owner) NOEXCEPT;

    NODISCARD void* Data() const { return (_owner ? _owner->Data() : nullptr); }
    NODISCARD u64 SizeInBytes() const { return (_owner ? _owner->SizeInBytes() : 0); }

    NODISCARD bool IsValid() const { return _owner.valid(); }
    NODISCARD bool IsOwned() const { return (not _owner || _owner->IsOwned()); }
    NODISCARD bool IsImmutable() const { return (not _owner || _owner->IsImmutable()); }
    NODISCARD bool IsMaterialized() const { return (not _owner || _owner->IsMaterialized()); }

    PPE_FAKEBOOL_OPERATOR_DECL() { return IsValid(); }

    PPE_CORE_API void Materialize();
    PPE_CORE_API void Reset();

    NODISCARD PPE_CORE_API FSharedBuffer MakeOwned() const &/* only on const refs */;
    NODISCARD PPE_CORE_API FSharedBuffer MakeOwned() &&/* only on ref values */;
    NODISCARD PPE_CORE_API FUniqueBuffer MoveToUnique();

    NODISCARD FRawMemoryConst MakeView() const {
        return MakeRawView(static_cast<const void*>(Data()), checked_cast<size_t>(SizeInBytes()));
    }

    NODISCARD operator FRawMemoryConst() const { return MakeView(); }

    NODISCARD friend hash_t hash_value(const FSharedBuffer& buffer) NOEXCEPT {
        return hash_value(buffer._owner);
    }

public:
    NODISCARD PPE_CORE_API static FSharedBuffer Clone(FRawMemoryConst view);
    NODISCARD PPE_CORE_API static FSharedBuffer Clone(const void* data, u64 sizeInBytes);

    NODISCARD PPE_CORE_API static FSharedBuffer MakeView(FRawMemoryConst view);
    NODISCARD PPE_CORE_API static FSharedBuffer MakeView(const void* data, u64 sizeInBytes);

    NODISCARD PPE_CORE_API static FSharedBuffer MakeView(FRawMemoryConst view, FSharedBuffer&& outer);
    NODISCARD PPE_CORE_API static FSharedBuffer MakeView(FRawMemoryConst view, const FSharedBuffer& outer);
    NODISCARD PPE_CORE_API static FSharedBuffer MakeView(const void* data, u64 sizeInBytes, FSharedBuffer&& outer);
    NODISCARD PPE_CORE_API static FSharedBuffer MakeView(const void* data, u64 sizeInBytes, const FSharedBuffer& outer);
    NODISCARD PPE_CORE_API static FSharedBuffer MakeView(u64 offset, u64 sizeInBytes, FSharedBuffer&& outer);
    NODISCARD PPE_CORE_API static FSharedBuffer MakeView(u64 offset, u64 sizeInBytes, const FSharedBuffer& outer);

    NODISCARD PPE_CORE_API static FSharedBuffer MakeView(const FUniqueBuffer& outer);
    NODISCARD PPE_CORE_API static FSharedBuffer MakeView(u64 offset, u64 sizeInBytes, const FUniqueBuffer& outer);

    template <typename _Deleter,
        decltype(std::declval<_Deleter>()(std::declval<void*>()))* = nullptr>
    NODISCARD inline static FSharedBuffer TakeOwn(const void* data, u64 sizeInBytes, _Deleter&& deleter) NOEXCEPT;
    template <typename _SizedDeleter,
        decltype(std::declval<_SizedDeleter>()(std::declval<void*>(), std::declval<u64>()))* = nullptr>
    NODISCARD inline static FSharedBuffer TakeOwn(const void* data, u64 sizeInBytes, _SizedDeleter&& deleter) NOEXCEPT;

private:
    friend class FUniqueBuffer;
    friend class FWeakSharedBuffer;

    PPE_CORE_API explicit FSharedBuffer(PBufferOwner&& sharedOwner) NOEXCEPT;
    PPE_CORE_API explicit FSharedBuffer(const WBufferOwner& weakOwner) NOEXCEPT;

    NODISCARD friend PBufferOwner MakeBufferOwner(FSharedBuffer&& buffer) { return std::move(buffer._owner); }
    NODISCARD friend const PBufferOwner& MakeBufferOwner(const FSharedBuffer& buffer) { return buffer._owner; }

    template <typename _OuterBuffer>
    NODISCARD static FSharedBuffer MakeSharedViewWithOuter_(const void* data, u64 sizeInBytes, _OuterBuffer&& outer);

    PBufferOwner _owner;
};
//----------------------------------------------------------------------------
class FWeakSharedBuffer {
public:
    FWeakSharedBuffer() = default;

    PPE_CORE_API FWeakSharedBuffer(const FSharedBuffer& shared) NOEXCEPT;
    PPE_CORE_API FWeakSharedBuffer& operator =(const FSharedBuffer& shared) NOEXCEPT;

    PPE_CORE_API void Reset();

    NODISCARD PPE_CORE_API FSharedBuffer Pin() const NOEXCEPT;

    NODISCARD friend hash_t hash_value(const FWeakSharedBuffer& buffer) NOEXCEPT {
        return hash_value(buffer._owner);
    }

private:
    NODISCARD friend const WBufferOwner& MakeBufferOwner(const FWeakSharedBuffer& buffer) { return buffer._owner; }

    WBufferOwner _owner;
};
//----------------------------------------------------------------------------
template <typename _BufferA, typename _BufferB>
auto operator ==(const _BufferA& bufferA, const _BufferB& bufferB)
    -> decltype(MakeBufferOwner(bufferA) == MakeBufferOwner(bufferB)) {
    return (MakeBufferOwner(bufferA) == MakeBufferOwner(bufferB));
}
//----------------------------------------------------------------------------
template <typename _BufferA, typename _BufferB>
auto operator !=(const _BufferA& bufferA, const _BufferB& bufferB)
    -> decltype(MakeBufferOwner(bufferA) != MakeBufferOwner(bufferB)) {
    return (MakeBufferOwner(bufferA) != MakeBufferOwner(bufferB));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
class TRawStorage;
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
NODISCARD inline FSharedBuffer MakeSharedRawStorage(TRawStorage<T, _Allocator>&& rstorage);
//----------------------------------------------------------------------------
NODISCARD inline size_t SizeOf(const FUniqueBuffer& buffer) NOEXCEPT {
    return buffer.SizeInBytes();
}
//----------------------------------------------------------------------------
NODISCARD inline FRawMemory MakeView(FUniqueBuffer& buffer) NOEXCEPT {
    return buffer.MakeView();
}
//----------------------------------------------------------------------------
NODISCARD inline FRawMemoryConst MakeView(const FUniqueBuffer& buffer) NOEXCEPT {
    return buffer.MakeView();
}
//----------------------------------------------------------------------------
inline void Resize_DiscardData(FUniqueBuffer& buffer, size_t sizeInBytes) {
    if (buffer.SizeInBytes() != sizeInBytes)
        buffer = FUniqueBuffer::Allocate(sizeInBytes);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#include "Memory/SharedBuffer-inl.h"
