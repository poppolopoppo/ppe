#include "stdafx.h"

#include "IO/StringBuilder.h"

#include "IO/String.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
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
EXTERN_TEMPLATE_CLASS_DEF(PPE_CORE_API) TBasicStringBuilder<char>;
EXTERN_TEMPLATE_CLASS_DEF(PPE_CORE_API) TBasicStringBuilder<wchar_t>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
