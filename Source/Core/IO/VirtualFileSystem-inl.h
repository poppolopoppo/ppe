#pragma once

#include "Core/IO/VirtualFileSystem.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void VirtualFileSystemRoot::ReadAll(const Filename& filename, RawStorage<T, _Allocator>& storage, AccessPolicy::Mode policy/* = AccessPolicy::None */) {
    policy = static_cast<AccessPolicy::Mode>(policy|AccessPolicy::Binary);
    const UniquePtr<IVirtualFileSystemIStream> istream = OpenReadable(filename, policy);
    Assert(istream);
    istream->ReadAll(storage);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void VirtualFileSystemRoot::WriteAll(const Filename& filename, const RawStorage<T, _Allocator>& storage, AccessPolicy::Mode policy /* = AccessPolicy::None */) {
    policy = static_cast<AccessPolicy::Mode>(policy|AccessPolicy::Binary);
    const UniquePtr<IVirtualFileSystemOStream> ostream = OpenWritable(filename, policy);
    Assert(ostream);
    ostream->Write(storage.Pointer(), storage.SizeInBytes());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
