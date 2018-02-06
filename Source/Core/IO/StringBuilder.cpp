#include "stdafx.h"

#include "StringBuilder.h"

#include "IO/String.h"

namespace Core {
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
TBasicStringBuilder<_Char>::TBasicStringBuilder(string_type&& stolen)
    : stream_type(TStealBasicString_<_Char>::StealData(
        allocator_type(static_cast<insitu_type&>(*this)),
        std::move(stolen) ))
    , TBasicTextWriter<_Char>(static_cast<IBufferedStreamWriter*>(this))
{}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicStringBuilder<_Char>::~TBasicStringBuilder()
{}
//----------------------------------------------------------------------------
template <typename _Char>
void TBasicStringBuilder<_Char>::reserve(size_t count) {
    stream_type::reserve(count * sizeof(_Char) + 1/* null char */);
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
/*CORE_API extern*/ template class TBasicStringBuilder<char>;
/*CORE_API extern*/ template class TBasicStringBuilder<wchar_t>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
