#pragma once

#include "Core/Core.h"

#include "Core/Allocator/Allocation.h"
#include "Core/Allocator/InSituAllocator.h"
#include "Core/IO/StreamProvider.h"
#include "Core/IO/String.h"
#include "Core/IO/TextWriter.h"
#include "Core/Memory/MemoryStream.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char>
using TStringBuilderAllocator = TInSituAllocator<
    _Char,
    TBasicString<_Char>::GInSituSize * sizeof(_Char),
    TRebindAlloc<TStringAllocator<_Char>, u8>
>;
//----------------------------------------------------------------------------
template <typename _Char>
class TBasicStringBuilder :
    private TStringBuilderAllocator<_Char>::storage_type
,   private TMemoryStream< TStringBuilderAllocator<_Char> >
,   public  TBasicTextWriter<_Char> {
public:
    typedef TBasicString<_Char> string_type;
    typedef TMemoryStream< TStringBuilderAllocator<_Char> > stream_type;

    typedef TStringBuilderAllocator<_Char> allocator_type;
    typedef typename TStringBuilderAllocator<_Char>::storage_type insitu_type;

    TBasicStringBuilder();
    virtual ~TBasicStringBuilder();

    explicit TBasicStringBuilder(string_type&& stolen);

    TBasicStringBuilder(const TBasicStringBuilder&) = delete;
    TBasicStringBuilder& operator =(const TBasicStringBuilder&) = delete;

    size_t size() const { return (stream_type::size() / sizeof(_Char)); }
    size_t capacity() const { return (stream_type::capacity() / sizeof(_Char)); }
    bool empty() const { return stream_type::empty(); }

    void reserve(size_t count);

    void clear();
    void clear_ReleaseMemory();

    TBasicStringView<_Char> Written() const {
        return stream_type::MakeView().template Cast<const _Char>();
    }

    auto begin() { return stream_type::MakeView().template Cast<_Char>().begin(); }
    auto end() { return stream_type::MakeView().template Cast<_Char>().end(); }

    auto begin() const { return Written().begin(); }
    auto end() const { return Written().end(); }

    string_type ToString();
    void ToString(string_type& output);

    template <typename _OtherAllocator>
    TMemoryView<typename _OtherAllocator::value_type> StealDataUnsafe(_OtherAllocator& alloc, size_t* plen = nullptr) {
        return stream_type::StealDataUnsafe(alloc, plen);
    }
};
//----------------------------------------------------------------------------
CORE_API extern template class TBasicStringBuilder<char>;
CORE_API extern template class TBasicStringBuilder<wchar_t>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
