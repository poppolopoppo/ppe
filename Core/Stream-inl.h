#pragma once

#include "Stream.h"
#include "CrtDebug.h" //%NOCOMMIT%

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits >
BasicOCStrStreamBuffer<_Char, _Traits>::BasicOCStrStreamBuffer(_Char* storage, std::streamsize capacity)
:   _storage(storage), _capacity(capacity) {
    Assert(storage);
    Assert(capacity);
    parent_type::setp(storage, storage + capacity - 1);
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
    // null terminate the string
    *parent_type::pptr() = traits_type::to_char_type(0);
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits >
auto BasicOCStrStreamBuffer<_Char, _Traits>::overflow(int_type ch) -> int_type {
    if (traits_type::eof() == ch)
        return traits_type::eof();

    Assert(parent_type::pptr() <= parent_type::epptr());
    *parent_type::pptr() = traits_type::to_char_type(ch);
    parent_type::pbump(1);

    return ch;
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
void BasicOCStrStream<_Char, _Traits>::swap(BasicOCStrStream& other){
    parent_type::swap(other);
    std::swap(other._buffer, _buffer);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
