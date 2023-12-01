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

    NODISCARD const u128& Fingerprint() const { return _internal.LockShared()->Fingerprint; }
    NODISCARD const Meta::TOptional<FFilename>& SourceFile() const { return _internal.LockShared()->SourceFile; }

    NODISCARD bool HasPayloads() const { return (not _internal.LockShared()->Payloads.empty()); }
    NODISCARD PPE_CORE_API u32 AttachPayload(FBytesRange range);
    NODISCARD PPE_CORE_API FBytesRange RetrievePayload(u32 payloadIndex) const;

    PPE_CORE_API void AttachSourceFile(const FFilename& sourceFile);

    PPE_CORE_API void Resize_DiscardData(u64 sizeInBytes);
    PPE_CORE_API void Reset();
    PPE_CORE_API void Reset(FBulkData&& rvalue);

    NODISCARD PPE_CORE_API FSharedBuffer LockRead() const NOEXCEPT;
    PPE_CORE_API void UnlockRead(FSharedBuffer&& data) const NOEXCEPT;

    NODISCARD PPE_CORE_API FUniqueBuffer LockWrite() NOEXCEPT;
    PPE_CORE_API void UnlockWrite(FUniqueBuffer&& data) NOEXCEPT;

    struct FReadScope : Meta::FNonCopyableNorMovable {
        explicit FReadScope(const FBulkData& bulk)  NOEXCEPT : Bulk(bulk) {
            Buffer = Bulk.LockRead();
        }

        ~FReadScope() NOEXCEPT {
            Bulk.UnlockRead(std::move(Buffer));
        }

        const FBulkData& Bulk;
        FSharedBuffer Buffer;
    };

    struct FWriteScope : Meta::FNonCopyableNorMovable {
        explicit FWriteScope(FBulkData& bulk) NOEXCEPT : Bulk(bulk) {
            Buffer = Bulk.LockWrite();
        }

        ~FWriteScope() NOEXCEPT {
            Bulk.UnlockWrite(std::move(Buffer));
        }

        FBulkData& Bulk;
        FUniqueBuffer Buffer;
    };

private:
    TThreadSafe<FInternalData, EThreadBarrier::RWDataRaceCheck> _internal;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
