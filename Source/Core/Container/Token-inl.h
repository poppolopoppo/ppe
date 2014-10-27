#pragma once

#include "Core/Container/Token.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename _TokenTraits>
bool ValidateToken(const BasicStringSlice<_Char>& content) {
    if (content.empty())
        return false;

    const _TokenTraits traits;
    for (const _Char *p = content.begin(); *p; ++p)
    if (!traits.IsAllowedChar(*p))
        return false;

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, CaseSensitive _CaseSensitive, typename _TokenTraits, typename _Allocator >
Token<_Tag, _Char, _CaseSensitive, _TokenTraits, _Allocator>::Token() {}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, CaseSensitive _CaseSensitive, typename _TokenTraits, typename _Allocator >
Token<_Tag, _Char, _CaseSensitive, _TokenTraits, _Allocator>::~Token() {}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, CaseSensitive _CaseSensitive, typename _TokenTraits, typename _Allocator >
Token<_Tag, _Char, _CaseSensitive, _TokenTraits, _Allocator>::Token(const _Char* cstr)
:   _data(factory_type::Instance().GetOrCreate<_TokenTraits>(cstr)) {}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, CaseSensitive _CaseSensitive, typename _TokenTraits, typename _Allocator >
auto Token<_Tag, _Char, _CaseSensitive, _TokenTraits, _Allocator>::operator =(const _Char* cstr) -> Token& {
    _data = factory_type::Instance().GetOrCreate<_TokenTraits>(cstr);
    return *this;
}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, CaseSensitive _CaseSensitive, typename _TokenTraits, typename _Allocator >
Token<_Tag, _Char, _CaseSensitive, _TokenTraits, _Allocator>::Token(const BasicStringSlice<_Char>& content)
:   _data(factory_type::Instance().GetOrCreate<_TokenTraits>(content)) {}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, CaseSensitive _CaseSensitive, typename _TokenTraits, typename _Allocator >
Token<_Tag, _Char, _CaseSensitive, _TokenTraits, _Allocator>::Token(const _Char* cstr, size_t length)
:   _data(factory_type::Instance().GetOrCreate<_TokenTraits>(cstr, length)) {}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, CaseSensitive _CaseSensitive, typename _TokenTraits, typename _Allocator >
Token<_Tag, _Char, _CaseSensitive, _TokenTraits, _Allocator>::Token(const Token& other)
:   _data(other._data) {}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, CaseSensitive _CaseSensitive, typename _TokenTraits, typename _Allocator >
auto Token<_Tag, _Char, _CaseSensitive, _TokenTraits, _Allocator>::operator =(const Token& other) -> Token& {
    _data = other._data;
    return *this;
}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, CaseSensitive _CaseSensitive, typename _TokenTraits, typename _Allocator >
size_t Token<_Tag, _Char, _CaseSensitive, _TokenTraits, _Allocator>::HashValue() const {
    size_t h = size_t(_data.Ptr);
#ifdef ARCH_X64
    static_assert(sizeof(size_t) == 8, "This code is for 64-bit size_t.");
    h ^= h >> 32;
#else
    static_assert(sizeof(size_t) == 4, "This code is for 32-bit size_t.");
#endif
    return h;
}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, CaseSensitive _CaseSensitive, typename _TokenTraits, typename _Allocator >
void Token<_Tag, _Char, _CaseSensitive, _TokenTraits, _Allocator>::Swap(Token& other) {
    std::swap(other._data, _data);
}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, CaseSensitive _CaseSensitive, typename _TokenTraits, typename _Allocator >
bool Token<_Tag, _Char, _CaseSensitive, _TokenTraits, _Allocator>::Equals(const Token& other) const {
    return _data.Ptr == other._data.Ptr;
}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, CaseSensitive _CaseSensitive, typename _TokenTraits, typename _Allocator >
bool Token<_Tag, _Char, _CaseSensitive, _TokenTraits, _Allocator>::Less(const Token& other) const {
    return TokenDataLess<_Char, _CaseSensitive>()(_data, other._data);
}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, CaseSensitive _CaseSensitive, typename _TokenTraits, typename _Allocator >
bool Token<_Tag, _Char, _CaseSensitive, _TokenTraits, _Allocator>::Equals(const _Char *cstr) const {
    return StringSliceEqualTo<_Char, _CaseSensitive>()(_data.MakeView(), BasicStringSlice<_Char>(cstr, Length(cstr)));
}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, CaseSensitive _CaseSensitive, typename _TokenTraits, typename _Allocator >
bool Token<_Tag, _Char, _CaseSensitive, _TokenTraits, _Allocator>::Equals(const BasicStringSlice<_Char>& slice) const {
    return StringSliceEqualTo<_Char, _CaseSensitive>()(_data.MakeView(), slice);
}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, CaseSensitive _CaseSensitive, typename _TokenTraits, typename _Allocator >
void Token<_Tag, _Char, _CaseSensitive, _TokenTraits, _Allocator>::Start(size_t capacity) {
    factory_type::Create(capacity);
}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, CaseSensitive _CaseSensitive, typename _TokenTraits, typename _Allocator >
void Token<_Tag, _Char, _CaseSensitive, _TokenTraits, _Allocator>::Clear() {
    factory_type::Instance().Clear();
}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, CaseSensitive _CaseSensitive, typename _TokenTraits, typename _Allocator >
void Token<_Tag, _Char, _CaseSensitive, _TokenTraits, _Allocator>::Shutdown() {
    factory_type::Destroy();
}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, CaseSensitive _CaseSensitive, typename _TokenTraits, typename _Allocator >
auto Token<_Tag, _Char, _CaseSensitive, _TokenTraits, _Allocator>::Factory() -> factory_type& {
    return factory_type::Instance();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename _Allocator>
TokenAllocator<_Char, _Allocator>::TokenAllocator()
:   _buckets(nullptr) {}
//----------------------------------------------------------------------------
template <typename _Char, typename _Allocator>
TokenAllocator<_Char, _Allocator>::~TokenAllocator() {
    Clear();
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Allocator>
auto TokenAllocator<_Char, _Allocator>::Allocate(size_t count) -> _Char * {
    Assert(count);
    _Char* result = nullptr;

    Bucket* bucket = _buckets;
    while (bucket) {
        if ((result = _buckets->Stack.Allocate(count)))
            return result;
        bucket = bucket->Next;
    }

    bucket = _Allocator::allocate(1);
    _Allocator::construct(bucket);

    bucket->Next = _buckets;
    _buckets = bucket;

    result = bucket->Stack.Allocate(count);
    Assert(result);

    return result;
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Allocator>
void TokenAllocator<_Char, _Allocator>::Clear() {
    while (_buckets) {
        Bucket* const next = _buckets->Next;
        _Allocator::destroy(_buckets);
        _Allocator::deallocate(_buckets, 1);
        _buckets = next;
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, CaseSensitive _CaseSensitive, typename _Allocator>
TokenSetSlot<_Char, _CaseSensitive, _Allocator>::TokenSetSlot() {}
//----------------------------------------------------------------------------
template <typename _Char, CaseSensitive _CaseSensitive, typename _Allocator>
TokenSetSlot<_Char, _CaseSensitive, _Allocator>::~TokenSetSlot() {}
//----------------------------------------------------------------------------
template <typename _Char, CaseSensitive _CaseSensitive, typename _Allocator>
template <typename _TokenTraits>
auto TokenSetSlot<_Char, _CaseSensitive, _Allocator>::GetOrCreate(const BasicStringSlice<_Char>& content) -> TokenData<_Char> {
    if (content.empty())
        return TokenData<_Char>();

    TokenData<_Char> result;
    {
        std::lock_guard<std::mutex> scopeLock(_barrier);

        const auto it = _set.find(content);
        if (it == _set.end()) {
            const size_t length = content.size();

            _Char* newToken = parent_type::Allocate(length + 2);
            newToken[0] = checked_cast<u8>(length); // size of cstr is packed in cstr[-1]
            memcpy(newToken + 1, content.begin(), length * sizeof(_Char));
            newToken[length + 1] = _Char(0);

            result.Ptr = &newToken[1];

            _set.insert(result.MakeView());
        }
        else {
            result.Ptr = it->begin();
        }
        Assert(result.size() == content.size());
    }
    Assert(Validate<_TokenTraits>(result.MakeView()) );
    return result;
}
//----------------------------------------------------------------------------
template <typename _Char, CaseSensitive _CaseSensitive, typename _Allocator>
template <typename _TokenTraits>
auto TokenSetSlot<_Char, _CaseSensitive, _Allocator>::GetOrCreate(const _Char* cstr, size_t length) -> TokenData<_Char> {
    return GetOrCreate<_TokenTraits>(BasicStringSlice<_Char>(cstr, length));
}
//----------------------------------------------------------------------------
template <typename _Char, CaseSensitive _CaseSensitive, typename _Allocator>
template <typename _TokenTraits>
auto TokenSetSlot<_Char, _CaseSensitive, _Allocator>::GetOrCreate(const _Char* cstr) -> TokenData<_Char> {
    return GetOrCreate<_TokenTraits>(cstr, Length(cstr));
}
//----------------------------------------------------------------------------
template <typename _Char, CaseSensitive _CaseSensitive, typename _Allocator>
template <typename _TokenTraits >
bool TokenSetSlot<_Char, _CaseSensitive, _Allocator>::Validate(const BasicStringSlice<_Char>& content) {
    return ValidateToken<_Char, _TokenTraits>(content);
}
//----------------------------------------------------------------------------
template <typename _Char, CaseSensitive _CaseSensitive, typename _Allocator>
void TokenSetSlot<_Char, _CaseSensitive, _Allocator>::Clear() {
    // invalidates all tokens !
    std::lock_guard<std::mutex> scopeLock(_barrier);
    _set.clear();
    parent_type::Clear();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, CaseSensitive _CaseSensitive, typename _Allocator>
TokenSet<_Char, _CaseSensitive, _Allocator>::TokenSet(size_t capacity) {
    for (token_set_slot_type& slot : _slots)
        slot.reserve(capacity/SlotCount);
}
//----------------------------------------------------------------------------
template <typename _Char, CaseSensitive _CaseSensitive, typename _Allocator>
TokenSet<_Char, _CaseSensitive, _Allocator>::~TokenSet() {}
//----------------------------------------------------------------------------
template <typename _Char, CaseSensitive _CaseSensitive, typename _Allocator>
size_t TokenSet<_Char, _CaseSensitive, _Allocator>::size() const {
    size_t s = 0;
    for (const token_set_slot_type& slot : _slots)
        s += slot.size();
    return s;
}
//----------------------------------------------------------------------------
template <typename _Char, CaseSensitive _CaseSensitive, typename _Allocator>
template <typename _TokenTraits>
auto TokenSet<_Char, _CaseSensitive, _Allocator>::GetOrCreate(const BasicStringSlice<_Char>& content) -> TokenData<_Char> {
    Assert(content.begin());
    if (content.empty())
        return TokenData<_Char>();

    const size_t h = SlotHash<_TokenTraits>(content);
    Assert(h < lengthof(_slots));

    token_set_slot_type& slot = _slots[h];
    return slot.GetOrCreate<_TokenTraits>(content);
}
//----------------------------------------------------------------------------
template <typename _Char, CaseSensitive _CaseSensitive, typename _Allocator>
template <typename _TokenTraits>
auto TokenSet<_Char, _CaseSensitive, _Allocator>::GetOrCreate(const _Char* cstr, size_t length) -> TokenData<_Char> {
    Assert(cstr);
    return GetOrCreate<_TokenTraits>(BasicStringSlice<_Char>(cstr, length));
}
//----------------------------------------------------------------------------
template <typename _Char, CaseSensitive _CaseSensitive, typename _Allocator>
template <typename _TokenTraits>
auto TokenSet<_Char, _CaseSensitive, _Allocator>::GetOrCreate(const _Char* cstr) -> TokenData<_Char> {
    Assert(cstr);
    return GetOrCreate<_TokenTraits>(cstr, Length(cstr));
}
//----------------------------------------------------------------------------
template <typename _Char, CaseSensitive _CaseSensitive, typename _Allocator>
void TokenSet<_Char, _CaseSensitive, _Allocator>::Clear() {
    // invalidates all tokens !
    for (token_set_slot_type& slot : _slots)
        slot.Clear();
}
//----------------------------------------------------------------------------
template <typename _Char, CaseSensitive _CaseSensitive, typename _Allocator>
template <typename _TokenTraits>
size_t TokenSet<_Char, _CaseSensitive, _Allocator>::SlotHash(const BasicStringSlice<_Char>& content) {
    static_assert(IS_POW2(SlotCount), "SlotCount must be a power of 2");
    size_t h = 0;
    if (CaseSensitive::True == _CaseSensitive) {
        for (const _Char ch : content)
            h += size_t(ch);
    }
    else {
        const std::locale& locale = _TokenTraits().Locale();
        for (const _Char ch : content)
            h += size_t(std::tolower(ch, locale));
    }
    return (h & SlotMask);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
