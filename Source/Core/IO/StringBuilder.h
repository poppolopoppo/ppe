#pragma once

#include "Core/Core.h"

#include "Core/IO/StreamProvider.h"
#include "Core/IO/String.h"
#include "Core/IO/TextWriter.h"
#include "Core/Memory/MemoryStream.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char>
class TBasicStringBuilder :
    private TMemoryStream< TRebindAlloc<TStringAllocator<_Char>, u8> >,
    public TBasicTextWriter<_Char> {
public:
    typedef TBasicString<_Char> string_type;
    typedef TMemoryStream< TRebindAlloc<TStringAllocator<_Char>, u8> > stringstream_type;

    TBasicStringBuilder();
    explicit TBasicStringBuilder(string_type&& stolen);
    TBasicStringBuilder(Meta::FForceInit, const TMemoryView<_Char>& stolen);

    TBasicStringBuilder(const TBasicStringBuilder&) = delete;
    TBasicStringBuilder& operator =(const TBasicStringBuilder&) = delete;

    virtual ~TBasicStringBuilder();

    size_t size() const { return (stringstream_type::size() / sizeof(_Char)); }
    size_t capacity() const { return (stringstream_type::capacity() / sizeof(_Char)); }
    bool empty() const { return stringstream_type::empty(); }

    void reserve(size_t count);

    TBasicStringView<_Char> Written() const {
        return stringstream_type::MakeView().template Cast<const _Char>();
    }

    string_type ToString();
    void ToString(string_type& output);
};
//----------------------------------------------------------------------------
CORE_API extern template class TBasicStringBuilder<char>;
CORE_API extern template class TBasicStringBuilder<wchar_t>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
