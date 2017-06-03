#pragma once

#include "Core/IO/Stream.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits >
TBasicOCStrStreamBuffer<_Char, _Traits>::TBasicOCStrStreamBuffer(_Char* storage, std::streamsize capacity)
:   _storage(storage), _capacity(capacity) {
    Reset();
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits >
TBasicOCStrStreamBuffer<_Char, _Traits>::TBasicOCStrStreamBuffer(TBasicOCStrStreamBuffer&& rvalue)
:   parent_type(std::move(rvalue))
,   _storage(nullptr), _capacity(0) {
    std::swap(rvalue._capacity, _capacity);
    std::swap(rvalue._storage, _storage);
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits >
auto TBasicOCStrStreamBuffer<_Char, _Traits>::operator =(TBasicOCStrStreamBuffer&& rvalue) -> TBasicOCStrStreamBuffer& {
    parent_type::operator =(std::move(rvalue));
    std::swap(rvalue._capacity, _capacity);
    std::swap(rvalue._storage, _storage);
    return *this;
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits >
TBasicOCStrStreamBuffer<_Char, _Traits>::~TBasicOCStrStreamBuffer() {
    PutEOS();

    // checks that final buffer is non empty or null
    Assert( parent_type::pbase() == nullptr ||
            parent_type::pbase() <  parent_type::pptr() );

    // checks that final buffer is null terminated or null
    Assert( parent_type::pbase() == nullptr ||
            parent_type::pptr()[-1] == traits_type::to_char_type(0) );
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits >
void TBasicOCStrStreamBuffer<_Char, _Traits>::ForceEOS() {
    const _Char eos = traits_type::to_char_type(0); // End Of FString

    if (nullptr == parent_type::pbase() ||
        (parent_type::pptr() > parent_type::pbase() && eos == parent_type::pptr()[-1]) )
        return; // skip if the string is already null terminated (or write buffer null)

    // null terminate the string
    if (parent_type::pptr() == parent_type::epptr())
        _storage[_capacity - 1] = eos;
    else
        parent_type::sputc(eos);
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits >
void TBasicOCStrStreamBuffer<_Char, _Traits>::PutEOS() {
    const _Char eos = traits_type::to_char_type(0); // End Of FString

    if (nullptr == parent_type::pbase() ||
        (parent_type::pptr() > parent_type::pbase() && eos == parent_type::pptr()[-1]) )
        return; // skip if the string is already null terminated (or write buffer null)

    // null terminate the string
    parent_type::sputc(eos);
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits >
void TBasicOCStrStreamBuffer<_Char, _Traits>::RemoveEOS() {
    const _Char eos = traits_type::to_char_type(0); // End Of FString

    if (nullptr == parent_type::pbase() ||
        (parent_type::pptr() > parent_type::pbase() && eos != parent_type::pptr()[-1]) )
        return; // skip if the string is not null terminated (or write buffer null)

    // remove null terminator
    parent_type::pbump(-1);
    Assert(*parent_type::pptr() == eos);
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits >
void TBasicOCStrStreamBuffer<_Char, _Traits>::Reset() {
    Assert(nullptr != _storage || 0 == _capacity);
    parent_type::setp(_storage, _storage + _capacity);
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits >
void TBasicOCStrStreamBuffer<_Char, _Traits>::swap(TBasicOCStrStreamBuffer& other){
    parent_type::swap(other);
    std::swap(other._storage, _storage);
    std::swap(other._capacity, _capacity);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits >
TBasicICStrStreamBuffer<_Char, _Traits>::TBasicICStrStreamBuffer(const _Char* storage, std::streamsize capacity)
    : _storage(storage), _capacity(capacity) {
    Reset();
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits >
TBasicICStrStreamBuffer<_Char, _Traits>::TBasicICStrStreamBuffer(TBasicICStrStreamBuffer&& rvalue)
    : parent_type(std::move(rvalue))
    , _storage(nullptr), _capacity(0) {
    std::swap(rvalue._capacity, _capacity);
    std::swap(rvalue._storage, _storage);
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits >
auto TBasicICStrStreamBuffer<_Char, _Traits>::operator =(TBasicICStrStreamBuffer&& rvalue) -> TBasicICStrStreamBuffer& {
    parent_type::operator =(std::move(rvalue));
    std::swap(rvalue._capacity, _capacity);
    std::swap(rvalue._storage, _storage);
    return *this;
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits >
TBasicICStrStreamBuffer<_Char, _Traits>::~TBasicICStrStreamBuffer() {}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits >
void TBasicICStrStreamBuffer<_Char, _Traits>::Reset() {
    Assert(nullptr != _storage || 0 == _capacity);
    _Char* p = (_Char*)_storage;
    parent_type::setg(p, p, p + _capacity);
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits >
void TBasicICStrStreamBuffer<_Char, _Traits>::swap(TBasicICStrStreamBuffer& other) {
    parent_type::swap(other);
    std::swap(other._storage, _storage);
    std::swap(other._capacity, _capacity);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits >
TBasicOCStrStream<_Char, _Traits>::TBasicOCStrStream(_Char* storage, std::streamsize capacity)
:   buffer_type(storage, capacity)
,   stream_type(this) {}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits >
TBasicOCStrStream<_Char, _Traits>::TBasicOCStrStream(TBasicOCStrStream&& rvalue)
:   buffer_type(std::move(rvalue))
,   parent_type(this) {
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits >
auto TBasicOCStrStream<_Char, _Traits>::operator =(TBasicOCStrStream&& rvalue) -> TBasicOCStrStream& {
    buffer_type::operator =(std::move(rvalue._buffer));
    stream_type::operator =(parent_type(&_buffer));
    return *this;
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits >
TBasicOCStrStream<_Char, _Traits>::~TBasicOCStrStream() {}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits >
const _Char *TBasicOCStrStream<_Char, _Traits>::NullTerminatedStr() {
    buffer_type::PutEOS();
    return buffer_type::storage();
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits >
TMemoryView<const _Char> TBasicOCStrStream<_Char, _Traits>::MakeView() const {
    const _Char eos = _Traits::to_char_type(0); // ignores null char IFN

    const _Char *ptr = buffer_type::storage();
    size_t length = checked_cast<size_t>(buffer_type::size());
    Assert(nullptr != ptr || 0 == length);
    if (length && ptr[length - 1] == eos)
        --length;

    return TMemoryView<const _Char>(ptr, length);
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits >
TMemoryView<const _Char> TBasicOCStrStream<_Char, _Traits>::MakeView_NullTerminated() {
    buffer_type::PutEOS();
    return MakeView();
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits >
void TBasicOCStrStream<_Char, _Traits>::swap(TBasicOCStrStream& other){
    buffer_type::swap(other);
    stream_type::swap(other);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits >
TBasicICStrStream<_Char, _Traits>::TBasicICStrStream(const _Char* storage, std::streamsize capacity)
    : buffer_type(storage, capacity)
    , stream_type(this) {}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits >
TBasicICStrStream<_Char, _Traits>::TBasicICStrStream(TBasicICStrStream&& rvalue)
    : buffer_type(std::move(rvalue))
    , parent_type(this) {
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits >
auto TBasicICStrStream<_Char, _Traits>::operator =(TBasicICStrStream&& rvalue) -> TBasicICStrStream& {
    buffer_type::operator =(std::move(rvalue._buffer));
    stream_type::operator =(parent_type(&_buffer));
    return *this;
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits >
TBasicICStrStream<_Char, _Traits>::~TBasicICStrStream() {}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits >
void TBasicICStrStream<_Char, _Traits>::swap(TBasicICStrStream& other) {
    buffer_type::swap(other);
    stream_type::swap(other);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
