#include "stdafx.h"

#include "StringBuilder.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char>
TBasicStringBuilder<_Char>::TBasicStringBuilder()
    : TBasicTextWriter<_Char>(static_cast<IBufferedStreamWriter*>(this))
{}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicStringBuilder<_Char>::TBasicStringBuilder(string_type&& stolen)
    : stringstream_type(Meta::FForceInit{}, stolen.clear_StealMemoryUnsafe().template Cast<u8>())
    , TBasicTextWriter<_Char>(static_cast<IBufferedStreamWriter*>(this))
{}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicStringBuilder<_Char>::TBasicStringBuilder(Meta::FForceInit, const TMemoryView<_Char>& stolen)
    : stringstream_type(Meta::FForceInit{}, stolen.template Cast<u8>())
    , TBasicTextWriter<_Char>(static_cast<IBufferedStreamWriter*>(this))
{}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicStringBuilder<_Char>::~TBasicStringBuilder()
{}
//----------------------------------------------------------------------------
template <typename _Char>
void TBasicStringBuilder<_Char>::reserve(size_t count) {
    stringstream_type::reserve(count * sizeof(_Char) + 1/* null char */);
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
    if (stringstream_type::MakeView().template Cast<const _Char>().back() != _Char())
        *this << Eos;

    size_t len = 0;
    TMemoryView<_Char> block = stringstream_type::template clear_StealDataUnsafe<_Char>(&len);
    Assert(len/* null char */);
    Assert(block[len] == _Char());

    output.assign(Meta::FForceInit{}, block, len);
}
//----------------------------------------------------------------------------
/*CORE_API extern*/ template class TBasicStringBuilder<char>;
/*CORE_API extern*/ template class TBasicStringBuilder<wchar_t>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
