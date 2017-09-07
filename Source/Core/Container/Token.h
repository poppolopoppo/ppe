#pragma once

#include "Core/Core.h"

#if 0
#include "Core/Allocator/Allocation.h"
#include "Core/Container/HashSet.h"
#include "Core/Container/Stack.h"
#include "Core/IO/StringView.h"

#include <iosfwd>
#include <mutex>
#endif 

#include "Core/IO/StringView.h"
#include "Core/Meta/Singleton.h"

#include <mutex>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define BEGIN_BASICTOKEN_CLASS_DEF(_NAME, _CHAR, _CASESENSITIVE, _TRAITS) \
    class _NAME : public Core::TToken< \
        _NAME, \
        _CHAR, \
        _CASESENSITIVE, \
        _TRAITS \
    > { \
    public: \
        typedef Core::TToken< \
            _NAME, \
            _CHAR, \
            _CASESENSITIVE, \
            _TRAITS \
        >   parent_type; \
        \
        using parent_type::parent_type; \
        using parent_type::operator =
//----------------------------------------------------------------------------
#define END_BASICTOKEN_CLASS_DEF() \
    }
//----------------------------------------------------------------------------
#define BASICTOKEN_CLASS_DEF(_NAME, _CHAR, _CASESENSITIVE, _TRAITS) \
    BEGIN_BASICTOKEN_CLASS_DEF(_NAME, _CHAR, _CASESENSITIVE, _TRAITS); \
    END_BASICTOKEN_CLASS_DEF() \
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char>
class TTokenTraits {
public:
    const std::locale& Locale() const {  return std::locale::classic(); }
    bool IsAllowedChar(_Char ch) const { return std::isprint(ch, Locale()); }
};
//----------------------------------------------------------------------------
template <typename _Char, typename _TokenTraits = TTokenTraits<_Char> >
bool ValidateToken(const TBasicStringView<_Char>& content);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FTokenFactory {
public:
    STATIC_CONST_INTEGRAL(size_t, MaxTokenLength, 1024);

    struct ALIGN(16) FChunk {
        FChunk* Next;
        u32 WriteOffset;
#ifdef WITH_CORE_ASSERT
#   ifdef ARCH_X86
        u64 Canary = 0xFADEDEADFADEDEADull;
        bool TestCanary() const { return (0xFADEDEADFADEDEADull == Canary); }
#   else
        u32 Canary = 0xFADEDEADul;
        bool TestCanary() const { return (0xFADEDEADul == Canary); }
#   endif
#endif
        FChunk() : Next(nullptr), WriteOffset(0) {}
        u8* Data() const {
            Assert(TestCanary());
            return (u8*)(this + 1);
        }
    };
    STATIC_ASSERT(sizeof(FChunk) == 16);

    struct FEntry {
        FEntry* Next;
        size_t HashValue;
#ifdef WITH_CORE_ASSERT
#   ifdef ARCH_X86
        u16 Length;
        u16 Canary = 0xFEED;
        bool TestCanary() const { return (0xFEED == Canary); }
#   else
        u32 Length;
        u32 Canary = 0xFEEDFACEul;
        bool TestCanary() const { return (0xFEEDFACEul == Canary); }
#   endif
#else
        size_t Length;
#endif
        u8* Data() const { return (u8*)(this + 1); }
        FEntry(size_t length, size_t hashValue) 
            : Next(nullptr)
            , HashValue(hashValue)
            , Length(checked_cast<decltype(Length)>(length)) {}
    };
    STATIC_ASSERT(sizeof(FEntry) == 3*sizeof(intptr_t));

    FTokenFactory();
    ~FTokenFactory();

    FTokenFactory(const FTokenFactory&) = delete;
    FTokenFactory& operator =(const FTokenFactory&) = delete;

    const FEntry* Lookup(size_t len, size_t hash, const FEntry* head) const;
    const FEntry* Allocate(void* src, size_t len, size_t stride, size_t hash, const FEntry* tail);

private:
    STATIC_CONST_INTEGRAL(size_t, MaskBuckets, 0xFFFF);
    STATIC_CONST_INTEGRAL(size_t, NumBuckets, MaskBuckets + 1);

    FChunk* _chunks;
    std::mutex _barrier;

    FEntry* _bucketHeads[NumBuckets] = { 0 };
    FEntry* _bucketTails[NumBuckets] = { 0 };
};
//----------------------------------------------------------------------------
template <
    typename        _Tag,
    typename        _Char,
    ECase            _Sensitive,
    typename        _TokenTraits = TTokenTraits<_Char>
>
class TToken {
public:
    typedef _Tag tag_type;
    typedef _Char char_type;
    typedef _TokenTraits token_traits;

    typedef TBasicStringView<_Char> stringview_type;

    typedef TStringViewEqualTo<_Char, _Sensitive> equalto_type;
    typedef TStringViewLess<_Char, _Sensitive> less_type;
    typedef TStringViewHasher<_Char, _Sensitive> hasher_type;

    enum { Sensitiveness = size_t(_Sensitive) };

    TToken() : _handle{ nullptr } {}

    TToken(const TToken& other) : _handle(other._handle) {}
    TToken& operator =(const TToken& other) { _handle = other._handle; return (*this); }

    explicit TToken(const stringview_type& content) : _handle(FindOrAdd_(content)) {}
    TToken& operator =(const stringview_type& content) { _handle = FindOrAdd_(content); return (*this); }

    template <typename _CharTraits, typename _Allocator>
    explicit TToken(const std::basic_string<_Char, _CharTraits, _Allocator>& content)
        : TToken(stringview_type(content.c_str(), content.size())) {}

    size_t size() const { return (_handle ? _handle->Length : 0); }
    bool empty() const { return (_handle == nullptr); }

    const _Char* c_str() const { return (const _Char*)(_handle ? _handle->Data() : nullptr); }
    const _Char* data() const { return (const _Char*)(_handle ? _handle->Data() : nullptr); }

    stringview_type MakeView() const { 
        return ((_handle)
            ? stringview_type((const _Char*)_handle->Data(), _handle->Length) 
            : stringview_type() ); 
    }

    size_t HashValue() const { return (_handle ? _handle->HashValue : 0); }

    void Swap(TToken& other) { std::swap(_handle, other._handle); }

    bool Equals(const TToken& other) const { return (_handle == other._handle); }
    bool Less(const TToken& other) const { 
        return ((_handle != other._handle)
            ? less_type{}(MakeView(), other.MakeView())
            : false ); 
    }

    bool Equals(const stringview_type& str) const { return equalto_type{}(MakeView(), str); }
    bool Less(const stringview_type& str) const { return less_type{}(MakeView(), str); }

    friend void swap(TToken& lhs, TToken& rhs) { lhs.Swap(rhs); }
    friend hash_t hash_value(const TToken& token) { return token.HashValue(); }
    friend stringview_type MakeStringView(const TToken& token) { return token.MakeView(); }

    friend bool operator ==(const TToken& lhs, const TToken& rhs) { return lhs.Equals(rhs); }
    friend bool operator !=(const TToken& lhs, const TToken& rhs) { return !operator ==(lhs, rhs); }

    friend bool operator < (const TToken& lhs, const TToken& rhs) { return lhs.Less(rhs); }
    friend bool operator >=(const TToken& lhs, const TToken& rhs) { return !operator < (lhs, rhs); }

    friend bool operator ==(const TToken& lhs, const stringview_type& rhs) { return lhs.Equals(rhs); }
    friend bool operator !=(const TToken& lhs, const stringview_type& rhs) { return !operator ==(lhs, rhs); }

    friend bool operator ==(const stringview_type& lhs, const TToken& rhs) { return rhs.Equals(lhs); }
    friend bool operator !=(const stringview_type& lhs, const TToken& rhs) { return !operator ==(lhs, rhs); }

    static void Start() { factory_type::Create(); }
    static void Shutdown() { factory_type::Destroy(); }

private:
    typedef FTokenFactory::FEntry handle_type;
    typedef Meta::TIndirectSingleton<FTokenFactory, _Tag> factory_type;

    const handle_type* _handle;

    static const handle_type* FindOrAdd_(const stringview_type& str);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if 0
template <typename _Char>
struct TTokenData {
    const _Char *Ptr;

    const _Char *c_str() const { return Ptr; }
    size_t size() const { return Ptr ? Ptr[-1] : 0; }
    bool empty() const { return Ptr == nullptr; }

    TBasicStringView<_Char> MakeView() const {
        return (nullptr != Ptr)
            ? TBasicStringView<_Char>(Ptr, Ptr[-1])
            : TBasicStringView<_Char>();
    }
};
//----------------------------------------------------------------------------
template <typename _Char, ECase _Sensitive>
struct FTokenDataEqualTo : TStringViewEqualTo<_Char, _Sensitive> {
    bool operator ()(const TTokenData<_Char>& lhs, const TTokenData<_Char>& rhs) const {
        return TStringViewEqualTo<_Char, _Sensitive>::operator ()(lhs.MakeView(), rhs.MakeView());
    }
};
//----------------------------------------------------------------------------
template <typename _Char, ECase _Sensitive>
struct TTokenDataLess : TStringViewLess<_Char, _Sensitive> {
    bool operator ()(const TTokenData<_Char>& lhs, const TTokenData<_Char>& rhs) const {
        return TStringViewLess<_Char, _Sensitive>::operator ()(lhs.MakeView(), rhs.MakeView());
    }
};
//----------------------------------------------------------------------------
template <typename _Char, ECase _Sensitive>
struct FTokenDataHasher : TStringViewHasher<_Char, _Sensitive> {
    size_t operator ()(const TTokenData<_Char>& data) const {
        return TStringViewHasher<_Char, _Sensitive>::operator ()(data.MakeView());
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Token>
class TTokenFactory;
//----------------------------------------------------------------------------
template <
    typename        _Tag,
    typename        _Char,
    ECase            _Sensitive,
    typename        _TokenTraits = TTokenTraits<_Char>,
    typename        _Allocator = ALLOCATOR(Token, _Char)
>
class TToken {
public:
    template <
        typename        _Tag2,
        typename        _Char2,
        ECase            _Sensitive2,
        typename        _TokenTraits2,
        typename        _Allocator2
    >   friend class TToken;

    typedef _Tag tag_type;
    typedef _Char char_type;
    typedef _TokenTraits token_traits;
    typedef _Allocator allocator_type;
    typedef TTokenFactory<TToken> factory_type;

    enum { Sensitiveness = size_t(_Sensitive) };

    TToken() : _data{nullptr} {}

    explicit TToken(const _Char* content);
    TToken& operator =(const _Char* content);

    explicit TToken(const TBasicStringView<_Char>& content);
    TToken& operator =(const TBasicStringView<_Char>& content);

    TToken(const _Char* content, size_t length);

    template <typename _CharTraits, typename _Allocator>
    explicit TToken(const std::basic_string<_Char, _CharTraits, _Allocator>& content)
        : TToken(content.c_str(), content.size()) {}

    TToken(const TToken& token);
    TToken& operator =(const TToken& token);

    size_t size() const { return _data.size(); }
    bool empty() const { return _data.empty(); }

    const _Char* c_str() const { return _data.c_str(); }
    const _Char* data() const { return _data.c_str(); }
    TBasicStringView<_Char> MakeView() const { return _data.MakeView(); }

    size_t HashValue() const;

    void Swap(TToken& other);

    bool Equals(const TToken& other) const;
    bool Less(const TToken& other) const;

    bool Equals(const _Char *cstr) const;
    bool Equals(const TBasicStringView<_Char>& slice) const;

    template <size_t _Dim>
    bool Equals(const _Char (&array)[_Dim]) const {
        return Equals(MakeStringView(array));
    }

    friend void swap(TToken& lhs, TToken& rhs) { lhs.Swap(rhs); }
    friend hash_t hash_value(const TToken& token) { return token.HashValue(); }
    friend TBasicStringView<const _Char> MakeStringView(const TToken& token) { return token.MakeView(); }

    friend bool operator ==(const TToken& lhs, const TToken& rhs) { return lhs.Equals(rhs); }
    friend bool operator !=(const TToken& lhs, const TToken& rhs) { return !operator ==(lhs, rhs); }

    friend bool operator < (const TToken& lhs, const TToken& rhs) { return lhs.Less(rhs); }
    friend bool operator >=(const TToken& lhs, const TToken& rhs) { return !operator < (lhs, rhs); }

    friend bool operator ==(const TToken& lhs, const _Char* rhs) { return lhs.Equals(rhs); }
    friend bool operator !=(const TToken& lhs, const _Char* rhs) { return !operator ==(lhs, rhs); }

    friend bool operator ==(const _Char* lhs, const TToken& rhs) { return rhs.Equals(lhs); }
    friend bool operator !=(const _Char* lhs, const TToken& rhs) { return !operator ==(lhs, rhs); }

    friend bool operator ==(const TToken& lhs, const TBasicStringView<_Char>& rhs) { return lhs.Equals(rhs); }
    friend bool operator !=(const TToken& lhs, const TBasicStringView<_Char>& rhs) { return !operator ==(lhs, rhs); }

    friend bool operator ==(const TBasicStringView<_Char>& lhs, const TToken& rhs) { return rhs.Equals(lhs); }
    friend bool operator !=(const TBasicStringView<_Char>& lhs, const TToken& rhs) { return !operator ==(lhs, rhs); }

    static void Start(size_t capacity);
    static void Clear();
    static void Shutdown();

    static factory_type& Factory();

    class FStartup {
    public:
        FStartup(int capacity) { Start(capacity); }
        ~FStartup() { Shutdown(); }
    };

private:
    TTokenData<_Char> _data;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define CORE_TOKEN_BUCKET_SIZE (64 * 1024) // 64 Kbytes
//----------------------------------------------------------------------------
template <typename _Char>
struct TTokenBucket {
    TTokenBucket* Next;
    enum { Size = (CORE_TOKEN_BUCKET_SIZE - sizeof(TTokenBucket*)-sizeof(TStack<_Char>)) / sizeof(_Char) - 1 };
    TFixedSizeStack<_Char, Size> Stack;
};
//----------------------------------------------------------------------------
template <typename _Char, typename _Allocator = ALLOCATOR(Token, TTokenBucket<_Char>) >
class TTokenAllocator : public _Allocator {
public:
    typedef TTokenBucket<_Char> FBucket;
    static_assert(sizeof(FBucket) == CORE_TOKEN_BUCKET_SIZE, "invalid FBucket size");

    TTokenAllocator();
    ~TTokenAllocator();

    TTokenAllocator(const TTokenAllocator&) = delete;
    TTokenAllocator& operator =(const TTokenAllocator&) = delete;

    _Char* Allocate(size_t count);
    void Clear();

private:
    FBucket* _buckets;
};
//----------------------------------------------------------------------------
#undef CORE_TOKEN_BUCKET_SIZE
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, ECase _Sensitive, typename _Allocator = ALLOCATOR(Token, TTokenBucket<_Char>) >
class TTokenSetSlot : TTokenAllocator<_Char, typename _Allocator::template rebind< TTokenBucket<_Char> >::other> {
public:
    typedef TTokenAllocator<_Char, typename _Allocator::template rebind< TTokenBucket<_Char> >::other > parent_type;

    explicit TTokenSetSlot();
    ~TTokenSetSlot();

    TTokenSetSlot(const TTokenSetSlot&) = delete;
    TTokenSetSlot& operator =(const TTokenSetSlot&) = delete;

    size_t size() const { return _set.size(); /* lockfree access should be ok */ }
    void reserve(size_t capacity) { _set.reserve(capacity); }

    template <typename _TokenTraits = TTokenTraits<_Char> >
    TTokenData<_Char> GetOrCreate(const TBasicStringView<_Char>& content);
    template <typename _TokenTraits = TTokenTraits<_Char> >
    TTokenData<_Char> GetOrCreate(const _Char *content, size_t length);
    template <typename _TokenTraits = TTokenTraits<_Char> >
    TTokenData<_Char> GetOrCreate(const _Char *content);

    template <typename _TokenTraits = TTokenTraits<_Char> >
    static bool Validate(const TBasicStringView<_Char>& content);

    void Clear();

private:
    typedef THashSet<
        TBasicStringView<_Char>,
        TStringViewHasher<_Char, _Sensitive>,
        TStringViewEqualTo<_Char, _Sensitive>,
        typename _Allocator::template rebind< TBasicStringView<_Char> >::other
    >   set_type;

    std::mutex _barrier;
    set_type _set;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, ECase _Sensitive, typename _Allocator = ALLOCATOR(Token, TTokenBucket<_Char>) >
class TTokenSet {
public:
    typedef TTokenSetSlot<_Char, _Sensitive, _Allocator> token_set_slot_type;

    enum : size_t {
        SlotCount = 4,
        SlotMask = (SlotCount - 1)
    };

    explicit TTokenSet(size_t capacity);
    ~TTokenSet();

    TTokenSet(const TTokenSet&) = delete;
    TTokenSet& operator =(const TTokenSet&) = delete;

    size_t size() const;

    template <typename _TokenTraits = TTokenTraits<_Char> >
    TTokenData<_Char> GetOrCreate(const TBasicStringView<_Char>& content);
    template <typename _TokenTraits = TTokenTraits<_Char> >
    TTokenData<_Char> GetOrCreate(const _Char *content, size_t length);
    template <typename _TokenTraits = TTokenTraits<_Char> >
    TTokenData<_Char> GetOrCreate(const _Char *content);

    void Clear();

    // slot hash function : tells in which slot goes content
    template <typename _TokenTraits = TTokenTraits<_Char> >
    size_t SlotHash(const TBasicStringView<_Char>& content);

private:
    // to diminish lock contention
    token_set_slot_type _slots[SlotCount];
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Token>
class TTokenFactory : Meta::TSingleton<
    TTokenSet<
        typename _Token::char_type,
        ECase(_Token::Sensitiveness),
        typename _Token::allocator_type
    >,
    TTokenFactory<_Token>
> {
public:
    typedef TTokenSet<
        typename _Token::char_type,
        ECase(_Token::Sensitiveness),
        typename _Token::allocator_type
    >   set_type;
    typedef Meta::TSingleton<set_type, TTokenFactory> parent_type;

    using parent_type::Instance;
#ifdef WITH_CORE_ASSERT
    using parent_type::HasInstance;
#endif
    using parent_type::Destroy;

    static void Create(size_t capacity) {
        parent_type::Create(capacity);
    }
};
#endif //!0
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <
    typename        _StreamChar,
    typename        _StreamTraits,
    typename        _Tag,
    typename        _TokenChar,
    ECase            _Sensitive,
    typename        _TokenTraits
>
std::basic_ostream<_StreamChar, _StreamTraits>& operator <<(
    std::basic_ostream<_StreamChar, _StreamTraits>& oss,
    const TToken<_Tag, _TokenChar, _Sensitive, _TokenTraits>& token) {
    if (!token.empty())
        oss << token.MakeView();
    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Container/Token-inl.h"
