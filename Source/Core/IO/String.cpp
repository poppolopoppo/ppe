#include "stdafx.h"

#include "String.h"

#include "StringView.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char>
TBasicString<_Char>::TBasicString(Meta::FForceInit, const TMemoryView<_Char>& stolen, size_t len/* = 0 */) noexcept
    : _large{ stolen.data() != nullptr, len, stolen.size(), stolen.data() } {
    Assert(CheckInvariants());
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicString<_Char>::~TBasicString() {
    if (is_large_()) {
        Assert(_large.Storage);
        allocator_traits_::deallocate(get_allocator_(), _large.Storage, _large.Capacity);
    }
}
//----------------------------------------------------------------------------
template <typename _Char>
void TBasicString<_Char>::assign(TBasicString&& rvalue) {
    clear_ReleaseMemory();

    if (rvalue.is_large_()) {
        size_t len;
        auto stolen = rvalue.clear_StealMemoryUnsafe(&len);
        assign(Meta::FForceInit{}, stolen, len);
    }
    else {
        assign(rvalue.MakeView());
        rvalue._large = { 0, 0, 0, nullptr };
    }
}
//----------------------------------------------------------------------------
template <typename _Char>
void TBasicString<_Char>::assign(const stringview_type& str) {
    clear();

    if (not str.empty()) {
        auto dst = resizeWNullChar_(str.size());
        std::copy(str.begin(), str.end(), dst);
    }
}
//----------------------------------------------------------------------------
template <typename _Char>
void TBasicString<_Char>::assign(size_t n, _Char fill) {
    Assert(n);

    clear();

    auto dst = resizeWNullChar_(n);
    std::fill_n(dst, n, fill);
}
//----------------------------------------------------------------------------
template <typename _Char>
void TBasicString<_Char>::append(size_t n, _Char fill) {
    Assert(n);

    const size_t oldSize = size();
    const size_t newSize = (oldSize + n);

    auto dst = resizeWNullChar_(newSize);
    std::fill_n(dst + oldSize, n, fill);
}
//----------------------------------------------------------------------------
template <typename _Char>
void TBasicString<_Char>::append(const stringview_type& str) {
    Assert(not str.empty());

    const size_t oldSize = size();
    const size_t newSize = (oldSize + str.size());

    auto dst = resizeWNullChar_(newSize);
    std::copy(str.begin(), str.end(), dst + oldSize);
}
//----------------------------------------------------------------------------
template <typename _Char>
void TBasicString<_Char>::insert(size_t pos, size_t n, _Char fill) {
    Assert(n);

    const size_t oldSize = size();
    if (pos == npos) pos = oldSize;
    const size_t newSize = (oldSize + n);

    auto dst = resizeWNullChar_(newSize);
    std::fill_n(dst + oldSize, n, fill);
    std::rotate(dst + pos, dst + oldSize, dst + newSize);
}
//----------------------------------------------------------------------------
template <typename _Char>
void TBasicString<_Char>::insert(size_t pos, const stringview_type& str) {
    Assert(not str.empty());

    const size_t oldSize = size();
    const size_t newSize = (oldSize + str.size());
    if (pos == npos) pos = oldSize;

    auto dst = resizeWNullChar_(newSize);
    std::copy(str.begin(), str.end(), dst + oldSize);
    std::rotate(dst + pos, dst + oldSize, dst + newSize);
}
//----------------------------------------------------------------------------
template <typename _Char>
void TBasicString<_Char>::erase(size_t pos/* = 0 */, size_t len/* = npos */) {
    const size_t oldSize = size();
    if (len == npos) len = (oldSize - pos);
    Assert(pos + len <= oldSize);

    const size_t newSize = (oldSize - len);
    auto dst = MakeCheckedIterator(data_(), oldSize, 0);
    std::rotate(dst + pos, dst + (pos + len), dst + oldSize);
    dst[newSize] = _Char();
}
//----------------------------------------------------------------------------
template <typename _Char>
void TBasicString<_Char>::replace(size_t pos, size_t len, const stringview_type& str) {
    const size_t oldSize = size();
    if (len == npos) len = (oldSize - pos);
    Assert(pos + len <= oldSize);

    const size_t newSize = (oldSize - len + str.size());
    auto dst = resizeNoNullChar_(newSize);

    if (newSize < oldSize) {
        Assert(pos + str.size() < len);
        std::rotate(dst + (pos + str.size()), dst + len, dst + oldSize);
    }
    else {
        Assert(pos + len <= str.size());
        std::rotate(dst + (pos + len), dst + str.size(), dst + oldSize);
        std::rotate(dst + (pos + str.size()), dst + oldSize, dst + newSize);
    }

    std::copy(str.begin(), str.end(), dst + pos);
    dst[newSize] = _Char();
}
//----------------------------------------------------------------------------
template <typename _Char>
void TBasicString<_Char>::replace(size_t pos, size_t len, size_t n, _Char fill) {
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

    std::fill_n(dst + pos, n, fill);
    dst[newSize] = _Char();
}
//----------------------------------------------------------------------------
template <typename _Char>
size_t TBasicString<_Char>::find(const stringview_type& str, size_t pos/* = 0 */) const noexcept {
    Assert(not str.empty());

    auto it = std::search(begin() + pos, end(), str.begin(), str.end());
    return (it == end() ? npos : it - begin());
}
//----------------------------------------------------------------------------
template <typename _Char>
size_t TBasicString<_Char>::rfind(const stringview_type& str, size_t pos/* = npos */) const noexcept {
    Assert(not str.empty());

    auto it = std::search(pos == npos ? rbegin() : const_reverse_iterator(begin() + pos), rend(), str.rbegin(), str.rend());
    return (it == rend() ? npos : rend() - it);
}
//----------------------------------------------------------------------------
template <typename _Char>
auto TBasicString<_Char>::substr(size_t pos/* = 0 */, size_t len/* = npos */) const -> TBasicString {
    return TBasicString(MakeView().SubRange(pos, len));
}
//----------------------------------------------------------------------------
template <typename _Char>
void TBasicString<_Char>::resize(size_t newSize, _Char fill) {
    const size_t oldSize = size();
    auto dst = resizeNoNullChar_(newSize);
    if (newSize > oldSize)
        std::fill_n(dst + oldSize, newSize - oldSize, fill);
    dst[newSize] = _Char();
}
//----------------------------------------------------------------------------
template <typename _Char>
void TBasicString<_Char>::shrink_to_fit() {
    if (is_large_()) {
        const size_t newCapacity = AllocatorSnapSize(get_allocator_(), _large.Size + 1/* null char */);

        if (newCapacity < _large.Capacity) {
            if (USE_CORE_BASICSTRING_SBO && newCapacity <= FSmallString_::GCapacity) {
                _small.IsLarge = 0;
                _small.Size = _large.Size;
                ::memcpy(_small.Buffer, _large.Storage, _large.Size * sizeof(_Char));
            }
            else {
                Relocate_AssumePod(get_allocator_(), mutableview_type(_large.Storage, _large.Size + 1), newCapacity, _large.Capacity);
            }
        }
    }
}
//----------------------------------------------------------------------------
template <typename _Char>
void TBasicString<_Char>::clear() {
    if (is_large_()) {
        _large.Size = 0;
        if (_large.Storage)
            _large.Storage[0] = _Char();
    }
    else {
        _small.Size = 0;
        _small.Buffer[0] = _Char();
    }
}
//----------------------------------------------------------------------------
template <typename _Char>
void TBasicString<_Char>::clear_ReleaseMemory() {
    if (is_large_()) {
        Assert(_large.Storage);
        allocator_traits::deallocate(get_allocator_(), _large.Storage, _large.Capacity);
    }
    _large = { 0, 0, 0, nullptr };
    Assert(not is_large_());
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicString<_Char> operator +(const TBasicString<_Char>& lhs, const TBasicString<_Char>& rhs) {
    const size_t new_size = (lhs.size() + rhs.size());
    TBasicString<_Char> result;
    result.reserve(new_size);
    result.assign(lhs);
    result.append(rhs);
    return result;
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicString<_Char> operator +(TBasicString<_Char>&& lhs, TBasicString<_Char>&& rhs) {
    const size_t new_size = (lhs.size() + rhs.size());
    if (capacity < rhs.capacity()) {
        rhs.insert(0, lhs);
        return std::move(rhs);
    }
    else {
        lhs.append(rhs);
        return std::move(lhs);
    }
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicString<_Char> operator +(TBasicString<_Char>&& lhs, const TBasicString<_Char>& rhs) {
    lhs.append(rhs);
    return std::move(lhs);
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicString<_Char> operator +(const TBasicString<_Char>& lhs, TBasicString<_Char>&& rhs) {
    rhs.insert(0, lhs);
    return std::move(rhs);
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicString<_Char> operator +(const TBasicString<_Char>& lhs, const TBasicStringView<_Char>& rhs) {
    const size_t new_size = (lhs.size() + rhs.size());
    TBasicString<_Char> result;
    result.reserve(new_size);
    result.assign(lhs);
    result.append(rhs);
    return result;
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicString<_Char> operator +(TBasicString<_Char>&& lhs, const TBasicStringView<_Char>& rhs) {
    lhs.append(rhs);
    return std::move(lhs);
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicString<_Char> operator +(const TBasicStringView<_Char>& lhs, const TBasicString<_Char>& rhs) {
    const size_t new_size = (lhs.size() + rhs.size());
    TBasicString<_Char> result;
    result.reserve(new_size);
    result.assign(lhs);
    result.append(rhs);
    return result;
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicString<_Char> operator +(const TBasicStringView<_Char>& lhs, TBasicString<_Char>&& rhs) {
    rhs.insert(0, lhs);
    return std::move(rhs);
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicString<_Char> operator +(const TBasicString<_Char>& lhs, _Char rhs) {
    TBasicString<_Char> result;
    result.reserve(lhs.size() + 1);
    result.assign(lhs);
    result.append(rhs);
    return result;
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicString<_Char> operator +(TBasicString<_Char>&& lhs, _Char rhs) {
    lhs.append(rhs);
    return std::move(lhs);
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicString<_Char> operator +(_Char lhs, const TBasicString<_Char>& rhs) {
    TBasicString<_Char> result;
    result.reserve(rhs.size() + 1);
    result.append(lhs);
    result.append(rhs);
    return result;
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicString<_Char> operator +(_Char lhs, TBasicString<_Char>&& rhs) {
    rhs.insert(0, lhs);
    return std::move(rhs);
}
//----------------------------------------------------------------------------
template <typename _Char>
void TBasicString<_Char>::gsub(_Char from, _Char to) {
    for (_Char& ch : MutableView())
        if (ch == from)
            ch = to;
}
//----------------------------------------------------------------------------
template <typename _Char>
void TBasicString<_Char>::gsub(_Char from, const stringview_type& to) {
    Assert(to.empty());

    for (size_t pos = 0; pos < size(); ++pos) {
        if (at(pos) == from) {
            replace(pos, 1, to);
            pos += (to.size() - 1);
        }
    }
}
//----------------------------------------------------------------------------
template <typename _Char>
void TBasicString<_Char>::gsub(const stringview_type& from, const stringview_type& to) {
    for (size_t pos = 0;;) {
        pos = find(from, pos);
        if (npos == pos)
            break;

        replace(pos, from.size(), to);
        pos += from.size();
    }
}
//----------------------------------------------------------------------------
template <typename _Char>
TMemoryView<_Char> TBasicString<_Char>::clear_StealMemoryUnsafe(size_t* plen/* = nullptr */) {
    if (is_large_()) {
        const TMemoryView<_Char> stolen(_large.Storage, _large.Capacity);
        if (plen) *plen = _large.Size;
        _large = { 0, 0, 0, nullptr };
        return stolen;
    }
    return TMemoryView<_Char>();
}
//----------------------------------------------------------------------------
template <typename _Char>
void TBasicString<_Char>::assign(Meta::FForceInit, TMemoryView<_Char>& stolen, size_t len/* = 0 */) {
    clear_ReleaseMemory();
    _large = { stolen.data() != nullptr, len, stolen.size(), stolen.data() };
    Assert(CheckInvariants());
}
//----------------------------------------------------------------------------
template <typename _Char>
bool TBasicString<_Char>::CheckInvariants() const {
#ifndef NDEBUG
    if (_large.IsLarge != _large.IsLarge)
        return false;
    if (_large.IsLarge) {
        if (nullptr == _large.Storage)
            return false;
        /* not necessarily true due to "steal" ctor
        if (_large.Size >= _large.Capacity)
            return false;
            */
        if (_large.Storage[_large.Size] != _Char())
            return false;
    }
    else {
        if (_small.Buffer[size_t(_small.Size)] != _Char())
            return false;
        if (USE_CORE_BASICSTRING_SBO && _large.Capacity <= FSmallString_::GCapacity)
            return false;
        if (_small.Size >= FSmallString_::GCapacity)
            return false;
    }
#endif
    return true;
}
//----------------------------------------------------------------------------
template <typename _Char>
void TBasicString<_Char>::reserve(size_t count) {
    if (count > capacity()) {
        const size_t sz = size();
        auto first = resizeNoNullChar_(count, false);
        Assert(size() == sz);
        first[sz] = _Char();
    }
}
//----------------------------------------------------------------------------
template <typename _Char>
TCheckedArrayIterator<_Char> TBasicString<_Char>::resizeNoNullChar_(size_t count, bool change_size/* = true */) {
    return ((USE_CORE_BASICSTRING_SBO && count < FSmallString_::GCapacity)
        ? resizeNoNullChar_Small_(count, change_size)
        : resizeNoNullChar_Large_(count, change_size) );
}
//----------------------------------------------------------------------------
template <typename _Char>
NO_INLINE TCheckedArrayIterator<_Char> TBasicString<_Char>::resizeNoNullChar_Large_(size_t count, bool change_size) {
    Assert(CheckInvariants());

    const size_t new_capacity = AllocatorSnapSize(get_allocator_(), count + 1);
    Assert(new_capacity > FSmallString_::GCapacity);

    if (is_large_()) {
        _large.Storage = Relocate_AssumePod(
            get_allocator_(),
            mutableview_type(_large.Storage, _large.Size),
            new_capacity,
            _large.Capacity );
    }
    else {
        _large.IsLarge = 1;
        _Char* const new_ptr = allocator_traits::allocate(get_allocator_(), new_capacity);
        ::memcpy(new_ptr, _small.Buffer, Min(_large.Size, count) * sizeof(_Char));
    }

    _large.Capacity = new_capacity;
    if (change_size)
        _large.Size = count;

    Assert(CheckInvariants());

    return MakeCheckedIterator(_large.Storage, count + 1, 0);
}
//----------------------------------------------------------------------------
template <typename _Char>
NO_INLINE TCheckedArrayIterator<_Char> TBasicString<_Char>::resizeNoNullChar_Small_(size_t count, bool change_size) {
    Assert(CheckInvariants());
    Assert(count < FSmallString_::GCapacity);

    if (is_large_()) {
        Assert(_large.Storage);
        _small.IsLarge = 0;
        _Char* const prev_ptr = _large.Storage;
        const size_t prev_capacity = _large.Capacity;
        ::memcpy(_small.Buffer, prev_ptr, Min(_large.Size, count) * sizeof(_Char));
        allocator_traits::deallocate(get_allocator_(), prev_ptr, prev_capacity);
    }

    if (change_size)
        _small.Size = count;

    Assert(CheckInvariants());

    return MakeCheckedIterator(_small.Buffer, count + 1, 0);
}
//----------------------------------------------------------------------------
template <typename _Char>
TCheckedArrayIterator<_Char> TBasicString<_Char>::resizeWNullChar_(size_t count) {
    auto dst = resizeNoNullChar_(count);
    dst[count] = _Char();
    return dst;
}
//----------------------------------------------------------------------------
/*extern CORE_API*/ template class TBasicString<char>;
/*extern CORE_API*/ template class TBasicString<wchar_t>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
size_t ToCStr(char *dst, size_t capacity, const wchar_t *wcstr, size_t length) {
    Assert(dst);
    Assert(capacity);
    if (0 == length) {
        dst[0] = '\0';
        return 0;
    }

    Assert(wcstr);

    size_t written;
    VerifyRelease(::wcstombs_s(&written, dst, capacity, wcstr, capacity - 1) == 0);

    Assert(written >= length);
    Assert(written < capacity);
    Assert(dst[written] == '\0');
    return written;
}
//----------------------------------------------------------------------------
size_t ToCStr(char *dst, size_t capacity, const wchar_t *wcstr) {
    return ToCStr(dst, capacity, wcstr, ::wcslen(wcstr));
}
//----------------------------------------------------------------------------
size_t ToCStr(char *dst, size_t capacity, const FWString& wstr) {
    return ToCStr(dst, capacity, wstr.c_str(), wstr.size());
}
//----------------------------------------------------------------------------
size_t ToWCStr(wchar_t *dst, size_t capacity, const char *cstr, size_t length) {
    Assert(dst);
    Assert(capacity);
    if (0 == length) {
        dst[0] = L'\0';
        return 0;
    }

    Assert(cstr);

    size_t written;
    VerifyRelease(::mbstowcs_s(&written, dst, capacity, cstr, capacity - 1) == 0);

    Assert(written >= length);
    Assert(written < capacity);
    Assert(dst[written] == L'\0');
    return written;
}
//----------------------------------------------------------------------------
size_t ToWCStr(wchar_t *dst, size_t capacity, const char *cstr) {
    return ToWCStr(dst, capacity, cstr, ::strlen(cstr));
}
//----------------------------------------------------------------------------
size_t ToWCStr(wchar_t *dst, size_t capacity, const FString& str) {
    return ToWCStr(dst, capacity, str.c_str(), str.size());
}
//----------------------------------------------------------------------------
FStringView ToCStr(const TMemoryView<char>& dst, const FWStringView& wstr) {
    const size_t len = ToCStr(dst.data(), dst.size(), wstr.data(), wstr.size());
    Assert(len + 1 == wstr.size());
    return FStringView(dst.data(), len - 1/* \0 */);
}
//----------------------------------------------------------------------------
FWStringView ToWCStr(const TMemoryView<wchar_t>& dst, const FStringView& str) {
    const size_t len = ToWCStr(dst.data(), dst.size(), str.data(), str.size());
    Assert(len + 1 == str.size());
    return FWStringView(dst.data(), len - 1/* \0 */);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FString ToString(const wchar_t *wcstr, size_t length) {
    if (0 == length)
        return FString();

    Assert(wcstr);

    // allocated with (state less) string allocator to let the result steal this pointer
    FString::allocator_type string_alloc;
    const size_t capacity = AllocatorSnapSize(string_alloc, length + 1);
    char* const ptr = string_alloc.allocate(capacity);

    const size_t written = ToCStr(ptr, capacity, wcstr, length);
    return FString(Meta::FForceInit{}, TMemoryView<char>(ptr, capacity), written);
}
//----------------------------------------------------------------------------
FString ToString(const wchar_t *wcstr) {
    Assert(wcstr);
    return ToString(wcstr, ::wcslen(wcstr));
}
//----------------------------------------------------------------------------
FString ToString(const FWString& wstr) {
    return ToString(wstr.c_str(), wstr.size());
}
//----------------------------------------------------------------------------
FWString ToWString(const char *cstr, size_t length) {
    if (0 == length)
        return FWString();

    Assert(cstr);

    // allocated with (state less) wstring allocator to let the result steal this pointer
    FWString::allocator_type wstring_alloc;
    const size_t capacity = AllocatorSnapSize(wstring_alloc, length + 1);
    wchar_t* const ptr = wstring_alloc.allocate(capacity);

    const size_t written = ToWCStr(ptr, capacity, cstr, length);
    return FWString(Meta::FForceInit{}, TMemoryView<wchar_t>(ptr, capacity), written);
}
//----------------------------------------------------------------------------
FWString ToWString(const char *cstr) {
    Assert(cstr);
    return ToWString(cstr, ::strlen(cstr));
}
//----------------------------------------------------------------------------
FWString ToWString(const FString& str) {
    return ToWString(str.c_str(), str.size());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
