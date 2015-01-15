#pragma once

#include "Core/Core.h"

#include "Core/Allocator/Allocation.h"
#include "Core/Container/HashSet.h"
#include "Core/IO/StringSlice.h"
#include "Core/Memory/MemoryStack.h"

#include <iosfwd>
#include <mutex>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char>
class TokenTraits {
public:
    const std::locale& Locale() const {  return std::locale::classic(); }
    bool IsAllowedChar(_Char ch) const { return std::isgraph(ch, Locale()); }
};
//----------------------------------------------------------------------------
template <typename _Char, typename _TokenTraits = TokenTraits<_Char> >
bool ValidateToken(const BasicStringSlice<_Char>& content);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char>
struct TokenData {
    const _Char *Ptr;

    TokenData() : Ptr(nullptr) {}

    const _Char *cstr() const { return Ptr; }
    size_t size() const { return Ptr ? Ptr[-1] : 0; }
    bool empty() const { return Ptr == nullptr; }

    BasicStringSlice<_Char> MakeView() const {
        return BasicStringSlice<_Char>(Ptr, Ptr[-1]);
    }
};
//----------------------------------------------------------------------------
template <typename _Char, CaseSensitive _CaseSensitive>
struct TokenDataEqualTo : StringSliceEqualTo<_Char, _CaseSensitive> {
    bool operator ()(const TokenData<_Char>& lhs, const TokenData<_Char>& rhs) const {
        return StringSliceEqualTo<_Char, _CaseSensitive>::operator ()(lhs.MakeView(), rhs.MakeView());
    }
};
//----------------------------------------------------------------------------
template <typename _Char, CaseSensitive _CaseSensitive>
struct TokenDataLess : StringSliceLess<_Char, _CaseSensitive> {
    bool operator ()(const TokenData<_Char>& lhs, const TokenData<_Char>& rhs) const {
        return StringSliceLess<_Char, _CaseSensitive>::operator ()(lhs.MakeView(), rhs.MakeView());
    }
};
//----------------------------------------------------------------------------
template <typename _Char, CaseSensitive _CaseSensitive>
struct TokenDataHasher : StringSliceHasher<_Char, _CaseSensitive> {
    size_t operator ()(const TokenData<_Char>& data) const {
        return StringSliceHasher<_Char, _CaseSensitive>::operator ()(data.MakeView());
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Token>
class TokenFactory;
//----------------------------------------------------------------------------
template <
    typename        _Tag,
    typename        _Char,
    CaseSensitive   _CaseSensitive,
    typename        _TokenTraits = TokenTraits<_Char>,
    typename        _Allocator = ALLOCATOR(Token, _Char)
>
class Token {
public:
    template <
        typename        _Tag2,
        typename        _Char2,
        CaseSensitive   _CaseSensitive2,
        typename        _TokenTraits2,
        typename        _Allocator2
    >   friend class Token;

    typedef _Tag tag_type;
    typedef _Char char_type;
    typedef _TokenTraits token_traits;
    typedef _Allocator allocator_type;
    typedef TokenFactory<Token> factory_type;

    enum { Sensitiveness = _CaseSensitive };

    Token();
    ~Token();

    Token(const _Char* content);
    Token& operator =(const _Char* content);

    Token(const BasicStringSlice<_Char>& content);
    Token(const _Char* content, size_t length);

    template <typename _CharTraits, typename _Allocator>
    Token(const std::basic_string<_Char, _CharTraits, _Allocator>& content)
        : Token(content.c_str(), content.size()) {}

    Token(const Token& token);
    Token& operator =(const Token& token);

    size_t size() const { return _data.size(); }
    bool empty() const { return _data.empty(); }

    const _Char* cstr() const { return _data.cstr(); }
    BasicStringSlice<_Char> MakeView() const { return _data.MakeView(); }

    size_t HashValue() const;

    void Swap(Token& other);

    bool Equals(const Token& other) const;
    bool Less(const Token& other) const;

    bool Equals(const _Char *cstr) const;
    bool Equals(const BasicStringSlice<_Char>& slice) const;

    template <size_t _Dim>
    bool Equals(const _Char (&array)[_Dim]) const {
        return Equals(MakeStringSlice(array));
    }

    static void Start(size_t capacity);
    static void Clear();
    static void Shutdown();

    static factory_type& Factory();

    class Startup {
    public:
        Startup(int capacity) { Start(capacity); }
        ~Startup() { Shutdown(); }
    };

private:
    TokenData<_Char> _data;
};
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, CaseSensitive _CaseSensitive, typename _TokenTraits, typename _Allocator >
bool operator ==(   const Token<_Tag, _Char, _CaseSensitive, _TokenTraits, _Allocator>& lhs,
                    const Token<_Tag, _Char, _CaseSensitive, _TokenTraits, _Allocator>& rhs) {
    return lhs.Equals(rhs);
}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, CaseSensitive _CaseSensitive, typename _TokenTraits, typename _Allocator >
bool operator !=(   const Token<_Tag, _Char, _CaseSensitive, _TokenTraits, _Allocator>& lhs,
                    const Token<_Tag, _Char, _CaseSensitive, _TokenTraits, _Allocator>& rhs) {
    return !operator ==(lhs, rhs);
}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, CaseSensitive _CaseSensitive, typename _TokenTraits, typename _Allocator >
bool operator <(const Token<_Tag, _Char, _CaseSensitive, _TokenTraits, _Allocator>& lhs,
                const Token<_Tag, _Char, _CaseSensitive, _TokenTraits, _Allocator>& rhs) {
    return lhs.Less(rhs);
}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, CaseSensitive _CaseSensitive, typename _TokenTraits, typename _Allocator >
bool operator >=(   const Token<_Tag, _Char, _CaseSensitive, _TokenTraits, _Allocator>& lhs,
                    const Token<_Tag, _Char, _CaseSensitive, _TokenTraits, _Allocator>& rhs) {
    return !operator <(lhs, rhs);
}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, CaseSensitive _CaseSensitive, typename _TokenTraits, typename _Allocator >
bool operator ==(   const Token<_Tag, _Char, _CaseSensitive, _TokenTraits, _Allocator>& lhs,
                    const _Char *rhs) {
    return lhs.Equals(rhs);
}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, CaseSensitive _CaseSensitive, typename _TokenTraits, typename _Allocator >
bool operator !=(   const Token<_Tag, _Char, _CaseSensitive, _TokenTraits, _Allocator>& lhs,
                    const _Char *rhs) {
    return !operator ==(lhs, rhs);
}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, CaseSensitive _CaseSensitive, typename _TokenTraits, typename _Allocator >
bool operator ==(   const _Char *lhs,
                    const Token<_Tag, _Char, _CaseSensitive, _TokenTraits, _Allocator>& rhs ) {
    return rhs.Equals(lhs);
}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, CaseSensitive _CaseSensitive, typename _TokenTraits, typename _Allocator >
bool operator !=(   const _Char *lhs,
                    const Token<_Tag, _Char, _CaseSensitive, _TokenTraits, _Allocator>& rhs) {
    return !operator ==(lhs, rhs);
}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, CaseSensitive _CaseSensitive, typename _TokenTraits, typename _Allocator >
bool operator ==(   const Token<_Tag, _Char, _CaseSensitive, _TokenTraits, _Allocator>& lhs,
                    const BasicStringSlice<_Char>& rhs) {
    return lhs.Equals(rhs);
}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, CaseSensitive _CaseSensitive, typename _TokenTraits, typename _Allocator >
bool operator !=(   const Token<_Tag, _Char, _CaseSensitive, _TokenTraits, _Allocator>& lhs,
                    const BasicStringSlice<_Char>& rhs) {
    return !operator ==(lhs, rhs);
}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, CaseSensitive _CaseSensitive, typename _TokenTraits, typename _Allocator >
bool operator ==(   const BasicStringSlice<_Char>& lhs,
                    const Token<_Tag, _Char, _CaseSensitive, _TokenTraits, _Allocator>& rhs ) {
    return rhs.Equals(lhs);
}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, CaseSensitive _CaseSensitive, typename _TokenTraits, typename _Allocator >
bool operator !=(   const BasicStringSlice<_Char>& lhs,
                    const Token<_Tag, _Char, _CaseSensitive, _TokenTraits, _Allocator>& rhs) {
    return !operator ==(lhs, rhs);
}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, CaseSensitive _CaseSensitive, typename _TokenTraits, typename _Allocator >
size_t hash_value(const Token<_Tag, _Char, _CaseSensitive, _TokenTraits, _Allocator>& token) {
    return token.HashValue();
}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, CaseSensitive _CaseSensitive, typename _TokenTraits, typename _Allocator >
void swap(Token<_Tag, _Char, _CaseSensitive, _TokenTraits, _Allocator>& lhs,
    Token<_Tag, _Char, _CaseSensitive, _TokenTraits, _Allocator>& rhs) {
    lhs.Swap(rhs);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define CORE_TOKEN_BUCKET_SIZE (4 * PAGE_SIZE)
//----------------------------------------------------------------------------
template <typename _Char>
struct TokenBucket {
    TokenBucket* Next;
    enum { Size = (CORE_TOKEN_BUCKET_SIZE - sizeof(TokenBucket*)-sizeof(MemoryStack<_Char>)) / sizeof(_Char) };
    StaticStack<_Char, Size> Stack;
};
//----------------------------------------------------------------------------
template <typename _Char, typename _Allocator = ALLOCATOR(Token, TokenBucket<_Char>) >
class TokenAllocator : public _Allocator {
public:
    typedef TokenBucket<_Char> Bucket;
    static_assert(sizeof(Bucket) == CORE_TOKEN_BUCKET_SIZE, "invalid Bucket size");

    TokenAllocator();
    ~TokenAllocator();

    TokenAllocator(const TokenAllocator&) = delete;
    TokenAllocator& operator =(const TokenAllocator&) = delete;

    _Char* Allocate(size_t count);
    void Clear();

private:
    Bucket* _buckets;
};
//----------------------------------------------------------------------------
#undef CORE_TOKEN_BUCKET_SIZE
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, CaseSensitive _CaseSensitive, typename _Allocator = ALLOCATOR(Token, TokenBucket<_Char>) >
class TokenSetSlot : TokenAllocator<_Char, typename _Allocator::template rebind< TokenBucket<_Char> >::other> {
public:
    typedef TokenAllocator<_Char, typename _Allocator::template rebind< TokenBucket<_Char> >::other > parent_type;

    explicit TokenSetSlot();
    ~TokenSetSlot();

    TokenSetSlot(const TokenSetSlot&) = delete;
    TokenSetSlot& operator =(const TokenSetSlot&) = delete;

    size_t size() const { return _set.size(); /* lockfree access should be ok */ }
    void reserve(size_t capacity) { _set.reserve(capacity); }

    template <typename _TokenTraits = TokenTraits<_Char> >
    TokenData<_Char> GetOrCreate(const BasicStringSlice<_Char>& content);
    template <typename _TokenTraits = TokenTraits<_Char> >
    TokenData<_Char> GetOrCreate(const _Char *content, size_t length);
    template <typename _TokenTraits = TokenTraits<_Char> >
    TokenData<_Char> GetOrCreate(const _Char *content);

    template <typename _TokenTraits = TokenTraits<_Char> >
    static bool Validate(const BasicStringSlice<_Char>& content);

    void Clear();

private:
    typedef std::unordered_set<
        BasicStringSlice<_Char>,
        StringSliceHasher<_Char, _CaseSensitive>,
        StringSliceEqualTo<_Char, _CaseSensitive>,
        typename _Allocator::template rebind< TokenData<_Char> >::other
    >   set_type;

    std::mutex _barrier;
    set_type _set;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, CaseSensitive _CaseSensitive, typename _Allocator = ALLOCATOR(Token, TokenBucket<_Char>) >
class TokenSet {
public:
    typedef TokenSetSlot<_Char, _CaseSensitive, _Allocator> token_set_slot_type;

    enum : size_t {
        SlotCount = 4,
        SlotMask = (SlotCount - 1)
    };

    explicit TokenSet(size_t capacity);
    ~TokenSet();

    TokenSet(const TokenSet&) = delete;
    TokenSet& operator =(const TokenSet&) = delete;

    size_t size() const;

    template <typename _TokenTraits = TokenTraits<_Char> >
    TokenData<_Char> GetOrCreate(const BasicStringSlice<_Char>& content);
    template <typename _TokenTraits = TokenTraits<_Char> >
    TokenData<_Char> GetOrCreate(const _Char *content, size_t length);
    template <typename _TokenTraits = TokenTraits<_Char> >
    TokenData<_Char> GetOrCreate(const _Char *content);

    void Clear();

    // slot hash function : tells in which slot goes content
    template <typename _TokenTraits = TokenTraits<_Char> >
    size_t SlotHash(const BasicStringSlice<_Char>& content);

private:
    // to diminish lock contention
    token_set_slot_type _slots[SlotCount];
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Token>
class TokenFactory : Meta::Singleton<
    TokenSet<
    typename _Token::char_type,
    CaseSensitive(_Token::Sensitiveness),
    typename _Token::allocator_type
    >,
    TokenFactory<_Token>
> {
public:
    typedef TokenSet<
        typename _Token::char_type,
        CaseSensitive(_Token::Sensitiveness),
        typename _Token::allocator_type
    >   set_type;
    typedef Meta::Singleton<set_type, TokenFactory> parent_type;

    using parent_type::Instance;
    using parent_type::HasInstance;
    using parent_type::Destroy;

    static void Create(size_t capacity) {
        parent_type::Create(capacity);
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <
    typename        _StreamChar,
    typename        _StreamTraits,
    typename        _Tag,
    typename        _TokenChar,
    CaseSensitive   _CaseSensitive,
    typename        _TokenTraits,
    typename        _Allocator
>
std::basic_ostream<_StreamChar, _StreamTraits>& operator <<(
    std::basic_ostream<_StreamChar, _StreamTraits>& oss,
    const Token<_Tag, _TokenChar, _CaseSensitive, _TokenTraits, _Allocator>& token) {
    if (!token.empty())
        oss << token.cstr();
    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Container/Token-inl.h"
