#pragma once

#include "Core/IO/String.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char>
template <typename _It>
void TBasicString<_Char>::assign(_It first, _It last) {
    clear();

    const size_t newSize = std::distance(first, last);
    auto dst = resizeWNullChar_(newSize);

    std::copy(first, last, dst);
}
//----------------------------------------------------------------------------
template <typename _Char>
template <typename _It>
void TBasicString<_Char>::append(_It first, _It last) {
    const size_t oldSize = size();
    const size_t newSize = (oldSize + std::distance(first, last));

    auto dst = resizeWNullChar_(newSize);
    std::copy(first, last, dst + oldSize);
}
//----------------------------------------------------------------------------
template <typename _Char>
template <typename _It>
void TBasicString<_Char>::insert(size_t pos, _It first, _It last) {
    const size_t oldSize = size();
    if (pos == npos) pos = oldSize;

    const size_t newSize = (oldSize + std::distance(first, last));
    auto dst = resizeWNullChar_(newSize);

    std::copy(first, last, dst + oldSize);
    std::rotate(dst + pos, dst + oldSize, dst + newSize);
}
//----------------------------------------------------------------------------
template <typename _Char>
template <typename _It>
void TBasicString<_Char>::replace(size_t pos, size_t len, _It first, _It last) {
    const size_t n = std::distance(first, last);
    const size_t oldSize = size();
    if (len == npos) len = (oldSize - pos);
    Assert(pos + len <= oldSize);

    const size_t newSize = (oldSize - len + n);
    auto dst = resizeNoNullChar_(newSize);

    if (newSize < oldSize) {
        Assert(pos + n < len);
        std::rotate(dst + (pos + n), dst + len, dst + oldSize);
    }
    else {
        Assert(pos + len <= n);
        std::rotate(dst + (pos + len), dst + n, dst + oldSize);
        std::rotate(dst + (pos + n), dst + oldSize, dst + newSize);
    }

    std::copy(first, last, dst + pos);
    *(dst + newSize) = _Char();
}
//----------------------------------------------------------------------------
template <typename _Char>
template <typename _Pred>
size_t TBasicString<_Char>::find_if(_Pred pred, size_t pos/* = 0 */) const noexcept {
    auto it = std::find_if(begin() + pos, end(), pred);
    return (it == end() ? npos : it - begin());
}
//----------------------------------------------------------------------------
template <typename _Char>
template <typename _Pred>
size_t TBasicString<_Char>::rfind_if(_Pred pred, size_t pos/* = npos */) const noexcept {
    auto it = std::find_if(pos == npos ? rbegin() : const_reverse_iterator(begin() + pos), rend(), pred);
    return (it == rend() ? npos : rend() - it);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
