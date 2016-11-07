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
template <typename T>
bool IStreamReader::ExpectPOD(const T& pod) {
    const std::streamoff off = TellI();
    typename POD_STORAGE(T) read;
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
template <typename T>
void IStreamWriter::WritePOD(const T& pod) {
    if (not Write(&pod, sizeof(T)) )
        AssertNotReached();
}
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
template <typename _Char, typename _Traits>
std::streamoff TBasicStreamReader<_Char, _Traits>::TellI() const {
    Assert(!_iss.bad());
    return _iss.tellg();
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
bool TBasicStreamReader<_Char, _Traits>::SeekI(std::streamoff offset, ESeekOrigin origin/* = ESeekOrigin::Begin */) {
    Assert(!_iss.bad());
    _iss.seekg(offset, int(origin));
    Assert(!_iss.bad());
    return true;
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
std::streamsize TBasicStreamReader<_Char, _Traits>::SizeInBytes() const {
    Assert(!_iss.bad());
    const std::streamoff off = _iss.tellg();
    _iss.seekg(0, int(ESeekOrigin::End));
    const std::streamsize sz(_iss.tellg());
    _iss.seekg(off, int(ESeekOrigin::Begin));
    return sz;
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
bool TBasicStreamReader<_Char, _Traits>::Read(void* storage, std::streamsize sizeInBytes) {
    Assert(!_iss.bad());
    _iss.read((_Char*)storage, sizeInBytes/(sizeof(_Char)));
    return (false == _iss.bad());
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
size_t TBasicStreamReader<_Char, _Traits>::ReadSome(void* storage, size_t eltsize, size_t count) {
    Assert(!_iss.bad());
    return checked_cast<size_t>(_iss.readsome((_Char*)storage, std::streamsize((count*eltsize)/(sizeof(_Char))) ) / sizeof(_Char) );
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
bool TBasicStreamReader<_Char, _Traits>::Peek(char& ch) {
    Assert(!_iss.bad());
    const auto read = _iss.peek();
    ch = char(read);
    return (read != Eof_);
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
bool TBasicStreamReader<_Char, _Traits>::Peek(wchar_t& wch) {
    Assert(!_iss.bad());
    const auto read = _iss.peek();
    wch = wchar_t(read);
    return (read != Eof_);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
std::streamoff TBasicStreamWriter<_Char, _Traits>::TellO() const {
    Assert(!_oss.bad());
    return _oss.tellp();
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
bool TBasicStreamWriter<_Char, _Traits>::SeekO(std::streamoff offset, ESeekOrigin policy/* = ESeekOrigin::Begin */) {
    Assert(!_oss.bad());
    _oss.seekp(offset, int(policy));
    Assert(!_oss.bad());
    return true;
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
bool TBasicStreamWriter<_Char, _Traits>::Write(const void* storage, std::streamsize sizeInBytes) {
    Assert(!_oss.bad());
    _oss.write((const _Char*)storage, (sizeInBytes)/(sizeof(_Char)) );
    return (false == _oss.bad());
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
size_t TBasicStreamWriter<_Char, _Traits>::WriteSome(const void* storage, size_t eltsize, size_t count) {
    Assert(!_oss.bad());
    _oss.write((const _Char*)storage, (eltsize*count)/(sizeof(_Char)) );
    return (_oss.bad() ? 0 : eltsize);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
