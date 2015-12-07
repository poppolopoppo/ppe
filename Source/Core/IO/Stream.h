#pragma once

#include "Core/Core.h"

#include "Core/Allocator/Allocation.h"
#include "Core/Memory/UniqueView.h"

#include <iostream>
#include <sstream>
#include <streambuf>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define STACKLOCAL_BASICOCSTRSTREAM(_CHAR, _NAME, _COUNT) \
    MALLOCA(_CHAR, CONCAT(_Alloca_, _NAME), _COUNT); \
    Core::BasicOCStrStream<_CHAR, std::char_traits<_CHAR> > _NAME( CONCAT(_Alloca_, _NAME).MakeView() )
//----------------------------------------------------------------------------
#define STACKLOCAL_OCSTRSTREAM(_NAME, _COUNT) STACKLOCAL_BASICOCSTRSTREAM(char, _NAME, _COUNT)
#define STACKLOCAL_WOCSTRSTREAM(_NAME, _COUNT) STACKLOCAL_BASICOCSTRSTREAM(wchar_t, _NAME, _COUNT)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
INSTANTIATE_CLASS_TYPEDEF(IStream,   std::basic_istream<char, std::char_traits<char> >);
INSTANTIATE_CLASS_TYPEDEF(WIStream,  std::basic_istream<wchar_t, std::char_traits<wchar_t> >);
//----------------------------------------------------------------------------
INSTANTIATE_CLASS_TYPEDEF(OStream,   std::basic_ostream<char, std::char_traits<char> >);
INSTANTIATE_CLASS_TYPEDEF(WOStream,  std::basic_ostream<wchar_t, std::char_traits<wchar_t> >);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
typedef std::basic_ifstream<char, std::char_traits<char> > IFStream;
typedef std::basic_ifstream<wchar_t, std::char_traits<wchar_t> > WIFStream;
//----------------------------------------------------------------------------
typedef std::basic_ofstream<char, std::char_traits<char> > OFStream;
typedef std::basic_ofstream<wchar_t, std::char_traits<wchar_t> > WOFStream;
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

    void ForceEOS();
    void PutEOS();
    void RemoveEOS();
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
INSTANTIATE_CLASS_TYPEDEF(OCStrStreamBuffer,    BasicOCStrStreamBuffer<char, std::char_traits<char> >);
INSTANTIATE_CLASS_TYPEDEF(WOCStrStreamBuffer,   BasicOCStrStreamBuffer<wchar_t, std::char_traits<wchar_t> >);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits = std::char_traits<_Char> >
class BasicOCStrStream : public std::basic_ostream<_Char, _Traits> {
public:
    typedef std::basic_ostream<_Char, _Traits> parent_type;

    BasicOCStrStream(_Char* storage, std::streamsize capacity);
    virtual ~BasicOCStrStream();

    explicit BasicOCStrStream(const MemoryView<_Char>& view)
        : BasicOCStrStream(view.Pointer(), view.size()) {}

    template <size_t _Capacity>
    explicit BasicOCStrStream(_Char(&staticArray)[_Capacity])
        : BasicOCStrStream(staticArray, _Capacity) {}

    BasicOCStrStream(BasicOCStrStream&& rvalue);
    BasicOCStrStream& operator =(BasicOCStrStream&& rvalue);

    BasicOCStrStream(const BasicOCStrStream&) = delete;
    BasicOCStrStream& operator =(const BasicOCStrStream&) = delete;

    const _Char *storage() const { return _buffer.storage(); }
    std::streamsize capacity() const { return _buffer.capacity(); }
    std::streamsize size() const { return _buffer.size(); }

    const _Char *begin() const { return _buffer.storage(); }
    const _Char *end() const { return _buffer.storage() + _buffer.size(); }

    const _Char *Pointer() const { return _buffer.storage(); }
    size_t SizeInBytes() const { return checked_cast<size_t>(_buffer.size() * sizeof(_Char)); }

    void ForceEOS();
    void PutEOS();
    void RemoveEOS();
    void Reset();

    const _Char *NullTerminatedStr();
    MemoryView<const _Char> MakeView() const;

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
INSTANTIATE_CLASS_TYPEDEF(OCStrStream,  BasicOCStrStream<char, std::char_traits<char> >);
INSTANTIATE_CLASS_TYPEDEF(WOCStrStream, BasicOCStrStream<wchar_t, std::char_traits<wchar_t> >);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename _Allocator = ALLOCATOR(String, _Char), typename _Traits = std::char_traits<_Char> >
using BasicIStringStream = std::basic_istringstream<_Char, _Traits, _Allocator>;
template <typename _Char, typename _Allocator = ALLOCATOR(String, _Char), typename _Traits = std::char_traits<_Char> >
using BasicOStringStream = std::basic_ostringstream<_Char, _Traits, _Allocator>;
//----------------------------------------------------------------------------
INSTANTIATE_CLASS_TYPEDEF(IStringStream,    BasicIStringStream<char>);
INSTANTIATE_CLASS_TYPEDEF(WIStringStream,   BasicIStringStream<wchar_t>);
//----------------------------------------------------------------------------
INSTANTIATE_CLASS_TYPEDEF(OStringStream,    BasicOStringStream<char>);
INSTANTIATE_CLASS_TYPEDEF(WOStringStream,   BasicOStringStream<wchar_t>);
//----------------------------------------------------------------------------
INSTANTIATE_CLASS_TYPEDEF(ThreadLocalIStringStream,     BasicIStringStream<char, THREAD_LOCAL_ALLOCATOR(String, char)>);
INSTANTIATE_CLASS_TYPEDEF(ThreadLocalWIStringStream,    BasicIStringStream<wchar_t, THREAD_LOCAL_ALLOCATOR(String, wchar_t)>);
//----------------------------------------------------------------------------
INSTANTIATE_CLASS_TYPEDEF(ThreadLocalOStringStream,     BasicOStringStream<char, THREAD_LOCAL_ALLOCATOR(String, char)>);
INSTANTIATE_CLASS_TYPEDEF(ThreadLocalWOStringStream,    BasicOStringStream<wchar_t, THREAD_LOCAL_ALLOCATOR(String, wchar_t)>);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/IO/Stream-inl.h"
