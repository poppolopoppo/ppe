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

    const _TokenTraits traits = {};
    for (const _Char& ch : content)
        if (!traits.IsAllowedChar(ch))
            return false;

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, Case _Sensitive, typename _TokenTraits, typename _Allocator >
Token<_Tag, _Char, _Sensitive, _TokenTraits, _Allocator>::Token() : _data{nullptr} {}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, Case _Sensitive, typename _TokenTraits, typename _Allocator >
Token<_Tag, _Char, _Sensitive, _TokenTraits, _Allocator>::Token(const _Char* cstr)
:   _data(factory_type::Instance().template GetOrCreate<_TokenTraits>(cstr)) {}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, Case _Sensitive, typename _TokenTraits, typename _Allocator >
auto Token<_Tag, _Char, _Sensitive, _TokenTraits, _Allocator>::operator =(const _Char* cstr) -> Token& {
    _data = factory_type::Instance().template GetOrCreate<_TokenTraits>(cstr);
    return *this;
}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, Case _Sensitive, typename _TokenTraits, typename _Allocator >
Token<_Tag, _Char, _Sensitive, _TokenTraits, _Allocator>::Token(const BasicStringSlice<_Char>& content)
:   _data(factory_type::Instance().template GetOrCreate<_TokenTraits>(content)) {}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, Case _Sensitive, typename _TokenTraits, typename _Allocator >
Token<_Tag, _Char, _Sensitive, _TokenTraits, _Allocator>::Token(const _Char* cstr, size_t length)
:   _data(factory_type::Instance().template GetOrCreate<_TokenTraits>(cstr, length)) {}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, Case _Sensitive, typename _TokenTraits, typename _Allocator >
Token<_Tag, _Char, _Sensitive, _TokenTraits, _Allocator>::Token(const Token& other)
:   _data(other._data) {}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, Case _Sensitive, typename _TokenTraits, typename _Allocator >
auto Token<_Tag, _Char, _Sensitive, _TokenTraits, _Allocator>::operator =(const Token& other) -> Token& {
    if (this != &other)
        _data = other._data;
    return *this;
}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, Case _Sensitive, typename _TokenTraits, typename _Allocator >
size_t Token<_Tag, _Char, _Sensitive, _TokenTraits, _Allocator>::HashValue() const {
    return hash_as_pod(intptr_t(_data.Ptr));
}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, Case _Sensitive, typename _TokenTraits, typename _Allocator >
void Token<_Tag, _Char, _Sensitive, _TokenTraits, _Allocator>::Swap(Token& other) {
    std::swap(other._data, _data);
}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, Case _Sensitive, typename _TokenTraits, typename _Allocator >
bool Token<_Tag, _Char, _Sensitive, _TokenTraits, _Allocator>::Equals(const Token& other) const {
    return _data.Ptr == other._data.Ptr;
}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, Case _Sensitive, typename _TokenTraits, typename _Allocator >
bool Token<_Tag, _Char, _Sensitive, _TokenTraits, _Allocator>::Less(const Token& other) const {
    return TokenDataLess<_Char, _Sensitive>()(_data, other._data);
}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, Case _Sensitive, typename _TokenTraits, typename _Allocator >
bool Token<_Tag, _Char, _Sensitive, _TokenTraits, _Allocator>::Equals(const _Char *cstr) const {
    return StringSliceEqualTo<_Char, _Sensitive>()(_data.MakeView(), BasicStringSlice<_Char>(cstr, Length(cstr)));
}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, Case _Sensitive, typename _TokenTraits, typename _Allocator >
bool Token<_Tag, _Char, _Sensitive, _TokenTraits, _Allocator>::Equals(const BasicStringSlice<_Char>& slice) const {
    return StringSliceEqualTo<_Char, _Sensitive>()(_data.MakeView(), slice);
}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, Case _Sensitive, typename _TokenTraits, typename _Allocator >
void Token<_Tag, _Char, _Sensitive, _TokenTraits, _Allocator>::Start(size_t capacity) {
    factory_type::Create(capacity);
}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, Case _Sensitive, typename _TokenTraits, typename _Allocator >
void Token<_Tag, _Char, _Sensitive, _TokenTraits, _Allocator>::Clear() {
    factory_type::Instance().Clear();
}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, Case _Sensitive, typename _TokenTraits, typename _Allocator >
void Token<_Tag, _Char, _Sensitive, _TokenTraits, _Allocator>::Shutdown() {
    factory_type::Destroy();
}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, Case _Sensitive, typename _TokenTraits, typename _Allocator >
auto Token<_Tag, _Char, _Sensitive, _TokenTraits, _Allocator>::Factory() -> factory_type& {
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
        if (nullptr != (result = _buckets->Stack.Allocate(count)) )
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
template <typename _Char, Case _Sensitive, typename _Allocator>
TokenSetSlot<_Char, _Sensitive, _Allocator>::TokenSetSlot() {}
//----------------------------------------------------------------------------
template <typename _Char, Case _Sensitive, typename _Allocator>
TokenSetSlot<_Char, _Sensitive, _Allocator>::~TokenSetSlot() {}
//----------------------------------------------------------------------------
template <typename _Char, Case _Sensitive, typename _Allocator>
template <typename _TokenTraits>
auto TokenSetSlot<_Char, _Sensitive, _Allocator>::GetOrCreate(const BasicStringSlice<_Char>& content) -> TokenData<_Char> {
    if (content.empty())
        return TokenData<_Char>();

    TokenData<_Char> result;
    {
        std::lock_guard<std::mutex> scopeLock(_barrier);

        const auto it = _set.find(content);
        if (it == _set.end()) {
            const size_t length = content.size();

            _Char* newToken = parent_type::Allocate(
                  1/* one char to store length */
                + length
                + 1/* null terminated */ );

            newToken[0] = checked_cast<_Char>(length); // size of cstr is packed in cstr[-1]
            memcpy(&newToken[1], content.Pointer(), sizeof(_Char)*(length + 1));
            newToken[1 + length] = _Char(0);

            result.Ptr = &newToken[1];
            Assert(Length(result.Ptr) == length);

            _set.insert(result.MakeView());
        }
        else {
            result.Ptr = it->data();
        }
        Assert(result.size() == content.size());
    }
    Assert(Validate<_TokenTraits>(result.MakeView()) );
    return result;
}
//----------------------------------------------------------------------------
template <typename _Char, Case _Sensitive, typename _Allocator>
template <typename _TokenTraits>
auto TokenSetSlot<_Char, _Sensitive, _Allocator>::GetOrCreate(const _Char* cstr, size_t length) -> TokenData<_Char> {
    return GetOrCreate<_TokenTraits>(BasicStringSlice<_Char>(cstr, length));
}
//----------------------------------------------------------------------------
template <typename _Char, Case _Sensitive, typename _Allocator>
template <typename _TokenTraits>
auto TokenSetSlot<_Char, _Sensitive, _Allocator>::GetOrCreate(const _Char* cstr) -> TokenData<_Char> {
    return GetOrCreate<_TokenTraits>(cstr, Length(cstr));
}
//----------------------------------------------------------------------------
template <typename _Char, Case _Sensitive, typename _Allocator>
template <typename _TokenTraits >
bool TokenSetSlot<_Char, _Sensitive, _Allocator>::Validate(const BasicStringSlice<_Char>& content) {
    return ValidateToken<_Char, _TokenTraits>(content);
}
//----------------------------------------------------------------------------
template <typename _Char, Case _Sensitive, typename _Allocator>
void TokenSetSlot<_Char, _Sensitive, _Allocator>::Clear() {
    // invalidates all tokens !
    std::lock_guard<std::mutex> scopeLock(_barrier);
    _set.clear();
    parent_type::Clear();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, Case _Sensitive, typename _Allocator>
TokenSet<_Char, _Sensitive, _Allocator>::TokenSet(size_t capacity) {
    for (token_set_slot_type& slot : _slots)
        slot.reserve(capacity/SlotCount);
}
//----------------------------------------------------------------------------
template <typename _Char, Case _Sensitive, typename _Allocator>
TokenSet<_Char, _Sensitive, _Allocator>::~TokenSet() {}
//----------------------------------------------------------------------------
template <typename _Char, Case _Sensitive, typename _Allocator>
size_t TokenSet<_Char, _Sensitive, _Allocator>::size() const {
    size_t s = 0;
    for (const token_set_slot_type& slot : _slots)
        s += slot.size();
    return s;
}
//----------------------------------------------------------------------------
template <typename _Char, Case _Sensitive, typename _Allocator>
template <typename _TokenTraits>
auto TokenSet<_Char, _Sensitive, _Allocator>::GetOrCreate(const BasicStringSlice<_Char>& content) -> TokenData<_Char> {
    Assert(content.data());
    if (content.empty())
        return TokenData<_Char>();

    const size_t h = SlotHash<_TokenTraits>(content);
    Assert(h < lengthof(_slots));

    token_set_slot_type& slot = _slots[h];
    return slot.template GetOrCreate<_TokenTraits>(content);
}
//----------------------------------------------------------------------------
template <typename _Char, Case _Sensitive, typename _Allocator>
template <typename _TokenTraits>
auto TokenSet<_Char, _Sensitive, _Allocator>::GetOrCreate(const _Char* cstr, size_t length) -> TokenData<_Char> {
    Assert(cstr);
    return GetOrCreate<_TokenTraits>(BasicStringSlice<_Char>(cstr, length));
}
//----------------------------------------------------------------------------
template <typename _Char, Case _Sensitive, typename _Allocator>
template <typename _TokenTraits>
auto TokenSet<_Char, _Sensitive, _Allocator>::GetOrCreate(const _Char* cstr) -> TokenData<_Char> {
    Assert(cstr);
    return GetOrCreate<_TokenTraits>(cstr, Length(cstr));
}
//----------------------------------------------------------------------------
template <typename _Char, Case _Sensitive, typename _Allocator>
void TokenSet<_Char, _Sensitive, _Allocator>::Clear() {
    // invalidates all tokens !
    for (token_set_slot_type& slot : _slots)
        slot.Clear();
}
//----------------------------------------------------------------------------
#pragma warning( push )
#pragma warning( disable : 4127) // C4127: l'expression conditionnelle est une constante
template <typename _Char, Case _Sensitive, typename _Allocator>
template <typename _TokenTraits>
size_t TokenSet<_Char, _Sensitive, _Allocator>::SlotHash(const BasicStringSlice<_Char>& content) {
    static_assert(IS_POW2(SlotCount), "SlotCount must be a power of 2");
    size_t h = 0;
    if (Case::Sensitive == _Sensitive) {
        for (const _Char ch : content)
            h += size_t(ch);
    }
    else {
        for (const _Char ch : content)
            h += size_t(ToLower(ch));
    }
    return (h & SlotMask);
}
#pragma warning( pop )
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
