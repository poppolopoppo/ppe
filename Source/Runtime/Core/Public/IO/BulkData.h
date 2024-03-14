#pragma once

#include "Core_fwd.h"

#include "Container/Vector.h"
#include "IO/Filename.h"
#include "Maths/Range.h"
#include "Memory/SharedBuffer.h"
#include "Meta/Optional.h"
#include "Thread/ThreadSafe.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Stores optional binary data payload, which can be streamed in after owner has been loaded
// #TODO: need to support chunking for streaming, payload is here but we need separated buffers and resident chunks
class FBulkData : public Meta::FNonCopyable {
public:
    struct FInternalData {
        u128 Fingerprint{};
        FUniqueBuffer Data;
        Meta::TOptional<FFilename> SourceFile;
        VECTORINSITU(BulkData, FBytesRange, 1) Payloads;
    };

    FBulkData() = default;

    FBulkData(FBulkData&& rvalue) NOEXCEPT {
        *_internal.LockExclusive() = std::move(*rvalue._internal.LockExclusive());
    }

    FBulkData& operator =(FBulkData&& rvalue) NOEXCEPT {
        *_internal.LockExclusive() = std::move(*rvalue._internal.LockExclusive());
        return (*this);
    }

    NODISCARD const u128& Fingerprint() const { return _internal.LockShared()->Fingerprint; }
    NODISCARD size_t SizeInBytes() const { return _internal.LockShared()->Data.SizeInBytes(); }
    NODISCARD const Meta::TOptional<FFilename>& SourceFile() const { return _internal.LockShared()->SourceFile; }

    NODISCARD bool HasPayloads() const { return (not _internal.LockShared()->Payloads.empty()); }
    NODISCARD PPE_CORE_API u32 AttachPayload(FBytesRange range);
    NODISCARD PPE_CORE_API FBytesRange RetrievePayload(u32 payloadIndex) const;

    PPE_CORE_API void AttachSourceFile(const FFilename& sourceFile);
    PPE_CORE_API void AttachSourceFile(const Meta::TOptional<FFilename>& optionalSourceFile);

    PPE_CORE_API void Reset();
    PPE_CORE_API void Reset(FBulkData&& rvalue);

    PPE_CORE_API void Resize_DiscardData(u64 sizeInBytes);
    PPE_CORE_API void TakeOwn(FUniqueBuffer&& data) NOEXCEPT;

    NODISCARD PPE_CORE_API FSharedBuffer LockRead() const NOEXCEPT;
    PPE_CORE_API void UnlockRead(FSharedBuffer&& data) const NOEXCEPT;

    NODISCARD PPE_CORE_API FUniqueBuffer LockWrite() NOEXCEPT;
    PPE_CORE_API void UnlockWrite(FUniqueBuffer&& data) NOEXCEPT;

private:
    TThreadSafe<FInternalData, EThreadBarrier::RWDataRaceCheck> _internal;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
