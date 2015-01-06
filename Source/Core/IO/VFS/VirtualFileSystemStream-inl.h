#pragma once

#include "Core/IO/VFS/VirtualFileSystemStream.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
bool IVirtualFileSystemIStream::ReadPOD(T* pod) {
    return sizeof(T) == ReadSome(pod, sizeof(T));
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
bool IVirtualFileSystemIStream::ReadArray(T(&staticArray)[_Dim]) {
    return (_Dim * sizeof(T)) == ReadSome(staticArray, _Dim * sizeof(T));
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void IVirtualFileSystemIStream::ReadAll(RawStorage<T, _Allocator>& dst) {
    typedef RawStorage<T, _Allocator>::size_type size_type;
    const std::streamsize s = Size();

    Assert(0 == (s % sizeof(T)) );
    dst.Resize_DiscardData(checked_cast<size_type>(s / sizeof(T)) );
    Assert(dst.SizeInBytes() == size_type(s));

    SeekI(0);
    Read(dst.Pointer(), s);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
void IVirtualFileSystemOStream::WritePOD(const T& pod) {
    Write(&pod, sizeof(T));
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
void IVirtualFileSystemOStream::WriteArray(const T(&staticArray)[_Dim]) {
    Write(staticArray, _Dim * sizeof(T));
}
//----------------------------------------------------------------------------
template <size_t _Dim>
void IVirtualFileSystemOStream::WriteCStr(const char (&cstr)[_Dim]) {
    Write(cstr, (_Dim - 1) * sizeof(char));
}
//----------------------------------------------------------------------------
template <size_t _Dim>
void IVirtualFileSystemOStream::WriteCStr(const wchar_t (&wcstr)[_Dim]) {
    Write(wcstr, (_Dim - 1) * sizeof(char));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
