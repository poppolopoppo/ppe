#pragma once

#include "Core/Container/Token.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename _TokenTraits>
bool ValidateToken(const TBasicStringView<_Char>& content) {
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
template <typename _Tag, typename _Char, ECase _Sensitive, typename _TokenTraits, typename _Allocator >
TToken<_Tag, _Char, _Sensitive, _TokenTraits, _Allocator>::TToken(const _Char* cstr)
:   _data(factory_type::Instance().template GetOrCreate<_TokenTraits>(cstr)) {}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, ECase _Sensitive, typename _TokenTraits, typename _Allocator >
auto TToken<_Tag, _Char, _Sensitive, _TokenTraits, _Allocator>::operator =(const _Char* cstr) -> TToken& {
    _data = factory_type::Instance().template GetOrCreate<_TokenTraits>(cstr);
    return *this;
}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, ECase _Sensitive, typename _TokenTraits, typename _Allocator >
TToken<_Tag, _Char, _Sensitive, _TokenTraits, _Allocator>::TToken(const TBasicStringView<_Char>& content)
:   _data(factory_type::Instance().template GetOrCreate<_TokenTraits>(content)) {}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, ECase _Sensitive, typename _TokenTraits, typename _Allocator >
auto TToken<_Tag, _Char, _Sensitive, _TokenTraits, _Allocator>::operator =(const TBasicStringView<_Char>& content) -> TToken&  {
    _data = factory_type::Instance().template GetOrCreate<_TokenTraits>(content);
    return *this;
}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, ECase _Sensitive, typename _TokenTraits, typename _Allocator >
TToken<_Tag, _Char, _Sensitive, _TokenTraits, _Allocator>::TToken(const _Char* cstr, size_t length)
:   _data(factory_type::Instance().template GetOrCreate<_TokenTraits>(cstr, length)) {}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, ECase _Sensitive, typename _TokenTraits, typename _Allocator >
TToken<_Tag, _Char, _Sensitive, _TokenTraits, _Allocator>::TToken(const TToken& other)
:   _data(other._data) {}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, ECase _Sensitive, typename _TokenTraits, typename _Allocator >
auto TToken<_Tag, _Char, _Sensitive, _TokenTraits, _Allocator>::operator =(const TToken& other) -> TToken& {
    if (this != &other)
        _data = other._data;
    return *this;
}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, ECase _Sensitive, typename _TokenTraits, typename _Allocator >
size_t TToken<_Tag, _Char, _Sensitive, _TokenTraits, _Allocator>::HashValue() const {
    return hash_ptr(_data.Ptr);
}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, ECase _Sensitive, typename _TokenTraits, typename _Allocator >
void TToken<_Tag, _Char, _Sensitive, _TokenTraits, _Allocator>::Swap(TToken& other) {
    std::swap(other._data, _data);
}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, ECase _Sensitive, typename _TokenTraits, typename _Allocator >
bool TToken<_Tag, _Char, _Sensitive, _TokenTraits, _Allocator>::Equals(const TToken& other) const {
    return _data.Ptr == other._data.Ptr;
}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, ECase _Sensitive, typename _TokenTraits, typename _Allocator >
bool TToken<_Tag, _Char, _Sensitive, _TokenTraits, _Allocator>::Less(const TToken& other) const {
    return TTokenDataLess<_Char, _Sensitive>()(_data, other._data);
}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, ECase _Sensitive, typename _TokenTraits, typename _Allocator >
bool TToken<_Tag, _Char, _Sensitive, _TokenTraits, _Allocator>::Equals(const _Char *cstr) const {
    return TStringViewEqualTo<_Char, _Sensitive>()(_data.MakeView(), TBasicStringView<_Char>(cstr, Length(cstr)));
}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, ECase _Sensitive, typename _TokenTraits, typename _Allocator >
bool TToken<_Tag, _Char, _Sensitive, _TokenTraits, _Allocator>::Equals(const TBasicStringView<_Char>& slice) const {
    return TStringViewEqualTo<_Char, _Sensitive>()(_data.MakeView(), slice);
}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, ECase _Sensitive, typename _TokenTraits, typename _Allocator >
void TToken<_Tag, _Char, _Sensitive, _TokenTraits, _Allocator>::Start(size_t capacity) {
    factory_type::Create(capacity);
}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, ECase _Sensitive, typename _TokenTraits, typename _Allocator >
void TToken<_Tag, _Char, _Sensitive, _TokenTraits, _Allocator>::Clear() {
    factory_type::Instance().Clear();
}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, ECase _Sensitive, typename _TokenTraits, typename _Allocator >
void TToken<_Tag, _Char, _Sensitive, _TokenTraits, _Allocator>::Shutdown() {
    factory_type::Destroy();
}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, ECase _Sensitive, typename _TokenTraits, typename _Allocator >
auto TToken<_Tag, _Char, _Sensitive, _TokenTraits, _Allocator>::Factory() -> factory_type& {
    return factory_type::Instance();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename _Allocator>
TTokenAllocator<_Char, _Allocator>::TTokenAllocator()
:   _buckets(nullptr) {}
//----------------------------------------------------------------------------
template <typename _Char, typename _Allocator>
TTokenAllocator<_Char, _Allocator>::~TTokenAllocator() {
    Clear();
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Allocator>
auto TTokenAllocator<_Char, _Allocator>::Allocate(size_t count) -> _Char * {
    Assert(count);
    _Char* result = nullptr;

    FBucket* bucket = _buckets;
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
void TTokenAllocator<_Char, _Allocator>::Clear() {
    while (_buckets) {
        FBucket* const next = _buckets->Next;
        _Allocator::destroy(_buckets);
        _Allocator::deallocate(_buckets, 1);
        _buckets = next;
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, ECase _Sensitive, typename _Allocator>
TTokenSetSlot<_Char, _Sensitive, _Allocator>::TTokenSetSlot() {}
//----------------------------------------------------------------------------
template <typename _Char, ECase _Sensitive, typename _Allocator>
TTokenSetSlot<_Char, _Sensitive, _Allocator>::~TTokenSetSlot() {}
//----------------------------------------------------------------------------
template <typename _Char, ECase _Sensitive, typename _Allocator>
template <typename _TokenTraits>
auto TTokenSetSlot<_Char, _Sensitive, _Allocator>::GetOrCreate(const TBasicStringView<_Char>& content) -> TTokenData<_Char> {
    if (content.empty())
        return TTokenData<_Char>();

    TTokenData<_Char> result;
    {
        const std::unique_lock<std::mutex> scopeLock(_barrier);

        const auto it = _set.find(content);
        if (it == _set.end()) {
            const size_t length = content.size();

            _Char* newToken = parent_type::Allocate(
                  1/* one char to store length */
                + length
                + 1/* null terminated */ );

            newToken[0] = _Char(checked_cast<u8>(length)); // size of cstr is packed in cstr[-1]
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
template <typename _Char, ECase _Sensitive, typename _Allocator>
template <typename _TokenTraits>
auto TTokenSetSlot<_Char, _Sensitive, _Allocator>::GetOrCreate(const _Char* cstr, size_t length) -> TTokenData<_Char> {
    return GetOrCreate<_TokenTraits>(TBasicStringView<_Char>(cstr, length));
}
//----------------------------------------------------------------------------
template <typename _Char, ECase _Sensitive, typename _Allocator>
template <typename _TokenTraits>
auto TTokenSetSlot<_Char, _Sensitive, _Allocator>::GetOrCreate(const _Char* cstr) -> TTokenData<_Char> {
    return GetOrCreate<_TokenTraits>(cstr, Length(cstr));
}
//----------------------------------------------------------------------------
template <typename _Char, ECase _Sensitive, typename _Allocator>
template <typename _TokenTraits >
bool TTokenSetSlot<_Char, _Sensitive, _Allocator>::Validate(const TBasicStringView<_Char>& content) {
    return ValidateToken<_Char, _TokenTraits>(content);
}
//----------------------------------------------------------------------------
template <typename _Char, ECase _Sensitive, typename _Allocator>
void TTokenSetSlot<_Char, _Sensitive, _Allocator>::Clear() {
    // invalidates all tokens !
    const std::unique_lock<std::mutex> scopeLock(_barrier);
    _set.clear();
    parent_type::Clear();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, ECase _Sensitive, typename _Allocator>
TTokenSet<_Char, _Sensitive, _Allocator>::TTokenSet(size_t capacity) {
    for (token_set_slot_type& slot : _slots)
        slot.reserve(capacity/SlotCount);
}
//----------------------------------------------------------------------------
template <typename _Char, ECase _Sensitive, typename _Allocator>
TTokenSet<_Char, _Sensitive, _Allocator>::~TTokenSet() {}
//----------------------------------------------------------------------------
template <typename _Char, ECase _Sensitive, typename _Allocator>
size_t TTokenSet<_Char, _Sensitive, _Allocator>::size() const {
    size_t s = 0;
    for (const token_set_slot_type& slot : _slots)
        s += slot.size();
    return s;
}
//----------------------------------------------------------------------------
template <typename _Char, ECase _Sensitive, typename _Allocator>
template <typename _TokenTraits>
auto TTokenSet<_Char, _Sensitive, _Allocator>::GetOrCreate(const TBasicStringView<_Char>& content) -> TTokenData<_Char> {
    Assert(content.data());
    if (content.empty())
        return TTokenData<_Char>();

    const size_t h = SlotHash<_TokenTraits>(content);
    Assert(h < lengthof(_slots));

    token_set_slot_type& slot = _slots[h];
    return slot.template GetOrCreate<_TokenTraits>(content);
}
//----------------------------------------------------------------------------
template <typename _Char, ECase _Sensitive, typename _Allocator>
template <typename _TokenTraits>
auto TTokenSet<_Char, _Sensitive, _Allocator>::GetOrCreate(const _Char* cstr, size_t length) -> TTokenData<_Char> {
    Assert(cstr);
    return GetOrCreate<_TokenTraits>(TBasicStringView<_Char>(cstr, length));
}
//----------------------------------------------------------------------------
template <typename _Char, ECase _Sensitive, typename _Allocator>
template <typename _TokenTraits>
auto TTokenSet<_Char, _Sensitive, _Allocator>::GetOrCreate(const _Char* cstr) -> TTokenData<_Char> {
    Assert(cstr);
    return GetOrCreate<_TokenTraits>(cstr, Length(cstr));
}
//----------------------------------------------------------------------------
template <typename _Char, ECase _Sensitive, typename _Allocator>
void TTokenSet<_Char, _Sensitive, _Allocator>::Clear() {
    // invalidates all tokens !
    for (token_set_slot_type& slot : _slots)
        slot.Clear();
}
//----------------------------------------------------------------------------
namespace details {
template <typename _Char>
inline size_t TokenSlotHash(const TBasicStringView<_Char>& content, Meta::TIntegralConstant<ECase, ECase::Sensitive>) {
    return hash_fnv1a(content.begin(), content.end());
}
template <typename _Char>
inline size_t TokenSlotHash(const TBasicStringView<_Char>& content, Meta::TIntegralConstant<ECase, ECase::Insensitive>) {
    const auto lower = content.ToLower();
    return hash_fnv1a(lower.begin(), lower.end());
}
} //!details
template <typename _Char, ECase _Sensitive, typename _Allocator>
template <typename _TokenTraits>
size_t TTokenSet<_Char, _Sensitive, _Allocator>::SlotHash(const TBasicStringView<_Char>& content) {
    static_assert(Meta::IsPow2(SlotCount), "SlotCount must be a power of 2");
    return (details::TokenSlotHash(content, Meta::TIntegralConstant<ECase, _Sensitive>{}) & SlotMask);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
