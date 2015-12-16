#pragma once

#include "Core/IO/Stream.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits >
BasicOCStrStreamBuffer<_Char, _Traits>::BasicOCStrStreamBuffer(_Char* storage, std::streamsize capacity)
:   _storage(storage), _capacity(capacity) {
    Reset();
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits >
BasicOCStrStreamBuffer<_Char, _Traits>::BasicOCStrStreamBuffer(BasicOCStrStreamBuffer&& rvalue)
:   parent_type(std::move(rvalue))
,   _storage(nullptr), _capacity(0) {
    std::swap(rvalue._capacity, _capacity);
    std::swap(rvalue._storage, _storage);
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits >
auto BasicOCStrStreamBuffer<_Char, _Traits>::operator =(BasicOCStrStreamBuffer&& rvalue) -> BasicOCStrStreamBuffer& {
    parent_type::operator =(std::move(rvalue));
    std::swap(rvalue._capacity, _capacity);
    std::swap(rvalue._storage, _storage);
    return *this;
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits >
BasicOCStrStreamBuffer<_Char, _Traits>::~BasicOCStrStreamBuffer() {
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
void BasicOCStrStreamBuffer<_Char, _Traits>::ForceEOS() {
    const _Char eos = traits_type::to_char_type(0); // End Of String

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
void BasicOCStrStreamBuffer<_Char, _Traits>::PutEOS() {
    const _Char eos = traits_type::to_char_type(0); // End Of String

    if (nullptr == parent_type::pbase() ||
        (parent_type::pptr() > parent_type::pbase() && eos == parent_type::pptr()[-1]) )
        return; // skip if the string is already null terminated (or write buffer null)

    // null terminate the string
    parent_type::sputc(eos);
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits >
void BasicOCStrStreamBuffer<_Char, _Traits>::RemoveEOS() {
    const _Char eos = traits_type::to_char_type(0); // End Of String

    if (nullptr == parent_type::pbase() ||
        (parent_type::pptr() > parent_type::pbase() && eos != parent_type::pptr()[-1]) )
        return; // skip if the string is not null terminated (or write buffer null)

    // remove null terminator
    parent_type::pbump(-1);
    Assert(*parent_type::pptr() == eos);
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits >
void BasicOCStrStreamBuffer<_Char, _Traits>::Reset() {
    Assert(nullptr != _storage || 0 == _capacity);
    parent_type::setp(_storage, _storage + _capacity);
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits >
void BasicOCStrStreamBuffer<_Char, _Traits>::swap(BasicOCStrStreamBuffer& other){
    parent_type::swap(other);
    std::swap(other._storage, _storage);
    std::swap(other._capacity, _capacity);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits >
BasicOCStrStream<_Char, _Traits>::BasicOCStrStream(_Char* storage, std::streamsize capacity)
:   _buffer(storage, capacity)
,   parent_type(&_buffer) {}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits >
BasicOCStrStream<_Char, _Traits>::BasicOCStrStream(BasicOCStrStream&& rvalue)
:   _buffer(std::move(rvalue._buffer))
,   parent_type(&_buffer) {
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits >
auto BasicOCStrStream<_Char, _Traits>::operator =(BasicOCStrStream&& rvalue) -> BasicOCStrStream& {
    _buffer = std::move(rvalue._buffer);
    parent_type::operator =(parent_type(&_buffer));
    return *this;
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits >
BasicOCStrStream<_Char, _Traits>::~BasicOCStrStream() {}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits >
void BasicOCStrStream<_Char, _Traits>::ForceEOS() {
    _buffer.ForceEOS();
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits >
void BasicOCStrStream<_Char, _Traits>::PutEOS() {
    _buffer.PutEOS();
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits >
void BasicOCStrStream<_Char, _Traits>::RemoveEOS() {
    _buffer.RemoveEOS();
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits >
void BasicOCStrStream<_Char, _Traits>::Reset() {
    _buffer.Reset();
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits >
const _Char *BasicOCStrStream<_Char, _Traits>::NullTerminatedStr() {
    _buffer.PutEOS();
    return _buffer.storage();
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits >
MemoryView<const _Char> BasicOCStrStream<_Char, _Traits>::MakeView() const {
    const _Char eos = _Traits::to_char_type(0); // ignores null char IFN

    const _Char *ptr = _buffer.storage();
    size_t length = checked_cast<size_t>(_buffer.size());
    Assert(nullptr != ptr || 0 == length);
    if (length && ptr[length - 1] == eos)
        --length;

    return MemoryView<const _Char>(ptr, length);
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits >
void BasicOCStrStream<_Char, _Traits>::swap(BasicOCStrStream& other){
    parent_type::swap(other);
    std::swap(other._buffer, _buffer);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
