#include "stdafx.h"

#include "IO/StringBuilder.h"

#include "IO/String.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <typename _Char>
struct TStealBasicString_ {
    using string_type = typename TBasicStringBuilder<_Char>::string_type;
    using stream_type = typename TBasicStringBuilder<_Char>::stream_type;
    using allocator_type = typename TBasicStringBuilder<_Char>::allocator_type;
    static stream_type StealData(allocator_type&& alloc, string_type&& str) {
        return stream_type(std::move(alloc), str.StealDataUnsafe(alloc));
    }
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char>
TBasicStringBuilder<_Char>::TBasicStringBuilder()
    : stream_type(allocator_type(static_cast<insitu_type&>(*this)))
    , TBasicTextWriter<_Char>(static_cast<IBufferedStreamWriter*>(this))
{}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicStringBuilder<_Char>::TBasicStringBuilder(size_t capacity)
    : TBasicStringBuilder() {
    reserve(capacity);
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicStringBuilder<_Char>::TBasicStringBuilder(string_type&& stolen)
    : stream_type(TStealBasicString_<_Char>::StealData(
        allocator_type(static_cast<insitu_type&>(*this)),
        std::move(stolen) ))
    , TBasicTextWriter<_Char>(static_cast<IBufferedStreamWriter*>(this))
{}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicStringBuilder<_Char>::~TBasicStringBuilder() {
    // this predicate is needed to steal from/to TBasicString<> !
#if USE_PPE_MEMORY_DEBUGGING && USE_PPE_BASICSTRING_SBO
    STATIC_ASSERT(allocator_type::Capacity == string_type::GInSituSize);
#elif !USE_PPE_MEMORY_DEBUGGING
    STATIC_ASSERT(Meta::TCheckSameSize<
        Meta::TArray<char, allocator_type::Capacity>,
        Meta::TArray<char, string_type::GInSituSize>
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
    if (str.empty() || str.back() != _Char())
        *this << Eos;

    output.assign(std::move(*this));
}
//----------------------------------------------------------------------------
EXTERN_TEMPLATE_CLASS_DEF(PPE_CORE_API) TBasicStringBuilder<char>;
EXTERN_TEMPLATE_CLASS_DEF(PPE_CORE_API) TBasicStringBuilder<wchar_t>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
