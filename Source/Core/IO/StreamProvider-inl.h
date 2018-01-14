#pragma once

#include "Core/IO/StreamProvider.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
bool IStreamReader::ReadArray(T(&staticArray)[_Dim]) {
    return (_Dim == ReadSome(staticArray, sizeof(T), _Dim));
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void IStreamReader::ReadAll(TRawStorage<T, _Allocator>& dst) {
    typedef typename TRawStorage<T, _Allocator>::size_type size_type;
    const std::streamsize s = SizeInBytes();

    Assert(0 == (s % sizeof(T)) );
    dst.Resize_DiscardData(checked_cast<size_type>(s / sizeof(T)) );
    Assert(dst.SizeInBytes() == size_type(s));

    SeekI(0);
    Read(dst.Pointer(), s);
}
//----------------------------------------------------------------------------
template <typename T>
bool IStreamReader::ReadView(const TMemoryView<T>& dst) {
    return Read(dst.data(), dst.SizeInBytes());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
bool IBufferedStreamReader::ReadPOD(T* pod) {
    return (1 == ReadSome(pod, sizeof(T), 1));
}
//----------------------------------------------------------------------------
template <typename T>
bool IBufferedStreamReader::ExpectPOD(const T& pod) {
    const std::streamoff off = TellI();
    POD_STORAGE(T) read;
    if (false == ReadPOD(&read))
        return false;

    if (*reinterpret_cast<const T*>(&read) == pod)
        return true;

    SeekI(off, ESeekOrigin::Begin);
    return false;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
void IStreamWriter::WriteArray(const T(&staticArray)[_Dim]) {
    if (not Write(staticArray, sizeof(T)*_Dim) )
        AssertNotReached();
}
//----------------------------------------------------------------------------
inline void IStreamWriter::WriteView(const FStringView& str) {
    if (not Write(str.data(), str.SizeInBytes()) )
        AssertNotReached();
}
//----------------------------------------------------------------------------
inline void IStreamWriter::WriteView(const FWStringView& wstr) {
    if (not Write(wstr.data(), wstr.SizeInBytes()) )
        AssertNotReached();
}
//----------------------------------------------------------------------------
template <typename T>
void IStreamWriter::WriteView(const TMemoryView<T>& data) {
    if (not Write(data.Pointer(), data.SizeInBytes()) )
        AssertNotReached();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
void IBufferedStreamWriter::WritePOD(const T& pod) {
    if (not Write(&pod, sizeof(T)))
        AssertNotReached();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
