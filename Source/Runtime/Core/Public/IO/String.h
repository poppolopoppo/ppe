#pragma once

#include "Core.h"

#include "Allocator/Allocation.h"
#include "IO/String_fwd.h"
#include "IO/StringConversion.h"
#include "IO/StringView.h"
#include "HAL/PlatformString.h"

#include <algorithm>
#include <initializer_list>
#include <iterator>

// Disable small buffer optimizations when debugging memory to catch overflows
#define USE_PPE_BASICSTRING_SBO (!USE_PPE_MEMORY_DEBUGGING)

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
using FStringAllocator = ALLOCATOR(String);
//----------------------------------------------------------------------------
template <typename _Char>
class TBasicString : private FStringAllocator {
public:
    typedef _Char char_type;
    typedef size_t size_type;

    typedef FStringAllocator allocator_type;
    typedef TAllocatorTraits<allocator_type> allocator_traits;

    typedef TMemoryView<_Char> mutableview_type;
    typedef TBasicStringLiteral<_Char> stringliteral_type;
    typedef TBasicStringView<_Char> stringview_type;
    typedef TBasicStringConversion<_Char> stringconvertion_type;

    typedef TCheckedArrayIterator<_Char> iterator;
    typedef std::reverse_iterator<iterator> reverse_iterator;

    typedef TCheckedArrayIterator<const _Char> const_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    STATIC_CONST_INTEGRAL(size_t, npos, INDEX_NONE);

    TBasicString() noexcept : _large{0, 0, 0, nullptr} {}
    ~TBasicString();

    TBasicString(const _Char* s, size_t len) : TBasicString() { assign(stringview_type(s, len)); }
    TBasicString(size_t n, _Char fill) : TBasicString() { assign(n, fill); }
    explicit TBasicString(std::initializer_list<_Char> il) : TBasicString() { assign(il); }

    template <typename _It, class = Meta::TEnableIf<Meta::is_iterator_v<_It>> >
    TBasicString(_It first, _It last) : TBasicString() { assign(first, last); }

    template <size_t _Dim>
    TBasicString(const _Char(&staticStr)[_Dim]) : TBasicString(MakeStringView(staticStr)) {}
    template <size_t _Dim>
    TBasicString& operator =(const _Char(&staticStr)[_Dim]) { assign(MakeStringView(staticStr)); return (*this); }

    explicit TBasicString(const stringliteral_type& str) : TBasicString() { assign(str.MakeView()); }
    TBasicString& operator =(const stringliteral_type& str) { assign(str.MakeView()); return (*this); }

    explicit TBasicString(const stringview_type& str) : TBasicString() { assign(str); }
    TBasicString& operator =(const stringview_type& str) { assign(str); return (*this); }

    TBasicString(const TBasicString& other) : TBasicString() { assign(other); }
    TBasicString& operator =(const TBasicString& other) { assign(other); return (*this); }

    TBasicString(TBasicString&& rvalue) NOEXCEPT : TBasicString() { assign(std::move(rvalue)); }
    TBasicString& operator =(TBasicString&& rvalue) NOEXCEPT { assign(std::move(rvalue)); return (*this); }

    bool empty() const { return (0 == size()); }
    size_t length() const { return size(); } // stl compat
    size_t size() const { return (is_large_() ? _large.Size : _small.Size); }
    size_t capacity() const { return (is_large_() ? _large.Capacity : FSmallString_::GCapacity); }
    size_t max_size() const { return (allocator_traits::MaxSize(*this) / sizeof(_Char)); }

    size_t SizeInBytes() const { return (size() * sizeof(_Char)); }

    iterator begin() { return MakeIterator_(0); }
    iterator end() { return MakeIterator_(size()); }

    reverse_iterator rbegin() { return reverse_iterator(end()); }
    reverse_iterator rend() { return reverse_iterator(begin()); }

    const_iterator begin() const { return MakeConstIterator_(0); }
    const_iterator end() const { return MakeConstIterator_(size()); }

    const_iterator cbegin() const { return begin(); }
    const_iterator cend() const { return end(); }

    const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
    const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }

    const _Char* c_str() const { return data_(); }
    const _Char* data() const { return data_(); }
    const _Char* operator *() const { return data_(); }

    FStringAllocator& get_allocator() { return allocator_traits::Get(*this); }
    const FStringAllocator& get_allocator() const { return allocator_traits::Get(*this); }

    _Char& at(size_t index) { Assert(index < size()); return data_()[index]; }
    _Char& operator [](size_t index) { return at(index); }

    _Char at(size_t index) const { Assert(index < size()); return data()[index]; }
    _Char operator [](size_t index) const { return at(index); }

    _Char front() const { return MakeView().front(); }
    _Char back() const { return MakeView().back(); }

    void assign(TBasicString&& rvalue);
    void assign(const TBasicString& other) { assign(other.MakeView()); }
    void assign(const stringview_type& str);
    void assign(std::initializer_list<_Char> il) { assign(il.begin(), il.end()); }
    void assign(size_t n, _Char fill);
    template <typename _It, class = Meta::TEnableIf<Meta::is_iterator_v<_It>> >
    void assign(_It first, _It last);
    template <size_t _Dim>
    void assign(const _Char(&staticArray)[_Dim]) { assign(MakeStringView(staticArray)); }
    void assign(const _Char* cstr, size_t len) { assign(MakeCheckedIterator(cstr, len, 0), MakeCheckedIterator(cstr, len, len)); }

    void append(_Char ch) { append(stringview_type(&ch, 1)); }
    void append(const TBasicString& other) { append(other.MakeView()); }
    void append(const stringview_type& str);
    void append(std::initializer_list<_Char> il) { append(il.begin(), il.end()); }
    void append(size_t n, _Char fill);
    template <typename _It, class = Meta::TEnableIf<Meta::is_iterator_v<_It>> >
    void append(_It first, _It last);
    template <size_t _Dim>
    void append(const _Char(&staticArray)[_Dim]) { append(MakeStringView(staticArray)); }

    void insert(size_t pos, _Char ch) { insert(pos, stringview_type(&ch, 1)); }
    void insert(size_t pos, const TBasicString& other) { insert(pos, other.MakeView()); }
    void insert(size_t pos, const stringview_type& str);
    void insert(size_t pos, std::initializer_list<_Char> il) { insert(pos, il.begin(), il.end()); }
    void insert(size_t pos, size_t n, _Char fill);
    template <typename _It, class = Meta::TEnableIf<Meta::is_iterator_v<_It>> >
    void insert(size_t pos, _It first, _It last);
    template <size_t _Dim>
    void insert(size_t pos, const _Char(&staticArray)[_Dim]) { insert(pos, MakeStringView(staticArray)); }

    void insert(iterator it, _Char ch) { insert(it - begin(), stringview_type(&ch, 1)); }
    void insert(iterator it, const TBasicString& other) { insert(it - begin(), other.MakeView()); }
    void insert(iterator it, size_t n, _Char fill) { insert(it - begin(), n, fill); }
    void insert(iterator it, std::initializer_list<_Char> il) { insert(it - begin(), il.begin(), il.end()); }
    void insert(iterator it, const stringview_type& str) { insert(it - begin(), str); }
    template <typename _It, class = Meta::TEnableIf<Meta::is_iterator_v<_It>> >
    void insert(iterator it, _It first, _It last) { insert(it - begin(), first, last); }
    template <size_t _Dim>
    void insert(iterator it, const _Char(&staticArray)[_Dim]) { insert(it, MakeStringView(staticArray)); }

    void push_back(_Char ch) { append(ch); }
    void pop_back() { resize(size() - 1); }

    void erase(size_t pos = 0, size_t len = npos);
    void erase(iterator first, iterator last) { erase(first - begin(), last - first); }

    void replace(size_t pos, size_t len, const TBasicString& other) { replace(pos, len, other.MakeView()); }
    void replace(size_t pos, size_t len, const stringview_type& str);
    void replace(size_t pos, size_t len, std::initializer_list<_Char> il) { replace(pos, len, il.begin(), il.end()); }
    void replace(size_t pos, size_t len, size_t n, _Char fill);
    template <typename _It, class = Meta::TEnableIf<Meta::is_iterator_v<_It>> >
    void replace(size_t pos, size_t len, _It first, _It last);
    template <size_t _Dim>
    void replace(size_t pos, size_t len, const _Char(&staticArray)[_Dim]) { replace(pos, len, MakeStringView(staticArray)); }

    void replace(iterator from, iterator to, const TBasicString& other) { replace(from - begin(), to - from, other.MakeView()); }
    void replace(iterator from, iterator to, const stringview_type& str) { replace(from - begin(), to - from, str); }
    void replace(iterator from, iterator to, std::initializer_list<_Char> il) { replace(from - begin(), to - from, il.begin(), il.end()); }
    void replace(iterator from, iterator to, size_t n, _Char fill) { replace(from - begin(), to - from, n, fill); }
    template <typename _It, class = Meta::TEnableIf<Meta::is_iterator_v<_It>> >
    void replace(iterator from, iterator to, _It first, _It last) { replace(from - begin(), to - from, first, last); }
    template <size_t _Dim>
    void replace(iterator from, iterator to, const _Char(&staticArray)[_Dim]) { replace(from, to, MakeStringView(staticArray)); }

    void copy(_Char* s, size_t len, size_t pos = 0) const { std::copy(begin() + pos, end(), MakeCheckedIterator(s, len, 0)); }
    void copy(const TMemoryView<_Char>& dst, size_t pos = 0) const { std::copy(begin() + pos, end(), dst.begin()); }

    template <typename _Pred>
    size_t find_if(_Pred pred, size_t pos = 0) const noexcept;
    template <typename _Pred>
    size_t rfind_if(_Pred pred, size_t pos = npos) const noexcept;

    size_t find(const TBasicString& other, size_t pos = 0) const noexcept { return find(other.MakeView(), pos); }
    size_t find(const stringview_type& str, size_t pos = 0) const noexcept;
    size_t find(_Char ch, size_t pos = 0) const noexcept { return find_if([ch](_Char in) { return (in == ch); }, pos); }
    template <size_t _Dim>
    size_t find(const _Char(&staticArray)[_Dim], size_t pos = 0) const noexcept { return find(MakeStringView(staticArray), pos); }

    size_t rfind(const TBasicString& other, size_t pos = npos) const noexcept { return rfind(other.MakeView(), pos); }
    size_t rfind(const stringview_type& str, size_t pos = npos) const noexcept;
    size_t rfind(_Char ch, size_t pos = npos) const noexcept { return rfind_if([ch](_Char in) { return (in == ch); }, pos); }
    template <size_t _Dim>
    size_t rfind(const _Char(&staticArray)[_Dim], size_t pos = npos) const noexcept { return rfind(MakeStringView(staticArray), pos); }

    size_t find_first_of(const TBasicString& other, size_t pos = 0) const noexcept { return find_first_of(other.MakeView(), pos); }
    size_t find_first_of(const stringview_type& str, size_t pos = 0) const noexcept { return find_if([&](_Char in) { return str.Contains(in); }, pos); }
    size_t find_first_of(_Char ch, size_t pos = 0) const noexcept { return find(ch, pos); }
    template <size_t _Dim>
    size_t find_first_of(const _Char(&staticArray)[_Dim], size_t pos = 0) const noexcept { return find_first_of(MakeStringView(staticArray), pos); }

    size_t find_last_of(const TBasicString& other, size_t pos = npos) const noexcept { return find_last_of(other.MakeView(), pos); }
    size_t find_last_of(const stringview_type& str, size_t pos = npos) const noexcept { return rfind_if([&](_Char in) { return str.Contains(in); }, pos); }
    size_t find_last_of(_Char ch, size_t pos = npos) const noexcept { return rfind(ch, pos); }
    template <size_t _Dim>
    size_t find_last_of(const _Char(&staticArray)[_Dim], size_t pos = npos) const noexcept { return find_last_of(MakeStringView(staticArray), pos); }

    size_t find_first_not_of(const TBasicString& other, size_t pos = 0) const noexcept { return find_first_not_of(other.MakeView(), pos); }
    size_t find_first_not_of(const stringview_type& str, size_t pos = 0) const noexcept { return find_if([&](_Char in) { return (not str.Contains(in)); }, pos); }
    size_t find_first_not_of(_Char ch, size_t pos = 0) const noexcept { return find_if([ch](_Char in) { return (in != ch); }, pos); }
    template <size_t _Dim>
    size_t find_first_not_of(const _Char(&staticArray)[_Dim], size_t pos = 0) const noexcept { return find_first_not_of(MakeStringView(staticArray), pos); }

    size_t find_last_not_of(const TBasicString& other, size_t pos = npos) const noexcept { return find_last_not_of(other.MakeView(), pos); }
    size_t find_last_not_of(const stringview_type& str, size_t pos = npos) const noexcept { return rfind_if([&](_Char in) { return (not str.Contains(in)); }, pos); }
    size_t find_last_not_of(_Char ch, size_t pos = npos) const noexcept { return rfind_if([ch](_Char in) { return (in != ch); }, pos); }
    template <size_t _Dim>
    size_t find_last_not_of(const _Char(&staticArray)[_Dim], size_t pos = npos) const noexcept { return find_last_not_of(MakeStringView(staticArray), pos); }

    TBasicString substr(size_t pos = 0, size_t len = npos) const;

    void reserve(size_t count);
    void reserve_Additional(size_t count) { reserve(capacity() + count); }
    void resize(size_t newSize) { resize(newSize, _Char()); }
    void resize(size_t newSize, _Char fill);
    void shrink_to_fit();

    void clear();
    void clear_ReleaseMemory();

    int compare(const stringview_type& str) const noexcept { return Compare(MakeView(), str); }
    int compare(const TBasicString& other) const noexcept { return Compare(MakeView(), other.MakeView()); }
    template <size_t _Dim>
    int compare(const _Char(&staticArray)[_Dim]) const noexcept { return Compare(MakeView(), MakeStringView(staticArray)); }

    TBasicString& operator +=(_Char ch) { append(ch); return (*this); }
    TBasicString& operator +=(const stringliteral_type& str) { append(str.MakeView()); return (*this); }
    TBasicString& operator +=(const stringview_type& str) { append(str); return (*this); }
    TBasicString& operator +=(const TBasicString& other) { append(other); return (*this); }
    TBasicString& operator +=(std::initializer_list<_Char> il) { append(il.begin(), il.end()); return (*this); }
    template <size_t _Dim>
    TBasicString& operator +=(const _Char(&staticArray)[_Dim]) { append(MakeStringView(staticArray)); return (*this); }

    inline friend TBasicString operator +(const TBasicString& lhs, const TBasicString& rhs) { return Concat(lhs, rhs); }
    inline friend TBasicString operator +(TBasicString&& lhs, TBasicString&& rhs) { return Concat(std::move(lhs), std::move(rhs)); }
    inline friend TBasicString operator +(TBasicString&& lhs, const TBasicString& rhs) { return Concat(std::move(lhs), rhs); }
    inline friend TBasicString operator +(const TBasicString& lhs, TBasicString&& rhs) { return Concat(lhs, std::move(rhs)); }

    inline friend TBasicString operator +(const TBasicString& lhs, const stringview_type& rhs) { return Concat(lhs, rhs); }
    inline friend TBasicString operator +(TBasicString&& lhs, const stringview_type& rhs) { return Concat(std::move(lhs), rhs); }
    inline friend TBasicString operator +(const stringview_type& lhs, const TBasicString& rhs) { return Concat(lhs, rhs); }
    inline friend TBasicString operator +(const stringview_type& lhs, TBasicString&& rhs) { return Concat(lhs, std::move(rhs)); }

    inline friend TBasicString operator +(const TBasicString& lhs, const stringliteral_type& rhs) { return Concat(lhs, rhs.MakeView()); }
    inline friend TBasicString operator +(const stringliteral_type& lhs, const TBasicString& rhs) { return Concat(lhs.MakeView(), rhs); }

    inline friend TBasicString operator +(const TBasicString& lhs, _Char rhs) { return Concat(lhs, rhs); }
    inline friend TBasicString operator +(TBasicString&& lhs, _Char rhs) { return Concat(std::move(lhs), rhs); }
    inline friend TBasicString operator +(_Char lhs, const TBasicString& rhs) { return Concat(lhs, rhs); }
    inline friend TBasicString operator +(_Char lhs, TBasicString&& rhs) { return Concat(lhs, std::move(rhs)); }

    template <size_t _Dim>
    inline friend TBasicString operator +(const TBasicString& lhs, const _Char(&rhs)[_Dim]) { return operator +(lhs, MakeStringView(rhs)); }
    template <size_t _Dim>
    inline friend TBasicString operator +(TBasicString&& lhs, const _Char(&rhs)[_Dim]) { return operator +(std::move(lhs), MakeStringView(rhs)); }
    template <size_t _Dim>
    inline friend TBasicString operator +(const _Char(&lhs)[_Dim], const TBasicString& rhs) { return operator +(MakeStringView(lhs), rhs); }
    template <size_t _Dim>
    inline friend TBasicString operator +(const _Char(&lhs)[_Dim], TBasicString&& rhs) { return operator +(MakeStringView(lhs), std::move(rhs)); }

    inline friend bool operator ==(const TBasicString& lhs, const TBasicString& rhs) { return Equals(lhs.MakeView(), rhs.MakeView()); }
    inline friend bool operator ==(const stringview_type& lhs, const TBasicString& rhs) { return Equals(lhs, rhs.MakeView()); }
    inline friend bool operator ==(const TBasicString& lhs, const stringview_type& rhs) { return Equals(lhs.MakeView(), rhs); }

    inline friend bool operator !=(const TBasicString& lhs, const TBasicString& rhs) { return (not operator ==(lhs, rhs)); }
    inline friend bool operator !=(const stringview_type& lhs, const TBasicString& rhs) { return (not operator ==(lhs, rhs)); }
    inline friend bool operator !=(const TBasicString& lhs, const stringview_type& rhs) { return (not operator ==(lhs, rhs)); }

    inline friend bool operator < (const TBasicString& lhs, const TBasicString& rhs) { return (Compare(lhs.MakeView(), rhs.MakeView()) < 0); }
    inline friend bool operator < (const stringview_type& lhs, const TBasicString& rhs) { return (Compare(lhs, rhs.MakeView()) < 0); }
    inline friend bool operator < (const TBasicString& lhs, const stringview_type& rhs) { return (Compare(lhs.MakeView(), rhs) < 0); }

    inline friend bool operator >=(const TBasicString& lhs, const TBasicString& rhs) { return (not operator <(lhs, rhs)); }
    inline friend bool operator >=(const stringview_type& lhs, const TBasicString& rhs) { return (not operator <(lhs, rhs)); }
    inline friend bool operator >=(const TBasicString& lhs, const stringview_type& rhs) { return (not operator <(lhs, rhs)); }

    inline friend bool operator > (const TBasicString& lhs, const TBasicString& rhs) { return (Compare(lhs.MakeView(), rhs.MakeView()) > 0); }
    inline friend bool operator > (const stringview_type& lhs, const TBasicString& rhs) { return (Compare(lhs, rhs.MakeView()) > 0); }
    inline friend bool operator > (const TBasicString& lhs, const stringview_type& rhs) { return (Compare(lhs.MakeView(), rhs) > 0); }

    inline friend bool operator <=(const TBasicString& lhs, const TBasicString& rhs) { return (not operator >(lhs, rhs)); }
    inline friend bool operator <=(const stringview_type& lhs, const TBasicString& rhs) { return (not operator >(lhs, rhs)); }
    inline friend bool operator <=(const TBasicString& lhs, const stringview_type& rhs) { return (not operator >(lhs, rhs)); }

    template <size_t _Dim>
    inline friend bool operator ==(const TBasicString& lhs, const _Char(&rhs)[_Dim]) { return Equals(lhs.MakeView(), MakeStringView(rhs)); }
    template <size_t _Dim>
    inline friend bool operator !=(const TBasicString& lhs, const _Char(&rhs)[_Dim]) { return not operator ==(lhs, rhs); }
    template <size_t _Dim>
    inline friend bool operator < (const TBasicString& lhs, const _Char(&rhs)[_Dim]) { return (Compare(lhs.MakeView(), MakeStringView(rhs)) <  0); }
    template <size_t _Dim>
    inline friend bool operator <=(const TBasicString& lhs, const _Char(&rhs)[_Dim]) { return (Compare(lhs.MakeView(), MakeStringView(rhs)) <= 0); }
    template <size_t _Dim>
    inline friend bool operator > (const TBasicString& lhs, const _Char(&rhs)[_Dim]) { return (Compare(lhs.MakeView(), MakeStringView(rhs)) >  0); }
    template <size_t _Dim>
    inline friend bool operator >=(const TBasicString& lhs, const _Char(&rhs)[_Dim]) { return (Compare(lhs.MakeView(), MakeStringView(rhs)) >= 0); }

    template <size_t _Dim>
    inline friend bool operator ==(const _Char(&lhs)[_Dim], const TBasicString& rhs) { return Equals(MakeStringView(lhs), rhs.MakeView()); }
    template <size_t _Dim>
    inline friend bool operator !=(const _Char(&lhs)[_Dim], const TBasicString& rhs) { return not operator ==(lhs, rhs); }
    template <size_t _Dim>
    inline friend bool operator < (const _Char(&lhs)[_Dim], const TBasicString& rhs) { return (Compare(MakeStringView(lhs), rhs.MakeView()) <  0); }
    template <size_t _Dim>
    inline friend bool operator <=(const _Char(&lhs)[_Dim], const TBasicString& rhs) { return (Compare(MakeStringView(lhs), rhs.MakeView()) <= 0); }
    template <size_t _Dim>
    inline friend bool operator > (const _Char(&lhs)[_Dim], const TBasicString& rhs) { return (Compare(MakeStringView(lhs), rhs.MakeView()) >  0); }
    template <size_t _Dim>
    inline friend bool operator >=(const _Char(&lhs)[_Dim], const TBasicString& rhs) { return (Compare(MakeStringView(lhs), rhs.MakeView()) >= 0); }

    inline friend void swap(TBasicString& lhs, TBasicString& rhs) NOEXCEPT { std::swap(lhs._large, rhs._large); }

    static TBasicString Concat(const TBasicString& lhs, const TBasicString& rhs);
    static TBasicString Concat(TBasicString&& lhs, TBasicString&& rhs);
    static TBasicString Concat(TBasicString&& lhs, const TBasicString& rhs);
    static TBasicString Concat(const TBasicString& lhs, TBasicString&& rhs);

    static TBasicString Concat(const TBasicString& lhs, const stringview_type& rhs);
    static TBasicString Concat(TBasicString&& lhs, const stringview_type& rhs);
    static TBasicString Concat(const stringview_type& lhs, const TBasicString& rhs);
    static TBasicString Concat(const stringview_type& lhs, TBasicString&& rhs);

    static TBasicString Concat(const TBasicString& lhs, _Char rhs);
    static TBasicString Concat(TBasicString&& lhs, _Char rhs);
    static TBasicString Concat(_Char lhs, const TBasicString& rhs);
    static TBasicString Concat(_Char lhs, TBasicString&& rhs);

public: // non stl
    // memory stealing between TBasicStringBuilder & TBasicString

    explicit TBasicString(TBasicStringBuilder<_Char>&& sb) NOEXCEPT;
    void assign(TBasicStringBuilder<_Char>&& sb);

    NODISCARD bool AcquireDataUnsafe(FAllocatorBlock b, size_t sz) NOEXCEPT;
    NODISCARD FAllocatorBlock StealDataUnsafe(size_t* sz = nullptr) NOEXCEPT;

    // can be implicitly casted to TBasicStringView<> since it's cheap and convenient
    operator TBasicStringView<_Char> () const { return MakeView(); }

    int compareI(const stringview_type& str) const { return CompareI(MakeView(), str); }
    int compareI(const TBasicString& other) const { return CompareI(MakeView(), other.MakeView()); }

    int compare(const stringview_type& str, ECase sensitive) const { return (sensitive == ECase::Sensitive ? compare(str) : compareI(str)); }
    int compare(const TBasicString& other, ECase sensitive) const { return (sensitive == ECase::Sensitive ? compare(other) : compareI(other)); }

    // note : equals() is faster than compare() == 0
    bool equals(const stringview_type& str) const { return Equals(MakeView(), str); }
    bool equals(const TBasicString& other) const { return Equals(MakeView(), other.MakeView()); }

    bool equalsI(const stringview_type& str) const { return EqualsI(MakeView(), str); }
    bool equalsI(const TBasicString& other) const { return EqualsI(MakeView(), other.MakeView()); }

    int equals(const stringview_type& str, ECase sensitive) const { return (sensitive == ECase::Sensitive ? equals(str) : equalsI(str)); }
    int equals(const TBasicString& other, ECase sensitive) const { return (sensitive == ECase::Sensitive ? equals(other) : equalsI(other)); }

    void to_lower() { InplaceToLower(MutableView()); }
    void to_upper() { InplaceToUpper(MutableView()); }

    bool gsub(_Char from, const TBasicString& to) { return gsub(from, to.MakeView()); }
    bool gsub(const TBasicString& from, const TBasicString& to) { return gsub(from.MakeView(), to.MakeView()); }
    bool gsub(_Char from, _Char to);
    bool gsub(_Char from, const stringview_type& to);
    bool gsub(const stringview_type& from, const stringview_type& to);
    template <size_t _Dim0, size_t _Dim1>
    bool gsub(const _Char(&from)[_Dim0], const _Char(&to)[_Dim1]) { return gsub(MakeStringView(from), MakeStringView(to)); }

    stringview_type MakeView() const {
        return (is_large_()
            ? stringview_type(_large.Storage, _large.Size)
            : stringview_type(_small.Buffer, _small.Size));
    }
    mutableview_type MutableView() {
        return (is_large_()
            ? mutableview_type(_large.Storage, _large.Size)
            : mutableview_type(_small.Buffer, _small.Size));
    }

    stringconvertion_type Converter() const {
        return stringconvertion_type{ MakeView() };
    }
    template <typename T>
    NODISCARD bool ConvertTo(T* dst) const {
        return Converter().template ConvertTo<T>(dst);
    }

    bool CheckInvariants() const;

    inline friend hash_t hash_value(const TBasicString& str) NOEXCEPT { return hash_string(str.MakeView()); }

private:
    bool is_large_() const { return (_large.IsLarge); }

    _Char* data_() { return (is_large_() ? _large.Storage : _small.Buffer); }
    const _Char* data_() const { return (is_large_() ? _large.Storage : _small.Buffer); }

    iterator MakeIterator_(size_t i) { return MakeCheckedIterator(data_(), size(), i); }
    const_iterator MakeConstIterator_(size_t i) const { return MakeCheckedIterator(data_(), size(), i); }

    // return a pointer to either small or large string with count + 1 characters available
    // also adds a null char ate of the storage
    TCheckedArrayIterator<_Char> resizeNoNullChar_(size_t count/* + 1 for null char */, bool change_size = true);
    TCheckedArrayIterator<_Char> resizeNoNullChar_Large_(size_t count/* + 1 for null char */, bool change_size);
    TCheckedArrayIterator<_Char> resizeNoNullChar_Small_(size_t count/* + 1 for null char */, bool change_size);
    TCheckedArrayIterator<_Char> resizeWNullChar_(size_t count/* + 1 for null char */);

    struct FLargeString_ {
        // use size_t to get more space for small strings on x64 :
        //          u32     size_t
        //  x86      12         12
        //  x64      16         24
        size_t IsLarge : 1;
        size_t Size : sizeof(size_t)*8 - 1;
        size_t Capacity;
        _Char* Storage;
    };
    STATIC_ASSERT(sizeof(FLargeString_) == sizeof(size_t) * 3);
    STATIC_ASSERT(Meta::IsAlignedPow2(sizeof(_Char), sizeof(FLargeString_)));

    struct FSmallString_ {
        STATIC_CONST_INTEGRAL(size_t, GCapacity, (sizeof(FLargeString_) - sizeof(_Char)) / sizeof(_Char));
        STATIC_CONST_INTEGRAL(size_t, GMaxSmallSize, GCapacity - 1);
        _Char IsLarge : 1;
        _Char Size : sizeof(_Char)*8 - 1;
        _Char Buffer[GCapacity];
    };
    STATIC_ASSERT(sizeof(FLargeString_) == sizeof(FSmallString_));

    union {
        FSmallString_ _small;
        FLargeString_ _large;
    };

public:
    STATIC_CONST_INTEGRAL(size_t, SmallCapacity, FSmallString_::GCapacity);
};
//----------------------------------------------------------------------------
template <typename _Char>
inline CONSTEXPR size_t BasicStringInSituSize = TBasicString<_Char>::SmallCapacity;
inline CONSTEXPR size_t StringInSituSize = BasicStringInSituSize<char>;
inline CONSTEXPR size_t WStringInSituSize = BasicStringInSituSize<wchar_t>;
//----------------------------------------------------------------------------
template <typename _Char>
template <typename _It, class>
inline void TBasicString<_Char>::assign(_It first, _It last) {
    clear();

    const size_t newSize = std::distance(first, last);
    auto dst = resizeWNullChar_(newSize);

    std::copy(first, last, dst);
}
//----------------------------------------------------------------------------
template <typename _Char>
template <typename _It, class>
inline void TBasicString<_Char>::append(_It first, _It last) {
    const size_t oldSize = size();
    const size_t newSize = (oldSize + std::distance(first, last));

    auto dst = resizeWNullChar_(newSize);
    std::copy(first, last, dst + oldSize);
}
//----------------------------------------------------------------------------
template <typename _Char>
template <typename _It, class>
inline void TBasicString<_Char>::insert(size_t pos, _It first, _It last) {
    const size_t oldSize = size();
    if (pos == npos) pos = oldSize;

    const size_t newSize = (oldSize + std::distance(first, last));
    auto dst = resizeWNullChar_(newSize);

    std::copy(first, last, dst + oldSize);
    std::rotate(dst + pos, dst + oldSize, dst + newSize);
}
//----------------------------------------------------------------------------
template <typename _Char>
template <typename _It, class>
inline void TBasicString<_Char>::replace(size_t pos, size_t len, _It first, _It last) {
    const size_t n = std::distance(first, last);
    const size_t oldSize = size();
    if (len == npos) len = (oldSize - pos);
    Assert(pos + len <= oldSize);

    const size_t newSize = (oldSize - len + n);
    auto dst = resizeNoNullChar_(newSize);

    if (newSize < oldSize)
        std::rotate(dst + (pos + n), dst + (pos + len), dst + oldSize);
    else if (newSize > oldSize)
        std::rotate(dst + (pos + len), dst + oldSize, dst + newSize);

    std::copy(first, last, dst + pos);
    *(dst + newSize) = _Char();
}
//----------------------------------------------------------------------------
template <typename _Char>
template <typename _Pred>
inline size_t TBasicString<_Char>::find_if(_Pred pred, size_t pos/* = 0 */) const noexcept {
    auto it = std::find_if(begin() + pos, end(), pred);
    return (it == end() ? npos : it - begin());
}
//----------------------------------------------------------------------------
template <typename _Char>
template <typename _Pred>
inline size_t TBasicString<_Char>::rfind_if(_Pred pred, size_t pos/* = npos */) const noexcept {
    auto it = std::find_if(pos == npos ? rbegin() : const_reverse_iterator(begin() + pos), rend(), pred);
    return (it == rend() ? npos : rend() - it - 1);
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Char2>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, const TBasicString<_Char2>& s) {
    return oss << s.MakeView(); // will be lazily converted to corresponding char type, :'( ?
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicStringView<_Char> Slice(const TBasicString<_Char>& str, size_t offset, size_t count) {
    return str.MakeView().SubRange(offset, count);
}
//----------------------------------------------------------------------------
template <typename _Char>
NODISCARD TMemoryView<_Char> MakeView(TBasicString<_Char>& s) NOEXCEPT {
    return s.MutableView();
}
//----------------------------------------------------------------------------
template <typename _Char>
NODISCARD TMemoryView<const _Char> MakeView(const TBasicString<_Char>& s) NOEXCEPT {
    return s.MakeView();
}
//----------------------------------------------------------------------------
template <typename _Char>
NODISCARD size_t SizeOf(const TBasicString<_Char>& s) NOEXCEPT {
    return s.size();
}
//----------------------------------------------------------------------------
template <typename _Char>
void Resize_DiscardData(TBasicString<_Char>& str, size_t size) {
    str.resize(size);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_CORE_API size_t ToCStr(char *dst, size_t capacity, const wchar_t *wcstr, size_t length);
PPE_CORE_API size_t ToCStr(char *dst, size_t capacity, const wchar_t *wcstr);
PPE_CORE_API size_t ToCStr(char *dst, size_t capacity, const FWString& wstr);
PPE_CORE_API FStringView ToCStr(const TMemoryView<char>& dst, const FWStringView& wstr);
//----------------------------------------------------------------------------
PPE_CORE_API size_t ToWCStr(wchar_t *dst, size_t capacity, const char *cstr, size_t length);
PPE_CORE_API size_t ToWCStr(wchar_t *dst, size_t capacity, const char *cstr);
PPE_CORE_API size_t ToWCStr(wchar_t *dst, size_t capacity, const FString& str);
PPE_CORE_API FWStringView ToWCStr(const TMemoryView<wchar_t>& dst, const FStringView& str);
//----------------------------------------------------------------------------
template <size_t _Dim>
size_t ToCStr(char(&dst)[_Dim], const wchar_t *wcstr) { return ToCStr(dst, _Dim, wcstr); }
//----------------------------------------------------------------------------
template <size_t _Dim>
size_t ToWCStr(wchar_t(&dst)[_Dim], const char *cstr) { return ToWCStr(dst, _Dim, cstr); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_CORE_API size_t WCHAR_to_CHAR(ECodePage codePage, char *dst, size_t capacity, const wchar_t *wcstr, size_t length);
PPE_CORE_API size_t WCHAR_to_CHAR(ECodePage codePage, char *dst, size_t capacity, const wchar_t *wcstr);
PPE_CORE_API size_t WCHAR_to_CHAR(ECodePage codePage, char *dst, size_t capacity, const FWString& wstr);
PPE_CORE_API FStringView WCHAR_to_CHAR(ECodePage codePage, const TMemoryView<char>& dst, const FWStringView& wstr);
//----------------------------------------------------------------------------
PPE_CORE_API size_t CHAR_to_WCHAR(ECodePage codePage, wchar_t *dst, size_t capacity, const char *cstr, size_t length);
PPE_CORE_API size_t CHAR_to_WCHAR(ECodePage codePage, wchar_t *dst, size_t capacity, const char *cstr);
PPE_CORE_API size_t CHAR_to_WCHAR(ECodePage codePage, wchar_t *dst, size_t capacity, const FString& str);
PPE_CORE_API FWStringView CHAR_to_WCHAR(ECodePage codePage, const TMemoryView<wchar_t>& dst, const FStringView& str);
//----------------------------------------------------------------------------
template <size_t _Dim>
size_t WCHAR_to_CHAR(ECodePage codePage, char(&dst)[_Dim], const wchar_t *wcstr) { return WCHAR_to_CHAR(codePage, dst, _Dim, wcstr); }
//----------------------------------------------------------------------------
template <size_t _Dim>
size_t CHAR_to_WCHAR(ECodePage codePage, wchar_t(&dst)[_Dim], const char *cstr) { return CHAR_to_WCHAR(codePage, dst, _Dim, cstr); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_CORE_API FString ToString(const wchar_t *wcstr, size_t length);
PPE_CORE_API FString ToString(const wchar_t *wcstr);
PPE_CORE_API FString ToString(const FWString& wstr);
inline const FString& ToString(const FString& str) { return str; }
PPE_CORE_API FString ToString(const TMemoryView<const char>& strview);
PPE_CORE_API FString ToString(const TMemoryView<const wchar_t>& strview);
//----------------------------------------------------------------------------
PPE_CORE_API FWString ToWString(const char *cstr, size_t length);
PPE_CORE_API FWString ToWString(const char *cstr);
PPE_CORE_API FWString ToWString(const FString& str);
inline const FWString& ToWString(const FWString& wstr) { return wstr; }
PPE_CORE_API FWString ToWString(const TMemoryView<const wchar_t>& strview);
PPE_CORE_API FWString ToWString(const TMemoryView<const char>& strview);
//----------------------------------------------------------------------------
template <typename _Char, size_t _Dim>
TBasicString<_Char> ToString(const _Char (&str)[_Dim]) {
    return ToString(MakeStringView(str));
}
//----------------------------------------------------------------------------
template <typename _Char>
FString ToString(const TBasicStringView<_Char>& str) {
    return ToString(str.MakeView());
}
//----------------------------------------------------------------------------
template <typename _Char>
FWString ToWString(const TBasicStringView<_Char>& str) {
    return ToWString(str.MakeView());
}
//----------------------------------------------------------------------------
inline bool operator >>(const FStringConversion& conv, FString* dst) {
    dst->assign(conv.Input);
    return (not dst->empty());
}
//----------------------------------------------------------------------------
inline bool operator >>(const FStringConversion& conv, FWString* dst) {
    dst->assign(ToWString(conv.Input));
    return (not dst->empty());
}
//----------------------------------------------------------------------------
inline bool operator >>(const FWStringConversion& conv, FWString* dst) {
    dst->assign(conv.Input);
    return (not dst->empty());
}
//----------------------------------------------------------------------------
inline bool operator >>(const FWStringConversion& conv, FString* dst) {
    dst->assign(ToString(conv.Input));
    return (not dst->empty());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, ECase _Sensitive>
struct TStringEqualTo : TStringViewEqualTo <_Char, _Sensitive> {
    bool operator ()(const TBasicString<_Char>& lhs, const TBasicString<_Char>& rhs) const {
        return TStringViewEqualTo<_Char, _Sensitive>::operator ()(lhs.MakeView(), rhs.MakeView());
    }
    bool operator ()(const TBasicString<_Char>& lhs, const TBasicStringView<_Char>& rhs) const {
        return TStringViewEqualTo<_Char, _Sensitive>::operator ()(lhs.MakeView(), rhs);
    }
};
//----------------------------------------------------------------------------
template <typename _Char, ECase _Sensitive>
struct TStringLess : TStringViewLess <_Char, _Sensitive> {
    bool operator ()(const TBasicString<_Char>& lhs, const TBasicString<_Char>& rhs) const {
        return TStringViewLess<_Char, _Sensitive>::operator ()(lhs.MakeView(), rhs.MakeView());
    }
    bool operator ()(const TBasicString<_Char>& lhs, const TBasicStringView<_Char>& rhs) const {
        return TStringViewLess<_Char, _Sensitive>::operator ()(lhs.MakeView(), rhs);
    }
};
//----------------------------------------------------------------------------
template <typename _Char, ECase _Sensitive>
struct TStringHasher : TStringViewHasher<_Char, _Sensitive> {
    size_t operator ()(const TBasicString<_Char>& str) const {
        return TStringViewHasher<_Char, _Sensitive>::operator ()(str.MakeView());
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#define USE_PPE_EXTERN_TEMPLATE_STRING 1

#if USE_PPE_EXTERN_TEMPLATE_STRING
#   ifndef EXPORT_PPE_RUNTIME_CORE_STRING
EXTERN_TEMPLATE_CLASS_DECL(PPE_CORE_API) PPE::TBasicString<char>;
EXTERN_TEMPLATE_CLASS_DECL(PPE_CORE_API) PPE::TBasicString<wchar_t>;
#   endif
#else
#   include "IO/String-inl.h"
#endif
