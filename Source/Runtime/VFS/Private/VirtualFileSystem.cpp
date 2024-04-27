// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "VirtualFileSystem.h"

#include "VFSModule.h"
#include "VirtualFileSystemComponent.h"
#include "VirtualFileSystemNativeComponent.h"
#include "VirtualFileSystemTrie.h"

#include "Container/RawStorage.h"
#include "Diagnostic/Logger.h"
#include "HAL/PlatformTime.h"
#include "IO/FileSystem.h"
#include "IO/Format.h"
#include "IO/StringBuilder.h"
#include "IO/StringView.h"
#include "Memory/Compression.h"
#include "Memory/SharedBuffer.h"
#include "Time/DateTime.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void* FVirtualFileSystem::class_singleton_storage() NOEXCEPT {
    return singleton_type::make_singleton_storage(); // for shared libs
}
//----------------------------------------------------------------------------
FBasename FVirtualFileSystem::TemporaryBasename(const FWStringView& prefix, const FWStringView& ext) {
    Assert(not prefix.empty());
    Assert(not ext.empty());

    const FDateTime dt = FDateTime::Now();
    STACKLOCAL_WTEXTWRITER(buffer, FileSystem::MaxPathLength);
    Format(buffer, L"{0}_{1:#4}{2:#2}{3:#2}{4:#2}{5:#2}{6:#2}_{7}",
        prefix,
        dt.Year, dt.Month, dt.Day, dt.Hours, dt.Minutes, dt.Seconds,
        ext );

    return FBasename(buffer.Written());
}
//----------------------------------------------------------------------------
FFilename FVirtualFileSystem::TemporaryFilename(const FWStringView& prefix, const FWStringView& ext) {
    Assert(not prefix.empty());
    Assert(not ext.empty());

    const FDateTime dt = FDateTime::Now();
    STACKLOCAL_WTEXTWRITER(buffer, FileSystem::MaxPathLength);
    Format(buffer, L"Tmp:/{0}_{1:#4}{2:#2}{3:#2}{4:#2}{5:#2}{6:#2}_{7}",
        prefix,
        dt.Year, dt.Month, dt.Day, dt.Hours, dt.Minutes, dt.Seconds,
        ext );

    return FFilename(buffer.Written());
}
//----------------------------------------------------------------------------
bool FVirtualFileSystem::WriteAll(const FFilename& filename, const TMemoryView<const u8>& storage, EAccessPolicy policy /* = EAccessPolicy::None */) {
    Assert(!filename.empty());

    bool needCompress = false;
    if (policy ^ EAccessPolicy::Compress) {
        needCompress = true;
        policy = policy - EAccessPolicy::Compress + EAccessPolicy::Binary;
    }

    policy = policy
        + EAccessPolicy::Sequential // we're going to make only 1 write, full sequential
        + EAccessPolicy::ShortLived // the handle will be closed immediately after write
        ;

    const UStreamWriter writer = Get().OpenWritable(filename, policy);

    if (writer) {
        if (needCompress) {
            RAWSTORAGE(Compress, u8) compressed;
            const size_t compressedSizeInBytes = Compression::CompressMemory(compressed, storage, Compression::HighCompression);
            writer->Write(compressed.Pointer(), compressedSizeInBytes);
        }
        else {
            writer->Write(storage.Pointer(), storage.SizeInBytes());
        }

        return true;
    }
    else {
        return false;
    }
}
//----------------------------------------------------------------------------
bool FVirtualFileSystem::Copy(const FFilename& dst, const FFilename& src, EAccessPolicy policy/* = EAccessPolicy::None */) {
    Assert(dst != src);
    Assert(not (policy ^ EAccessPolicy::Compress)); // use Compress() method instead

    if (src == dst)
        return true;

    const UStreamReader reader = Get().OpenReadable(src, policy - EAccessPolicy::Truncate);
    if (not reader)
        return false;

    const UStreamWriter writer = Get().OpenWritable(dst, policy + EAccessPolicy::Truncate);
    if (not writer)
        return false;

    STACKLOCAL_POD_ARRAY(u8, buffer, ALLOCATION_GRANULARITY);
    while (size_t read = reader->ReadSome(buffer.data(), 1, buffer.SizeInBytes())) {
        if (not writer->Write(buffer.data(), read)) {
            AssertNotReached();
            return false;
        }
    }

    return true;
}
//----------------------------------------------------------------------------
bool FVirtualFileSystem::Compress(const FFilename& dst, const FFilename& src, EAccessPolicy policy/* = EAccessPolicy::None */) {
    Assert(dst != src);
    Assert(not (policy ^ EAccessPolicy::Compress)); // don't specify it, implicit within this method

    if (src == dst)
        return true;

    size_t compressedSizeInBytes = 0;
    RAWSTORAGE(Compress, u8) data;
    {
        const UStreamReader reader = Get().OpenReadable(src, policy - EAccessPolicy::Truncate);
        if (not reader)
            return false;

        reader->ReadAll(data);
    }
    {
        RAWSTORAGE(Compress, u8) compressed;
        compressedSizeInBytes = Compression::CompressMemory(compressed, data.MakeConstView().Cast<const u8>());

        swap(data, compressed);
    }
    {
        const UStreamWriter writer = Get().OpenWritable(dst, policy + EAccessPolicy::Truncate + EAccessPolicy::Binary);
        if (not writer)
            return false;

        if (not writer->Write(data.Pointer(), compressedSizeInBytes))
            return false;
    }

    return true;
}
//----------------------------------------------------------------------------
bool FVirtualFileSystem::Decompress(const FFilename& dst, const FFilename& src, EAccessPolicy policy/* = EAccessPolicy::None */) {
    Assert(dst != src);
    Assert(not (policy ^ EAccessPolicy::Compress)); // don't specify it, implicit within this method

    if (src == dst)
        return true;

    RAWSTORAGE(Compress, u8) data;
    {
        const UStreamReader reader = Get().OpenReadable(src, policy - EAccessPolicy::Truncate + EAccessPolicy::Binary);
        if (not reader)
            return false;

        reader->ReadAll(data);
    }
    {
        RAWSTORAGE(Compress, u8) uncompressed;
        if (not Compression::DecompressMemory(uncompressed, data.MakeConstView().Cast<const u8>()))
            return false;

        swap(data, uncompressed);
    }
    {
        const UStreamWriter writer = Get().OpenWritable(dst, policy + EAccessPolicy::Truncate);
        if (not writer)
            return false;

        if (not writer->Write(data.Pointer(), data.SizeInBytes()))
            return false;
    }

    return true;
}
//----------------------------------------------------------------------------
FUniqueBuffer FVirtualFileSystem::ReadAll(const FFilename& filename, EAccessPolicy policy) {
    bool needDecompress = false;
    if (policy ^ EAccessPolicy::Compress) {
        needDecompress = true;
        policy = policy - EAccessPolicy::Compress + EAccessPolicy::Binary;
    }

    policy = policy
        + EAccessPolicy::Sequential // we're going to make one read only, fully sequential
        ;

    const UStreamReader reader = Get().OpenReadable(filename, policy);

    if (reader) {
        FUniqueBuffer content = FUniqueBuffer::Allocate(reader->SizeInBytes());
        PPE_LOG_CHECK(VFS, reader->ReadView(content.MakeView()) && reader->Eof());

        if (Likely(not needDecompress))
            return content;

        return Compression::DecompressBuffer(content.MoveToShared());
    }

    return Default;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
