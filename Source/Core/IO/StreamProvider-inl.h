#pragma once

#include "Core/IO/StreamProvider.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
bool IStreamReader::ReadPOD(T* pod) {
    return sizeof(T) == ReadSome(pod, sizeof(T), 1);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
bool IStreamReader::ReadArray(T(&staticArray)[_Dim]) {
    return (_Dim * sizeof(T)) == ReadSome(staticArray, sizeof(T), _Dim);
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void IStreamReader::ReadAll(RawStorage<T, _Allocator>& dst) {
    typedef typename RawStorage<T, _Allocator>::size_type size_type;
    const std::streamsize s = SizeInBytes();

    Assert(0 == (s % sizeof(T)) );
    dst.Resize_DiscardData(checked_cast<size_type>(s / sizeof(T)) );
    Assert(dst.SizeInBytes() == size_type(s));

    SeekI(0);
    Read(dst.Pointer(), s);
}

//----------------------------------------------------------------------------
template <typename T>
bool IStreamReader::ExpectPOD(const T& pod) {
    const std::streamoff off = TellI();
    typename POD_STORAGE(T) read;
    if (false == ReadPOD(&read))
        return false;

    if (*reinterpret_cast<const T*>(&read) == pod)
        return true;

    SeekI(off, SeekOrigin::Begin);
    return false;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
void IStreamWriter::WritePOD(const T& pod) {
    Write(&pod, sizeof(T));
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
void IStreamWriter::WriteArray(const T(&staticArray)[_Dim]) {
    WriteSome(staticArray, sizeof(T), _Dim);
}
//----------------------------------------------------------------------------
template <size_t _Dim>
void IStreamWriter::WriteCStr(const char (&cstr)[_Dim]) {
    WriteSome(cstr, sizeof(char), (_Dim - 1));
}
//----------------------------------------------------------------------------
template <size_t _Dim>
void IStreamWriter::WriteCStr(const wchar_t (&wcstr)[_Dim]) {
    WriteSome(wcstr, sizeof(wchar_t), (_Dim - 1));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core