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
typedef std::basic_istream<char, std::char_traits<char> > IStream;
typedef std::basic_istream<wchar_t, std::char_traits<wchar_t> > WIStream;
//----------------------------------------------------------------------------
typedef std::basic_ostream<char, std::char_traits<char> > OStream;
typedef std::basic_ostream<wchar_t, std::char_traits<wchar_t> > WOStream;
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
typedef BasicOCStrStreamBuffer<char, std::char_traits<char> > OCStrStreamBuffer;
typedef BasicOCStrStreamBuffer<wchar_t, std::char_traits<wchar_t> > WOCStrStreamBuffer;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits = std::char_traits<_Char> >
class BasicOCStrStream :
    private BasicOCStrStreamBuffer<_Char, _Traits>
,   public std::basic_ostream<_Char, _Traits> {
public:
    typedef BasicOCStrStreamBuffer<_Char, _Traits> buffer_type;
    typedef std::basic_ostream<_Char, _Traits> stream_type;

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

    const _Char *storage() const { return buffer_type::storage(); }
    std::streamsize capacity() const { return buffer_type::capacity(); }
    std::streamsize size() const { return buffer_type::size(); }

    const _Char *begin() const { return buffer_type::storage(); }
    const _Char *end() const { return buffer_type::storage() + buffer_type::size(); }

    const _Char *Pointer() const { return buffer_type::storage(); }
    size_t SizeInBytes() const { return checked_cast<size_t>(buffer_type::size() * sizeof(_Char)); }

    using buffer_type::ForceEOS;
    using buffer_type::PutEOS;
    using buffer_type::RemoveEOS;
    using buffer_type::Reset;

    const _Char* NullTerminatedStr();
    const _Char* c_str() { return NullTerminatedStr(); }

    MemoryView<const _Char> MakeView() const;
    MemoryView<const _Char> MakeView_NullTerminated();

    void swap(BasicOCStrStream& other);
};
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits = std::char_traits<_Char> >
void swap(BasicOCStrStream<_Char, _Traits>& lhs, BasicOCStrStream<_Char, _Traits>& rhs) {
    lhs.swap(rhs);
}
//----------------------------------------------------------------------------
typedef BasicOCStrStream<char, std::char_traits<char> > OCStrStream;
typedef BasicOCStrStream<wchar_t, std::char_traits<wchar_t> > WOCStrStream;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename _Allocator = ALLOCATOR(String, _Char), typename _Traits = std::char_traits<_Char> >
using BasicIStringStream = std::basic_istringstream<_Char, _Traits, _Allocator>;
template <typename _Char, typename _Allocator = ALLOCATOR(String, _Char), typename _Traits = std::char_traits<_Char> >
using BasicOStringStream = std::basic_ostringstream<_Char, _Traits, _Allocator>;
//----------------------------------------------------------------------------
typedef BasicIStringStream<char> IStringStream;
typedef BasicIStringStream<wchar_t> WIStringStream;
//----------------------------------------------------------------------------
typedef BasicOStringStream<char> OStringStream;
typedef BasicOStringStream<wchar_t> WOStringStream;
//----------------------------------------------------------------------------
typedef BasicIStringStream<char, THREAD_LOCAL_ALLOCATOR(String, char)> ThreadLocalIStringStream;
typedef BasicIStringStream<wchar_t, THREAD_LOCAL_ALLOCATOR(String, wchar_t)> ThreadLocalWIStringStream;
//----------------------------------------------------------------------------
typedef BasicOStringStream<char, THREAD_LOCAL_ALLOCATOR(String, char)> ThreadLocalOStringStream;
typedef BasicOStringStream<wchar_t, THREAD_LOCAL_ALLOCATOR(String, wchar_t)> ThreadLocalWOStringStream;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/IO/Stream-inl.h"
