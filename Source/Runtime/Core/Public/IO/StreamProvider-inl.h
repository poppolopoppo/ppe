#pragma once

#include "IO/StreamProvider.h"

namespace PPE {
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

    Assert_NoAssume(0 == (s % sizeof(T)) );
    dst.Resize_DiscardData(checked_cast<size_type>(s / sizeof(T)) );
    Assert_NoAssume(dst.SizeInBytes() == size_type(s));

    SeekI(0);
    Verify(Read(dst.data(), s));
}
//----------------------------------------------------------------------------
template <typename T>
bool IStreamReader::ReadAt(const TMemoryView<T>& dst, std::streamoff absolute) {
    if (dst.empty())
        return true;

    SeekI(absolute, ESeekOrigin::Begin);
    return Read(dst.data(), dst.SizeInBytes());
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool IStreamReader::ReadAt(TRawStorage<T, _Allocator>& dst, std::streamoff absolute, std::streamsize sizeInBytes) {
    if (0 == sizeInBytes)
        return true;

    Assert_NoAssume(Meta::IsAligned(sizeof(T), size_t(sizeInBytes)));
    dst.Resize_DiscardData(checked_cast<size_t>(sizeInBytes / sizeof(T)));
    Assert_NoAssume(dst.SizeInBytes() == size_t(sizeInBytes));

    SeekI(absolute, ESeekOrigin::Begin);
    return Read(dst.data(), sizeInBytes);
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
    VerifyRelease(Write(staticArray, sizeof(T)*_Dim));
}
//----------------------------------------------------------------------------
template <typename T>
void IStreamWriter::WritePOD(const T& pod) {
    if (not Write(&pod, sizeof(T)))
        AssertNotReached();
}
//----------------------------------------------------------------------------
inline void IStreamWriter::WriteView(const FStringView& str) {
    VerifyRelease(str.empty() || Write(str.data(), str.SizeInBytes()));
}
//----------------------------------------------------------------------------
inline void IStreamWriter::WriteView(const FWStringView& wstr) {
    VerifyRelease(wstr.empty() || Write(wstr.data(), wstr.SizeInBytes()));
}
//----------------------------------------------------------------------------
template <typename T>
void IStreamWriter::WriteView(const TMemoryView<T>& data) {
    VerifyRelease(data.empty() || Write(data.data(), data.SizeInBytes()));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
