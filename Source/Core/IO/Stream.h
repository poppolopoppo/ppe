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
    Core::TBasicOCStrStream<_CHAR, std::char_traits<_CHAR> > _NAME( CONCAT(_Alloca_, _NAME).MakeView() )
//----------------------------------------------------------------------------
#define STACKLOCAL_OCSTRSTREAM(_NAME, _COUNT)   STACKLOCAL_BASICOCSTRSTREAM(char, _NAME, _COUNT)
#define STACKLOCAL_WOCSTRSTREAM(_NAME, _COUNT)  STACKLOCAL_BASICOCSTRSTREAM(wchar_t, _NAME, _COUNT)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
typedef std::basic_istream<char, std::char_traits<char> >           FIStream;
typedef std::basic_istream<wchar_t, std::char_traits<wchar_t> >     FWIStream;
//----------------------------------------------------------------------------
typedef std::basic_ostream<char, std::char_traits<char> >           FOStream;
typedef std::basic_ostream<wchar_t, std::char_traits<wchar_t> >     FWOStream;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
typedef std::basic_ifstream<char, std::char_traits<char> >          FIFStream;
typedef std::basic_ifstream<wchar_t, std::char_traits<wchar_t> >    FWIFStream;
//----------------------------------------------------------------------------
typedef std::basic_ofstream<char, std::char_traits<char> >          FOFStream;
typedef std::basic_ofstream<wchar_t, std::char_traits<wchar_t> >    FWOFStream;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Allow to create a std::basic_ostream<> from a non-growable (char|wchar_t) buffer
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits = std::char_traits<_Char> >
class TBasicOCStrStreamBuffer : public std::basic_streambuf<_Char, _Traits> {
public:
    typedef std::basic_streambuf<_Char, _Traits> parent_type;

    using typename parent_type::int_type;
    using typename parent_type::traits_type;

    TBasicOCStrStreamBuffer(_Char* storage, std::streamsize capacity);
    virtual ~TBasicOCStrStreamBuffer(); // _storage is only null-terminated here

    TBasicOCStrStreamBuffer(TBasicOCStrStreamBuffer&& rvalue);
    TBasicOCStrStreamBuffer& operator =(TBasicOCStrStreamBuffer&& rvalue);

    TBasicOCStrStreamBuffer(const TBasicOCStrStreamBuffer&) = delete;
    TBasicOCStrStreamBuffer& operator =(const TBasicOCStrStreamBuffer&) = delete;

    const _Char* storage() const { return _storage; }
    std::streamsize capacity() const { return _capacity; }
    std::streamsize size() const { return parent_type::pptr() - _storage; }

    void ForceEOS();
    void PutEOS();
    void RemoveEOS();
    void Reset();

    void swap(TBasicOCStrStreamBuffer& other);

private:
    _Char* _storage;
    std::streamsize _capacity;
};
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits = std::char_traits<_Char> >
void swap(TBasicOCStrStreamBuffer<_Char, _Traits>& lhs, TBasicOCStrStreamBuffer<_Char, _Traits>& rhs) {
    lhs.swap(rhs);
}
//----------------------------------------------------------------------------
typedef TBasicOCStrStreamBuffer<char, std::char_traits<char> >          FOCStrStreamBuffer;
typedef TBasicOCStrStreamBuffer<wchar_t, std::char_traits<wchar_t> >    FWOCStrStreamBuffer;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits = std::char_traits<_Char> >
class TBasicOCStrStream :
    private TBasicOCStrStreamBuffer<_Char, _Traits>
,   public std::basic_ostream<_Char, _Traits> {
public:
    typedef TBasicOCStrStreamBuffer<_Char, _Traits> buffer_type;
    typedef std::basic_ostream<_Char, _Traits> stream_type;

    TBasicOCStrStream(_Char* storage, std::streamsize capacity);
    virtual ~TBasicOCStrStream();

    explicit TBasicOCStrStream(const TMemoryView<_Char>& view)
        : TBasicOCStrStream(view.Pointer(), view.size()) {}

    template <size_t _Capacity>
    explicit TBasicOCStrStream(_Char(&staticArray)[_Capacity])
        : TBasicOCStrStream(staticArray, _Capacity) {}

    TBasicOCStrStream(TBasicOCStrStream&& rvalue);
    TBasicOCStrStream& operator =(TBasicOCStrStream&& rvalue);

    TBasicOCStrStream(const TBasicOCStrStream&) = delete;
    TBasicOCStrStream& operator =(const TBasicOCStrStream&) = delete;

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

    TMemoryView<const _Char> MakeView() const;
    TMemoryView<const _Char> MakeView_NullTerminated();

    void swap(TBasicOCStrStream& other);
};
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits = std::char_traits<_Char> >
void swap(TBasicOCStrStream<_Char, _Traits>& lhs, TBasicOCStrStream<_Char, _Traits>& rhs) {
    lhs.swap(rhs);
}
//----------------------------------------------------------------------------
typedef TBasicOCStrStream<char, std::char_traits<char> >        FOCStrStream;
typedef TBasicOCStrStream<wchar_t, std::char_traits<wchar_t> >  FWOCStrStream;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename _Allocator = ALLOCATOR(String, _Char), typename _Traits = std::char_traits<_Char> >
using TBasicIStringStream = std::basic_istringstream<_Char, _Traits, _Allocator>;
template <typename _Char, typename _Allocator = ALLOCATOR(String, _Char), typename _Traits = std::char_traits<_Char> >
using TBasicOStringStream = std::basic_ostringstream<_Char, _Traits, _Allocator>;
//----------------------------------------------------------------------------
typedef TBasicIStringStream<char>       FIStringStream;
typedef TBasicIStringStream<wchar_t>    FWIStringStream;
//----------------------------------------------------------------------------
typedef TBasicOStringStream<char>       FOStringStream;
typedef TBasicOStringStream<wchar_t>    FWOStringStream;
//----------------------------------------------------------------------------
typedef TBasicIStringStream<char, THREAD_LOCAL_ALLOCATOR(String, char)>         FThreadLocalIStringStream;
typedef TBasicIStringStream<wchar_t, THREAD_LOCAL_ALLOCATOR(String, wchar_t)>   FThreadLocalWIStringStream;
//----------------------------------------------------------------------------
typedef TBasicOStringStream<char, THREAD_LOCAL_ALLOCATOR(String, char)>         FThreadLocalOStringStream;
typedef TBasicOStringStream<wchar_t, THREAD_LOCAL_ALLOCATOR(String, wchar_t)>   FThreadLocalWOStringStream;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/IO/Stream-inl.h"
