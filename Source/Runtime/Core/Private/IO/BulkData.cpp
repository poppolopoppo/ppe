// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "IO/BulkData.h"

#include <ios>

#include "Memory/HashFunctions.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
static constexpr u128 GBulkDataFingerprintSeed{
    0x32ecd8b625352e5a_u64,
    0xbd6807eec8529c78_u64
};
//----------------------------------------------------------------------------
void FBulkData::AttachSourceFile(const FFilename& sourceFile) {
    Assert_NoAssume(not sourceFile.empty());

    _internal.LockExclusive()->SourceFile = sourceFile;
}
//----------------------------------------------------------------------------
u32 FBulkData::AttachPayload(FBytesRange range) {
    const auto exclusive = _internal.LockExclusive();

    const u32 payloadIndex = checked_cast<u32>(exclusive->Payloads.size());
    exclusive->Payloads.push_back(range);
    return payloadIndex;
}
//----------------------------------------------------------------------------
FBytesRange FBulkData::RetrievePayload(u32 payloadIndex) const {
    return _internal.LockShared()->Payloads[payloadIndex];
}
//----------------------------------------------------------------------------
void FBulkData::Resize_DiscardData(u64 sizeInBytes) {
    MEMORYDOMAIN_THREAD_SCOPE(BulkData);
    _internal.LockExclusive()->Data = FUniqueBuffer::Allocate(sizeInBytes);
}
//----------------------------------------------------------------------------
void FBulkData::Reset() {
    const auto locked = _internal.LockExclusive();

    locked->Fingerprint = Default;
    locked->Data.Reset();
    locked->Payloads.clear_ReleaseMemory();
    locked->SourceFile.reset();
}
//----------------------------------------------------------------------------
void FBulkData::Reset(FBulkData&& rvalue) {
    const auto source = rvalue._internal.LockExclusive();
    const auto dest = _internal.LockExclusive();

    dest->Fingerprint = std::move(source->Fingerprint);
    dest->Data = std::move(source->Data);
    dest->Payloads = std::move(source->Payloads);
    dest->SourceFile = std::move(source->SourceFile);

    source->Fingerprint = Default;
}
//----------------------------------------------------------------------------
FSharedBuffer FBulkData::LockRead() const NOEXCEPT {
    VerifyRelease(_internal.AcquireReader());
    return FSharedBuffer::MakeView(_internal.Value_Unsafe().Data);
}
//----------------------------------------------------------------------------
void FBulkData::UnlockRead(FSharedBuffer&& data) const NOEXCEPT {
    data.Reset(); // release reader view, nothing to do here

    _internal.ReleaseReader();
}
//----------------------------------------------------------------------------
FUniqueBuffer FBulkData::LockWrite() NOEXCEPT {
    VerifyRelease(_internal.AcquireWriter());
    return std::move(_internal.Value_Unsafe().Data); // abandon buffer ownership while writing
}
//----------------------------------------------------------------------------
void FBulkData::UnlockWrite(FUniqueBuffer&& data) NOEXCEPT {
    FInternalData& unsafe = _internal.Value_Unsafe();

    unsafe.Data = std::move(data); // restore write buffer with given buffer, could be differente than one returned by LockWrite()

    if (unsafe.Payloads.empty())
        unsafe.Payloads.emplace_back(0_size_t, unsafe.Data.SizeInBytes());

#if USE_PPE_ASSERT_RELEASE
    const FBytesRange wholeRange{ 0_size_t, unsafe.Data.SizeInBytes() };
    for (const FBytesRange& payload : unsafe.Payloads)
        AssertRelease_NoAssume(wholeRange.Contains(payload));
#endif

    unsafe.Fingerprint = Fingerprint128(unsafe.Payloads.MakeView(), GBulkDataFingerprintSeed);
    unsafe.Fingerprint = Fingerprint128(unsafe.Data.MakeView(), unsafe.Fingerprint);

    _internal.ReleaseWriter();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //! namespace PPE
