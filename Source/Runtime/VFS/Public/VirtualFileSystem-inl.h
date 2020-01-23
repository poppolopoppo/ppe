#pragma once

#include "VirtualFileSystem.h"

#include "Memory/Compression.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool FVirtualFileSystem::ReadAll(const FFilename& filename, TRawStorage<T, _Allocator>& storage, EAccessPolicy policy/* = EAccessPolicy::None */) {
    bool needDecompress = false;
    if (policy ^ EAccessPolicy::Compress) {
        needDecompress = true;
        policy = policy - EAccessPolicy::Compress + EAccessPolicy::Binary;
    }

    policy = policy
        + EAccessPolicy::Sequential // we're going to make one read only, fully sequential
        ;

    const UStreamReader reader = Get().OpenReadable(filename, policy);

    bool succeed = false;
    if (reader) {
        reader->ReadAll(storage);

        if (needDecompress) {
            TRawStorage<u8, _Allocator> uncompressed{ TAllocatorTraits<_Allocator>::SelectOnCopy(storage.get_allocator()) };
            succeed = Compression::DecompressMemory(uncompressed, storage.MakeConstView().template Cast<const u8>());
            swap(storage, uncompressed);
        }
        else {
            succeed = true;
        }
    }

    return succeed;
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool FVirtualFileSystem::WriteAll(const FFilename& filename, const TRawStorage<T, _Allocator>& storage, EAccessPolicy policy /* = EAccessPolicy::None */) {
    return WriteAll(filename, storage.MakeConstView().template Cast<const u8>(), policy);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
