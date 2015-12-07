#pragma once

#include "Core/IO/VirtualFileSystem.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool VirtualFileSystemRoot::ReadAll(const Filename& filename, RawStorage<T, _Allocator>& storage, AccessPolicy::Mode policy/* = AccessPolicy::None */) {
    const UniquePtr<IVirtualFileSystemIStream> istream = OpenReadable(filename, policy);
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
bool VirtualFileSystemRoot::WriteAll(const Filename& filename, const RawStorage<T, _Allocator>& storage, AccessPolicy::Mode policy /* = AccessPolicy::None */) {
    WriteAll(filename, storage.MakeConstView().Cast<const u8>(), policy);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
