#pragma once

#include "Core.h"

#include "Allocator/LinearHeap.h"
#include "IO/StringView.h"
#include "IO/TextWriter_fwd.h"
#include "Meta/Singleton.h"

#include <mutex>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Needs to access the singleton through an exported function for DLLs builds
//----------------------------------------------------------------------------
#define BASICTOKEN_CLASS_DECL(_API, _NAME, _CHAR, _CASESENSITIVE, _TRAITS) \
    class CONCAT(_NAME, ActualTokenTraits) : public _TRAITS { \
    public: \
        using _TRAITS::Locale; \
        using _TRAITS::IsAllowedChar; \
        _API static void CreateFactory(); \
        _API static ::PPE::FTokenFactory& Factory() NOEXCEPT; \
        _API static void DestroyFactory(); \
    }; \
    class _API _NAME : public PPE::TToken< \
        _NAME, \
        _CHAR, \
        _CASESENSITIVE, \
        CONCAT(_NAME, ActualTokenTraits) \
    > { \
    public: \
        typedef PPE::TToken< \
            _NAME, \
            _CHAR, \
            _CASESENSITIVE, \
            CONCAT(_NAME, ActualTokenTraits) \
        >   parent_type; \
        \
        using parent_type::parent_type; \
        using parent_type::operator =; \
    }
//----------------------------------------------------------------------------
#define BASICTOKEN_CLASS_DEF(_NAME) \
    using CONCAT(_NAME, TokenFactory) = ::PPE::Meta::TIndirectSingleton<::PPE::FTokenFactory, _NAME>; \
    void CONCAT(_NAME, ActualTokenTraits)::CreateFactory() { \
        CONCAT(_NAME, TokenFactory)::Create(); \
    } \
    ::PPE::FTokenFactory& CONCAT(_NAME, ActualTokenTraits)::Factory() NOEXCEPT { \
        return CONCAT(_NAME, TokenFactory)::Get(); \
    } \
    void CONCAT(_NAME, ActualTokenTraits)::DestroyFactory() { \
        CONCAT(_NAME, TokenFactory)::Destroy(); \
    }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char>
class TTokenTraits {
public:
    const std::locale& Locale() const {  return std::locale::classic(); }
    bool IsAllowedChar(_Char ch) const { return std::isprint(ch, Locale()); }

    static void CreateFactory() = delete;
    static class FTokenFactory& Factory() = delete;
    static void DestroyFactory() = delete;
};
//----------------------------------------------------------------------------
template <typename _TokenTraits, typename _Char>
bool IsValidToken(const TBasicStringView<_Char>& content);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_CORE_API FTokenFactory {
public:
    STATIC_CONST_INTEGRAL(size_t, MaxTokenLength, 1024);

    struct FEntry {
        FEntry* Next;
        size_t HashValue;
#if USE_PPE_ASSERT
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

    std::mutex _barrier;
    LINEARHEAP(Token) _heap;

    FEntry* _bucketHeads[NumBuckets] = { 0 };
    FEntry* _bucketTails[NumBuckets] = { 0 };
};
//----------------------------------------------------------------------------
template <
    typename        _Tag,
    typename        _Char,
    ECase            _Sensitive,
    typename        _TokenTraits
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

    TToken(const TToken& other) = default;
    TToken& operator =(const TToken& other) = default;

    explicit TToken(const stringview_type& content) : _handle(FindOrAdd_(content)) {}
    TToken& operator =(const stringview_type& content) { _handle = FindOrAdd_(content); return (*this); }

    template <size_t _Dim>
    explicit TToken(const _Char(&content)[_Dim])
        : TToken(MakeStringView(content)) {}
    template <size_t _Dim>
    TToken& operator =(const _Char(&content)[_Dim]) {
        return operator =(MakeStringView(content));
    }

    size_t size() const { return (_handle ? _handle->Length : 0); }
    bool empty() const { return (_handle == nullptr); }

    const _Char* c_str() const { return (const _Char*)(_handle ? _handle->Data() : nullptr); }
    const _Char* data() const { return (const _Char*)(_handle ? _handle->Data() : nullptr); }

    stringview_type MakeView() const {
        return ((_handle)
            ? stringview_type((const _Char*)_handle->Data(), _handle->Length)
            : stringview_type() );
    }

    hash_t HashValue() const { return (_handle ? _handle->HashValue : 0); }

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

    static void Start() { token_traits::CreateFactory(); }
    static void Shutdown() { token_traits::DestroyFactory(); }

    static bool IsValidToken(const stringview_type& str) {
        return PPE::IsValidToken<token_traits>(str);
    }

    static hash_t HashValue(const stringview_type& str) {
        return hasher_type{}(str);
    }

private:
    typedef FTokenFactory::FEntry handle_type;

    const handle_type* _handle;

    static const handle_type* FindOrAdd_(const stringview_type& str);
};
//----------------------------------------------------------------------------
PPE_ASSUME_TEMPLATE_AS_POD(
    COMMA_PROTECT(TToken<_Tag COMMA _Char COMMA _Sensitive COMMA _TokenTraits>),
    typename _Tag,
    typename _Char,
    ECase    _Sensitive,
    typename _TokenTraits)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, ECase _Sensitive, typename _TokenTraits>
FTextWriter& operator <<(FTextWriter& oss, const TToken<_Tag, _Char, _Sensitive, _TokenTraits>& token) {
    if (not token.empty())
        oss << token.MakeView();
    return oss;
}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, ECase _Sensitive, typename _TokenTraits>
FWTextWriter& operator <<(FWTextWriter& oss, const TToken<_Tag, _Char, _Sensitive, _TokenTraits>& token) {
    if (not token.empty())
        oss << token.MakeView();
    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#include "Container/Token-inl.h"
