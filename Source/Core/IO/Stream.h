#pragma once

#include "Core/Core.h"

#include "Core/Allocator/Allocation.h"

#include <iosfwd>
#include <streambuf>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
using BasicIStream = std::basic_istream<char, std::char_traits<char> >;
using BasicWIStream = std::basic_istream<wchar_t, std::char_traits<wchar_t> >;
//----------------------------------------------------------------------------
using BasicOStream = std::basic_ostream<char, std::char_traits<char> >;
using BasicWOStream = std::basic_ostream<wchar_t, std::char_traits<wchar_t> >;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
using IFStream = std::basic_ifstream<char, std::char_traits<char> >;
using WIFStream = std::basic_ifstream<wchar_t, std::char_traits<wchar_t> >;
//----------------------------------------------------------------------------
using OFStream = std::basic_ofstream<char, std::char_traits<char> >;
using WOFStream = std::basic_ofstream<wchar_t, std::char_traits<wchar_t> >;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Allow to create a std::basic_ostream<> from a non-growable (char|wchar_t) buffer
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits = std::char_traits<_Char> >
class BasicOCStrStreamBuffer : public std::basic_streambuf<_Char, _Traits> {
public:
    typedef std::basic_streambuf<_Char, _Traits> parent_type;

    using typename parent_type::int_type;
    using typename parent_type::traits_type;

    BasicOCStrStreamBuffer(_Char* storage, std::streamsize capacity);
    virtual ~BasicOCStrStreamBuffer(); // _storage is only null-terminated here

    BasicOCStrStreamBuffer(BasicOCStrStreamBuffer&& rvalue);
    BasicOCStrStreamBuffer& operator =(BasicOCStrStreamBuffer&& rvalue);

    BasicOCStrStreamBuffer(const BasicOCStrStreamBuffer&) = delete;
    BasicOCStrStreamBuffer& operator =(const BasicOCStrStreamBuffer&) = delete;

    const _Char* storage() const { return _storage; }
    std::streamsize capacity() const { return _capacity; }
    std::streamsize size() const { return parent_type::pptr() - _storage; }

    void PutEOS();
    void Reset();

    void swap(BasicOCStrStreamBuffer& other);

private:
    _Char* _storage;
    std::streamsize _capacity;
};
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits = std::char_traits<_Char> >
void swap(BasicOCStrStreamBuffer<_Char, _Traits>& lhs, BasicOCStrStreamBuffer<_Char, _Traits>& rhs) {
    lhs.swap(rhs);
}
//----------------------------------------------------------------------------
using OCStrStreamBuffer = BasicOCStrStreamBuffer<char, std::char_traits<char> >;
using WOCStrStreamBuffer = BasicOCStrStreamBuffer<wchar_t, std::char_traits<wchar_t> >;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits = std::char_traits<_Char> >
class BasicOCStrStream : public std::basic_ostream<_Char, _Traits> {
public:
    typedef std::basic_ostream<_Char, _Traits> parent_type;

    BasicOCStrStream(_Char* storage, std::streamsize capacity);
    virtual ~BasicOCStrStream();

    template <size_t _Capacity>
    explicit BasicOCStrStream(_Char(&staticArray)[_Capacity])
        : BasicOCStrStream(staticArray, _Capacity) {}

    BasicOCStrStream(BasicOCStrStream&& rvalue);
    BasicOCStrStream& operator =(BasicOCStrStream&& rvalue);

    BasicOCStrStream(const BasicOCStrStream&) = delete;
    BasicOCStrStream& operator =(const BasicOCStrStream&) = delete;

    const _Char* storage() const { return _buffer.storage(); }
    std::streamsize capacity() const { return _buffer.capacity(); }
    std::streamsize size() const { return _buffer.size(); }

    void PutEOS();
    void Reset();

    void swap(BasicOCStrStream& other);

private:
    BasicOCStrStreamBuffer<_Char, _Traits> _buffer;
};
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits = std::char_traits<_Char> >
void swap(BasicOCStrStream<_Char, _Traits>& lhs, BasicOCStrStream<_Char, _Traits>& rhs) {
    lhs.swap(rhs);
}
//----------------------------------------------------------------------------
using OCStrStream = BasicOCStrStream<char, std::char_traits<char> >;
using WOCStrStream = BasicOCStrStream<wchar_t, std::char_traits<wchar_t> >;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename _Allocator = ALLOCATOR(String, _Char), typename _Traits = std::char_traits<_Char> >
using BasicIStringStream = std::basic_istringstream<_Char, _Traits, _Allocator>;
template <typename _Char, typename _Allocator = ALLOCATOR(String, _Char), typename _Traits = std::char_traits<_Char> >
using BasicOStringStream = std::basic_ostringstream<_Char, _Traits, _Allocator>;
//----------------------------------------------------------------------------
using IStringStream = BasicIStringStream<char>;
using WIStringStream = BasicIStringStream<wchar_t>;
//----------------------------------------------------------------------------
using OStringStream = BasicOStringStream<char>;
using WOStringStream = BasicOStringStream<wchar_t>;
//----------------------------------------------------------------------------
using ThreadLocalIStringStream = BasicIStringStream<char, THREAD_LOCAL_ALLOCATOR(String, char)>;
using ThreadLocalWIStringStream = BasicIStringStream<wchar_t, THREAD_LOCAL_ALLOCATOR(String, wchar_t)>;
//----------------------------------------------------------------------------
using ThreadLocalOStringStream = BasicOStringStream<char, THREAD_LOCAL_ALLOCATOR(String, char)>;
using ThreadLocalWOStringStream = BasicOStringStream<wchar_t, THREAD_LOCAL_ALLOCATOR(String, wchar_t)>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/IO/Stream-inl.h"
