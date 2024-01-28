#pragma once

#include "Core_fwd.h"

#include "Allocator/Allocation.h"
#include "Allocator/InSituAllocator.h"
#include "IO/String.h"
#include "IO/TextWriter.h"
#include "Memory/MemoryStream.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
// Skip in-situ storage when debugging memory
#if USE_PPE_MEMORY_DEBUGGING
template <typename _Char>
using TStringBuilderAllocator = FStringAllocator;
#else
template <typename _Char> struct header_unit_workaround_insitu_ {};
template <> struct header_unit_workaround_insitu_<char> : std::integral_constant<size_t, StringInSituSize * sizeof(char)> {};
template <> struct header_unit_workaround_insitu_<wchar_t> : std::integral_constant<size_t, WStringInSituSize * sizeof(wchar_t)> {};
template <typename _Char>
using TStringBuilderAllocator = TSegregateAllocator<
    header_unit_workaround_insitu_<_Char>::value,
    TInSituAllocator<header_unit_workaround_insitu_<_Char>::value>,
    FStringAllocator >;
#endif
} //!details
//----------------------------------------------------------------------------
template <typename _Char>
class TBasicStringBuilder :
    private TMemoryStream< details::TStringBuilderAllocator<_Char> >
,   public  TBasicTextWriter<_Char> {
public:
    typedef TBasicString<_Char> string_type;
    typedef TMemoryStream< details::TStringBuilderAllocator<_Char> > stream_type;

    TBasicStringBuilder();
    virtual ~TBasicStringBuilder();

    explicit TBasicStringBuilder(size_t capacity);
    explicit TBasicStringBuilder(string_type&& stolen);

    TBasicStringBuilder(const TBasicStringBuilder&) = delete;
    TBasicStringBuilder& operator =(const TBasicStringBuilder&) = delete;

    CONSTF size_t size() const;
    CONSTF size_t capacity() const;
    CONSTF bool empty() const;

    void reserve(size_t count);

    void clear();
    void clear_ReleaseMemory();

    TBasicStringView<_Char> Written() const;

    auto begin() { return stream_type::MakeView().template Cast<_Char>().begin(); }
    auto end() { return stream_type::MakeView().template Cast<_Char>().end(); }

    auto begin() const { return Written().begin(); }
    auto end() const { return Written().end(); }

    const _Char* c_str() const NOEXCEPT { return Written().data(); }

    string_type ToString();
    void ToString(string_type& output);

    TMemoryView<_Char> AppendUninitialized(size_t n);

    NODISCARD bool AcquireDataUnsafe(FAllocatorBlock b, size_t len = 0) NOEXCEPT;
    NODISCARD FAllocatorBlock StealDataUnsafe(size_t* len = nullptr) NOEXCEPT;

    using stream_type::AcquireDataUnsafe;
    using stream_type::StealDataUnsafe;

    using TBasicTextWriter<_Char>::Write; // otherwise ambiguous with TMemoryStream<>::Write()

};
//----------------------------------------------------------------------------
template <typename _Char>
TBasicStringBuilder<_Char>::TBasicStringBuilder()
:   TBasicTextWriter<_Char>(static_cast<IBufferedStreamWriter*>(this))
{}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicStringBuilder<_Char>::TBasicStringBuilder(size_t capacity)
:   TBasicStringBuilder() {
    reserve(capacity);
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicStringBuilder<_Char>::TBasicStringBuilder(string_type&& stolen)
:   TBasicTextWriter<_Char>(static_cast<IBufferedStreamWriter*>(this)) {
    if (FAllocatorBlock b = stolen.StealDataUnsafe())
        Verify(stream_type::AcquireDataUnsafe(b)); // ignore the content, just want the allocation
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicStringBuilder<_Char>::~TBasicStringBuilder() {
    // this predicate is needed to steal from/to TBasicString<> !
    using allocator_t = typename stream_type::allocator_type;
#if USE_PPE_MEMORY_DEBUGGING && USE_PPE_BASICSTRING_SBO
    STATIC_ASSERT(allocator_t::Threshold == string_type::GInSituSize * sizeof(_Char));
#elif !USE_PPE_MEMORY_DEBUGGING
    STATIC_ASSERT(Meta::TCheckSameSize<
        Meta::TArray<u8, allocator_t::Threshold>,
        Meta::TArray<_Char, string_type::GInSituSize>
        >::value );
#endif
}
//----------------------------------------------------------------------------
template <typename _Char>
size_t TBasicStringBuilder<_Char>::size() const {
    return (stream_type::size() / sizeof(_Char));
}
//----------------------------------------------------------------------------
template <typename _Char>
size_t TBasicStringBuilder<_Char>::capacity() const {
    return (stream_type::capacity() / sizeof(_Char));
}
//----------------------------------------------------------------------------
template <typename _Char>
bool TBasicStringBuilder<_Char>::empty() const {
    return stream_type::empty();
}
//----------------------------------------------------------------------------
template <typename _Char>
void TBasicStringBuilder<_Char>::reserve(size_t count) {
    stream_type::reserve((count + 1/* null char */) * sizeof(_Char));
}
//----------------------------------------------------------------------------
template <typename _Char>
void TBasicStringBuilder<_Char>::clear() {
    stream_type::clear();
}
//----------------------------------------------------------------------------
template <typename _Char>
void TBasicStringBuilder<_Char>::clear_ReleaseMemory() {
    stream_type::clear_ReleaseMemory();
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicStringView<_Char> TBasicStringBuilder<_Char>::Written() const {
    return stream_type::MakeView().template Cast<const _Char>();
}
//----------------------------------------------------------------------------
template <typename _Char>
auto TBasicStringBuilder<_Char>::ToString() -> string_type {
    string_type s;
    ToString(s);
    return s;
}
//----------------------------------------------------------------------------
template <typename _Char>
void TBasicStringBuilder<_Char>::ToString(string_type& output) {
    const TBasicStringView<_Char> str = stream_type::MakeView().template Cast<const _Char>();
    if (not str.empty() && str.back() != _Char())
        *this << Eos; // append '\0' terminator only to non empty strings

    output.assign(std::move(*this));
}
//----------------------------------------------------------------------------
template <typename _Char>
TMemoryView<_Char> TBasicStringBuilder<_Char>::AppendUninitialized(size_t n) {
    return stream_type::Append(n * sizeof(_Char)).template Cast<_Char>();
}
//----------------------------------------------------------------------------
template <typename _Char>
bool TBasicStringBuilder<_Char>::AcquireDataUnsafe(FAllocatorBlock b, size_t len/* = 0 */) NOEXCEPT {
    return stream_type::AcquireDataUnsafe(b, len * sizeof(_Char));
}
//----------------------------------------------------------------------------
template <typename _Char>
FAllocatorBlock TBasicStringBuilder<_Char>::StealDataUnsafe(size_t* len/* = nullptr */) NOEXCEPT {
    const FAllocatorBlock b = stream_type::StealDataUnsafe(len);
    if (len) *len /= sizeof(_Char);
    return b;
}
//----------------------------------------------------------------------------
#ifndef EXPORT_PPE_RUNTIME_CORE_STRINGBUILDER
EXTERN_TEMPLATE_CLASS_DECL(PPE_CORE_API) TBasicStringBuilder<char>;
EXTERN_TEMPLATE_CLASS_DECL(PPE_CORE_API) TBasicStringBuilder<wchar_t>;
#endif
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
