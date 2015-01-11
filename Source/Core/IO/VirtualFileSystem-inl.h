#pragma once

#include "Core/IO/VirtualFileSystem.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool VirtualFileSystemRoot::ReadAll(const Filename& filename, RawStorage<T, _Allocator>& storage, AccessPolicy::Mode policy/* = AccessPolicy::None */) {
    policy = static_cast<AccessPolicy::Mode>(policy|AccessPolicy::Binary);
    const UniquePtr<IVirtualFileSystemIStream> istream = OpenReadable(filename, policy);

    if (!istream)
        return false;

    istream->ReadAll(storage);
    return true;
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool VirtualFileSystemRoot::WriteAll(const Filename& filename, const RawStorage<T, _Allocator>& storage, AccessPolicy::Mode policy /* = AccessPolicy::None */) {
    policy = static_cast<AccessPolicy::Mode>(policy|AccessPolicy::Binary);
    const UniquePtr<IVirtualFileSystemOStream> ostream = OpenWritable(filename, policy);

    if (!ostream)
        return false;

    ostream->Write(storage.Pointer(), storage.SizeInBytes());
    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
