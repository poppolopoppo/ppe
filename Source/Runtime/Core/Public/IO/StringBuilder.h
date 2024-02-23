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
template <typename _Char, typename _Allocator>
class TGenericStringBuilder :
    private TMemoryStream<_Allocator>
,   public  TBasicTextWriter<_Char> {
public:
    typedef _Allocator allocator_type;
    typedef TMemoryStream<allocator_type> stream_type;

    TGenericStringBuilder();
    virtual ~TGenericStringBuilder() = default;

    explicit TGenericStringBuilder(allocator_type&& rallocator) NOEXCEPT;
    explicit TGenericStringBuilder(const allocator_type& allocator);

    explicit TGenericStringBuilder(size_t capacity);

    TGenericStringBuilder(const TGenericStringBuilder&) = delete;
    TGenericStringBuilder& operator =(const TGenericStringBuilder&) = delete;

    CONSTF size_t size() const;
    CONSTF size_t capacity() const;

    void reserve(size_t count);

    using stream_type::empty;
    using stream_type::clear;
    using stream_type::clear_ReleaseMemory;

    TBasicStringView<_Char> Written() const;
    TBasicConstChar<_Char> NullTerminated();

    auto begin() { return stream_type::MakeView().template Cast<_Char>().begin(); }
    auto end() { return stream_type::MakeView().template Cast<_Char>().end(); }

    auto begin() const { return Written().begin(); }
    auto end() const { return Written().end(); }

    const _Char* c_str() { return NullTerminated(); }

    TMemoryView<_Char> AppendUninitialized(size_t n);

    NODISCARD bool AcquireDataUnsafe(FAllocatorBlock b, size_t len = 0) NOEXCEPT;
    NODISCARD FAllocatorBlock StealDataUnsafe(size_t* len = nullptr) NOEXCEPT;

    using stream_type::AcquireDataUnsafe;
    using stream_type::StealDataUnsafe;

    using TBasicTextWriter<_Char>::Write; // otherwise ambiguous with TMemoryStream<>::Write()

};
//----------------------------------------------------------------------------
template <typename _Char, typename _Allocator>
TGenericStringBuilder<_Char, _Allocator>::TGenericStringBuilder()
:   TBasicTextWriter<_Char>(static_cast<IBufferedStreamWriter*>(this))
{}
//----------------------------------------------------------------------------
template <typename _Char, typename _Allocator>
TGenericStringBuilder<_Char, _Allocator>::TGenericStringBuilder(allocator_type&& rallocator) NOEXCEPT
:   TMemoryStream<_Allocator>(std::move(rallocator))
,   TBasicTextWriter<_Char>(static_cast<IBufferedStreamWriter*>(this))
{}
//----------------------------------------------------------------------------
template <typename _Char, typename _Allocator>
TGenericStringBuilder<_Char, _Allocator>::TGenericStringBuilder(const allocator_type& allocator)
:   TMemoryStream<_Allocator>(allocator)
,   TBasicTextWriter<_Char>(static_cast<IBufferedStreamWriter*>(this))
{}
//----------------------------------------------------------------------------
template <typename _Char, typename _Allocator>
TGenericStringBuilder<_Char, _Allocator>::TGenericStringBuilder(size_t capacity)
:   TGenericStringBuilder() {
    reserve(capacity);
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Allocator>
size_t TGenericStringBuilder<_Char, _Allocator>::size() const {
    return (stream_type::size() / sizeof(_Char));
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Allocator>
size_t TGenericStringBuilder<_Char, _Allocator>::capacity() const {
    return (stream_type::capacity() / sizeof(_Char));
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Allocator>
void TGenericStringBuilder<_Char, _Allocator>::reserve(size_t count) {
    stream_type::reserve((count + 1/* null char */) * sizeof(_Char));
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Allocator>
TBasicStringView<_Char> TGenericStringBuilder<_Char, _Allocator>::Written() const {
    return stream_type::MakeView().template Cast<const _Char>();
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Allocator>
TBasicConstChar<_Char> TGenericStringBuilder<_Char, _Allocator>::NullTerminated() {
    const TBasicStringView<_Char> str = stream_type::MakeView().template Cast<const _Char>();
    if (not str.empty() && str.back() != _Char())
        *this << Eos; // append '\0' terminator only to non empty strings
    return str.data();
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Allocator>
TMemoryView<_Char> TGenericStringBuilder<_Char, _Allocator>::AppendUninitialized(size_t n) {
    return stream_type::Append(n * sizeof(_Char)).template Cast<_Char>();
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Allocator>
bool TGenericStringBuilder<_Char, _Allocator>::AcquireDataUnsafe(FAllocatorBlock b, size_t len/* = 0 */) NOEXCEPT {
    return stream_type::AcquireDataUnsafe(b, len * sizeof(_Char));
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Allocator>
FAllocatorBlock TGenericStringBuilder<_Char, _Allocator>::StealDataUnsafe(size_t* len/* = nullptr */) NOEXCEPT {
    const FAllocatorBlock b = stream_type::StealDataUnsafe(len);
    if (len) *len /= sizeof(_Char);
    return b;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
// Skip in-situ storage when debugging memory
#if USE_PPE_MEMORY_DEBUGGING
template <typename _Char, typename _Allocator>
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
class TBasicStringBuilder : public TGenericStringBuilder<_Char, details::TStringBuilderAllocator<_Char>> {
public:
    using generic_type = TGenericStringBuilder<_Char, details::TStringBuilderAllocator<_Char>>;
    using stream_type = typename generic_type::stream_type;

    typedef TBasicString<_Char> string_type;

    TBasicStringBuilder() = default;
    virtual ~TBasicStringBuilder() override {
        // this predicate is needed to steal from/to TBasicString<> !
        using allocator_t = typename stream_type::allocator_type;
#if USE_PPE_MEMORY_DEBUGGING && USE_PPE_BASICSTRING_SBO
        STATIC_ASSERT(allocator_t::Threshold == string_type::SmallCapacity * sizeof(_Char));
#elif !USE_PPE_MEMORY_DEBUGGING
        STATIC_ASSERT(Meta::TCheckSameSize<
            Meta::TArray<u8, allocator_t::Threshold>,
            Meta::TArray<_Char, string_type::SmallCapacity>
            >::value );
#endif
    }

    explicit TBasicStringBuilder(size_t capacity) : generic_type(capacity) {}
    explicit TBasicStringBuilder(string_type&& stolen) : generic_type() {
        if (FAllocatorBlock b = stolen.StealDataUnsafe())
            Verify(generic_type::AcquireDataUnsafe(b)); // ignore the content, just want the allocation
    }

    TBasicStringBuilder(const TBasicStringBuilder&) = delete;
    TBasicStringBuilder& operator =(const TBasicStringBuilder&) = delete;

    string_type ToString();
    void ToString(string_type& output);

};
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
    generic_type::NullTerminated(); // append '\0' ifn
    output.assign(std::move(*this));
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
