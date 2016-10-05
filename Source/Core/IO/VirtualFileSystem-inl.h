#pragma once

#include "Core/IO/VirtualFileSystem.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool FVirtualFileSystem::ReadAll(const FFilename& filename, TRawStorage<T, _Allocator>& storage, AccessPolicy::EMode policy/* = AccessPolicy::None */) {
    const TUniquePtr<IVirtualFileSystemIStream> istream = Instance().OpenReadable(filename, policy);
    if (istream) {
        istream->ReadAll(storage);
        return true;
    }
    else {
        return false;
    }
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool FVirtualFileSystem::WriteAll(const FFilename& filename, const TRawStorage<T, _Allocator>& storage, AccessPolicy::EMode policy /* = AccessPolicy::None */) {
    WriteAll(filename, storage.MakeConstView().Cast<const u8>(), policy);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
