#pragma once

#include "Core.h"

#include "Allocator/Allocation.h"
#include "Allocator/InSituAllocator.h"
#include "IO/StreamProvider.h"
#include "IO/String.h"
#include "IO/TextWriter.h"
#include "Memory/MemoryStream.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char>
using TStringBuilderAllocator = TInSituAllocator<
    TInSituStorage<_Char, TBasicString<_Char>::GInSituSize>,
    TRebindAlloc< TStringAllocator<_Char>, u8 >
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

    explicit TBasicStringBuilder(size_t capacity);
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

    TMemoryView<_Char> AppendUninitialized(size_t n) {
        return stream_type::Append(n * sizeof(_Char)).Cast<_Char>();
    }

    template <typename _OtherAllocator>
    TMemoryView<typename _OtherAllocator::value_type> StealDataUnsafe(_OtherAllocator& alloc, size_t* plen = nullptr) {
        return stream_type::StealDataUnsafe(alloc, plen);
    }

    using TBasicTextWriter<_Char>::Write; // otherwise ambiguous with TMemoryStream<>::Write()

    bool UseInSitu() const {
        return (stream_type::data() == insitu_type::data());
    }
};
//----------------------------------------------------------------------------
PPE_CORE_API extern template class TBasicStringBuilder<char>;
PPE_CORE_API extern template class TBasicStringBuilder<wchar_t>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Arg>
FString ToString(_Arg&& arg) {
    FStringBuilder oss;
    oss << arg;
    return oss.ToString();
}
//----------------------------------------------------------------------------
template <typename _Arg>
FWString ToWString(_Arg&& arg) {
    FWStringBuilder oss;
    oss << arg;
    return oss.ToString();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
