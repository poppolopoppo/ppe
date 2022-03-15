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
// Skip in-situ storage when debugging memory
#if USE_PPE_MEMORY_DEBUGGING
template <typename _Char>
using TStringBuilderAllocator = FStringAllocator;
#else
template <typename _Char>
using TStringBuilderAllocator = TSegregateAllocator<
    TBasicString<_Char>::GInSituSize * sizeof(_Char),
    TInSituAllocator<TBasicString<_Char>::GInSituSize * sizeof(_Char)>,
    FStringAllocator >;
#endif
//----------------------------------------------------------------------------
template <typename _Char>
class TBasicStringBuilder :
    private TMemoryStream< TStringBuilderAllocator<_Char> >
,   public  TBasicTextWriter<_Char> {
public:
    typedef TBasicString<_Char> string_type;
    typedef TMemoryStream< TStringBuilderAllocator<_Char> > stream_type;

    TBasicStringBuilder();
    virtual ~TBasicStringBuilder();

    explicit TBasicStringBuilder(size_t capacity);
    explicit TBasicStringBuilder(string_type&& stolen);

    TBasicStringBuilder(const TBasicStringBuilder&) = delete;
    TBasicStringBuilder& operator =(const TBasicStringBuilder&) = delete;

    CONSTF size_t size() const { return (stream_type::size() / sizeof(_Char)); }
    CONSTF size_t capacity() const { return (stream_type::capacity() / sizeof(_Char)); }
    CONSTF bool empty() const { return stream_type::empty(); }

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

    const _Char* c_str() const NOEXCEPT { return Written().data(); }

    string_type ToString();
    void ToString(string_type& output);

    TMemoryView<_Char> AppendUninitialized(size_t n) {
        return stream_type::Append(n * sizeof(_Char)).template Cast<_Char>();
    }

    NODISCARD bool AcquireDataUnsafe(FAllocatorBlock b, size_t len = 0) NOEXCEPT {
        return stream_type::AcquireDataUnsafe(b, len * sizeof(_Char));
    }

    NODISCARD FAllocatorBlock StealDataUnsafe(size_t* len = nullptr) NOEXCEPT {
        const FAllocatorBlock b = stream_type::StealDataUnsafe(len);
        if (len) *len /= sizeof(_Char);
        return b;
    }

    using stream_type::AcquireDataUnsafe;
    using stream_type::StealDataUnsafe;

    using TBasicTextWriter<_Char>::Write; // otherwise ambiguous with TMemoryStream<>::Write()

};
//----------------------------------------------------------------------------
EXTERN_TEMPLATE_CLASS_DECL(PPE_CORE_API) TBasicStringBuilder<char>;
EXTERN_TEMPLATE_CLASS_DECL(PPE_CORE_API) TBasicStringBuilder<wchar_t>;
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
