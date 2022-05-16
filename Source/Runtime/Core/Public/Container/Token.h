#pragma once

#include "Core.h"

#include "Allocator/SlabHeap.h"
#include "IO/StringView.h"
#include "IO/TextWriter_fwd.h"
#include "Meta/Singleton.h"
#include "Thread/AtomicSpinLock.h"

// Needs to access the singleton through an exported function for DLLs builds
//----------------------------------------------------------------------------
#define BASICTOKEN_CLASS_DECL(_API, _NAME_WITHOUT_F, _CHAR, _CASESENSITIVE, _TRAITS) \
    class CONCAT(CONCAT(F, _NAME_WITHOUT_F), ActualTokenTraits) : public _TRAITS { \
    public: \
        using _TRAITS::IsAllowedChar; \
        _API static void CreateFactory(); \
        _API static ::PPE::FTokenFactory& Factory() NOEXCEPT; \
        _API static void DestroyFactory(); \
    }; \
    class _API CONCAT(F, _NAME_WITHOUT_F) : public PPE::TToken< \
        CONCAT(F, _NAME_WITHOUT_F), \
        _CHAR, \
        _CASESENSITIVE, \
        CONCAT(CONCAT(F, _NAME_WITHOUT_F), ActualTokenTraits) \
    > { \
    public: \
        typedef PPE::TToken< \
            CONCAT(F, _NAME_WITHOUT_F), \
            _CHAR, \
            _CASESENSITIVE, \
            CONCAT(CONCAT(F, _NAME_WITHOUT_F), ActualTokenTraits) \
        >   parent_type; \
        \
        using parent_type::parent_type; \
        using parent_type::operator =; \
        PPE_ASSUME_FRIEND_AS_POD(CONCAT(F, _NAME_WITHOUT_F)) \
    }; \
    class _API CONCAT(FLazy, _NAME_WITHOUT_F) : public PPE::TLazyToken< \
        CONCAT(F, _NAME_WITHOUT_F), \
        _CHAR, \
        _CASESENSITIVE, \
        CONCAT(CONCAT(F, _NAME_WITHOUT_F), ActualTokenTraits) \
    > { \
    public: \
        typedef PPE::TLazyToken< \
            CONCAT(F, _NAME_WITHOUT_F), \
            _CHAR, \
            _CASESENSITIVE, \
            CONCAT(CONCAT(F, _NAME_WITHOUT_F), ActualTokenTraits) \
        >   parent_type; \
        \
        using parent_type::parent_type; \
        using parent_type::operator =; \
        PPE_ASSUME_FRIEND_AS_POD(CONCAT(FLazy, _NAME_WITHOUT_F)) \
    }

#define BASICTOKEN_CLASS_DEF(_NAME) \
    using CONCAT(_NAME, TokenFactory) = ::PPE::Meta::TStaticIndirectSingleton<::PPE::FTokenFactory, _NAME>; \
    void CONCAT(_NAME, ActualTokenTraits)::CreateFactory() { \
        CONCAT(_NAME, TokenFactory)::Create(); \
    } \
    ::PPE::FTokenFactory& CONCAT(_NAME, ActualTokenTraits)::Factory() NOEXCEPT { \
        return CONCAT(_NAME, TokenFactory)::Get(); \
    } \
    void CONCAT(_NAME, ActualTokenTraits)::DestroyFactory() { \
        CONCAT(_NAME, TokenFactory)::Destroy(); \
    }

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char>
class TTokenTraits {
public:
    bool IsAllowedChar(_Char ch) const { return isprint(ch); }

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
    STATIC_CONST_INTEGRAL(size_t, MaskBuckets, 0x7FFF);
    STATIC_CONST_INTEGRAL(size_t, NumBuckets, MaskBuckets + 1);

    FAtomicMaskLock _barrier;
    SLABHEAP(Token) _heap;

    FEntry* _bucketHeads[NumBuckets] = { 0 };
    FEntry* _bucketTails[NumBuckets] = { 0 };
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <
    typename        _Tag,
    typename        _Char,
    ECase            _Sensitive,
    typename        _TokenTraits
>
class TLazyToken;
//----------------------------------------------------------------------------
template <
    typename        _Tag,
    typename        _Char,
    ECase            _Sensitive,
    typename        _TokenTraits
>
class TToken {
public:
    using tag_type = _Tag;
    using char_type = _Char;
    using token_traits = _TokenTraits;
    using lazytoken_type = TLazyToken<_Tag, _Char, _Sensitive, _TokenTraits>;

    using stringview_type = TBasicStringView<_Char>;

    using equalto_type = TStringViewEqualTo<_Char, _Sensitive>;
    using less_type = TStringViewLess<_Char, _Sensitive>;
    using hasher_type = TStringViewHasher<_Char, _Sensitive>;

    STATIC_CONST_INTEGRAL(ECase, Sensitiveness, _Sensitive);

    TToken() = default;

    TToken(const TToken& other) = default;
    TToken& operator =(const TToken& other) = default;

#if 1
    TToken(TToken&& rvalue) = default;
    TToken& operator =(TToken&& rvalue) = default;
#else
    TToken(TToken&& rvalue) NOEXCEPT { operator =(std::move(rvalue)); }
    TToken& operator =(TToken&& rvalue) NOEXCEPT {
        _handle = rvalue._handle;
        rvalue._handle = nullptr;
        return (*this);
    }
#endif

    explicit TToken(const lazytoken_type& lazy) : _handle(FindOrAdd_(lazy)) {}
    TToken& operator =(const lazytoken_type& lazy) { _handle = FindOrAdd_(lazy); return (*this); }

    explicit TToken(const stringview_type& content) : _handle(FindOrAdd_(content)) {}
    TToken& operator =(const stringview_type& content) { _handle = FindOrAdd_(content); return (*this); }

    template <size_t _Dim>
    explicit TToken(const _Char(&content)[_Dim])
    :   TToken(MakeStringView(content))
    {}
    template <size_t _Dim>
    TToken& operator =(const _Char(&content)[_Dim]) {
        return operator =(MakeStringView(content));
    }

    size_t size() const { return (_handle ? _handle->Length : 0); }
    bool empty() const { return (_handle == nullptr); }

    const _Char* c_str() const { return reinterpret_cast<const _Char*>(_handle ? _handle->Data() : nullptr); }
    const _Char* data() const { return reinterpret_cast<const _Char*>(_handle ? _handle->Data() : nullptr); }

    stringview_type MakeView() const {
        return ((_handle)
            ? stringview_type(reinterpret_cast<const _Char*>(_handle->Data()), _handle->Length)
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

    friend void swap(TToken& lhs, TToken& rhs) NOEXCEPT { lhs.Swap(rhs); }
    friend hash_t hash_value(const TToken& token) NOEXCEPT { return token.HashValue(); }
    friend stringview_type MakeStringView(const TToken& token) NOEXCEPT { return token.MakeView(); }

    friend bool operator ==(const TToken& lhs, const TToken& rhs) NOEXCEPT { return lhs.Equals(rhs); }
    friend bool operator !=(const TToken& lhs, const TToken& rhs) NOEXCEPT { return not operator ==(lhs, rhs); }
    friend bool operator < (const TToken& lhs, const TToken& rhs) NOEXCEPT { return lhs.Less(rhs); }
    friend bool operator >=(const TToken& lhs, const TToken& rhs) NOEXCEPT { return not operator < (lhs, rhs); }

    friend bool operator ==(const TToken& lhs, const stringview_type& rhs) NOEXCEPT { return lhs.Equals(rhs); }
    friend bool operator !=(const TToken& lhs, const stringview_type& rhs) NOEXCEPT { return not (lhs == rhs); }
    friend bool operator ==(const stringview_type& lhs, const TToken& rhs) NOEXCEPT { return rhs.Equals(lhs); }
    friend bool operator !=(const stringview_type& lhs, const TToken& rhs) NOEXCEPT { return not (lhs == rhs); }

    operator stringview_type () const NOEXCEPT { return MakeView(); }

    static void Start() { token_traits::CreateFactory(); }
    static void Shutdown() { token_traits::DestroyFactory(); }

    static bool IsValidToken(const stringview_type& str) NOEXCEPT {
        return PPE::IsValidToken<token_traits>(str);
    }

    static hash_t HashValue(const stringview_type& str) NOEXCEPT {
        return hasher_type{}(str);
    }

private:
    typedef FTokenFactory::FEntry handle_type;

    const handle_type* _handle{ nullptr };

    static const handle_type* FindOrAdd_(const lazytoken_type& lazy);
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
template <typename _Tag, typename _Char, ECase _Sensitive, typename _TokenTraits>
bool operator >>(const TBasicStringConversion<_Char>& iss, TToken<_Tag, _Char, _Sensitive, _TokenTraits>* token) {
    Assert(token);

    if (TToken<_Tag, _Char, _Sensitive, _TokenTraits>::IsValidToken(iss.Input)) {
        *token = iss.Input;
        return true;
    }

    return false;
}
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
template <
    typename        _Tag,
    typename        _Char,
    ECase            _Sensitive,
    typename        _TokenTraits
>
class TLazyToken {
public:
    using tag_type = _Tag;
    using char_type = _Char;
    using token_traits = _TokenTraits;
    using token_type = TToken<_Tag, _Char, _Sensitive, _TokenTraits>;

    using stringview_type = TBasicStringView<_Char>;

    using equalto_type = TStringViewEqualTo<_Char, _Sensitive>;
    using less_type = TStringViewLess<_Char, _Sensitive>;
    using hasher_type = TStringViewHasher<_Char, _Sensitive>;

    STATIC_CONST_INTEGRAL(ECase, Sensitiveness, _Sensitive);

    TLazyToken() = default;

    TLazyToken(const TLazyToken& other) = default;
    TLazyToken& operator =(const TLazyToken& other) = default;

    TLazyToken(TLazyToken&& rvalue) = default;
    TLazyToken& operator =(TLazyToken&& rvalue) = default;

    explicit TLazyToken(const stringview_type& content)
    :   _str(content)
    ,   _hash(hasher_type{}(content))
    {}

    TLazyToken& operator =(const stringview_type& content) {
        return (*this = TLazyToken{ content });
    }

    template <size_t _Dim>
    explicit TLazyToken(const _Char(&content)[_Dim])
    :   TLazyToken(MakeStringView(content)) {}
    template <size_t _Dim>
    TLazyToken& operator =(const _Char(&content)[_Dim]) {
        return operator =(MakeStringView(content));
    }

    CONSTF size_t size() const { return _str.size(); }
    CONSTF bool empty() const { return _str.empty(); }
    CONSTF const _Char* data() const { return _str.data(); }
    CONSTF stringview_type MakeView() const { return _str; }
    CONSTF hash_t HashValue() const { return _hash; }

    void Swap(TLazyToken& other) NOEXCEPT {
        std::swap(_str, other._str);
        std::swap(_hash, other._hash);
    }

    CONSTF bool Valid() const NOEXCEPT { return IsValidToken(_str); }

    bool Equals(const TLazyToken& other) const NOEXCEPT { return (_hash == other._hash && equalto_type{}(_str, other._str)); }
    bool Less(const TLazyToken& other) const NOEXCEPT { return (less_type{}(_str, other._str)); }

    bool Equals(const token_type& token) const NOEXCEPT { return (_hash == token.HashValue() && token.Equals(_str)); }
    bool Less(const token_type& token) const NOEXCEPT { return less_type{}(_str, token.MakeView()); }

    bool Equals(const stringview_type& str) const NOEXCEPT { return equalto_type{}(_str, str); }
    bool Less(const stringview_type& str) const NOEXCEPT { return less_type{}(_str, str); }

    friend void swap(TLazyToken& lhs, TLazyToken& rhs) NOEXCEPT { lhs.Swap(rhs); }
    friend hash_t hash_value(const TLazyToken& token) NOEXCEPT { return token.HashValue(); }
    friend stringview_type MakeStringView(const TLazyToken& token) NOEXCEPT { return token.MakeView(); }

    static CONSTF bool IsValidToken(const stringview_type & str) NOEXCEPT {
        return PPE::IsValidToken<token_traits>(str);
    }

    friend bool operator ==(const TLazyToken& lhs, const TLazyToken& rhs) NOEXCEPT { return lhs.Equals(rhs); }
    friend bool operator !=(const TLazyToken& lhs, const TLazyToken& rhs) NOEXCEPT { return !operator ==(lhs, rhs); }
    friend bool operator < (const TLazyToken& lhs, const TLazyToken& rhs) NOEXCEPT { return lhs.Less(rhs); }
    friend bool operator >=(const TLazyToken& lhs, const TLazyToken& rhs) NOEXCEPT { return !operator < (lhs, rhs); }

    friend bool operator ==(const TLazyToken& lhs, const token_type& rhs) NOEXCEPT { return lhs.Equals(rhs); }
    friend bool operator !=(const TLazyToken& lhs, const token_type& rhs) NOEXCEPT { return not (lhs == rhs); }
    friend bool operator ==(const token_type& lhs, const TLazyToken& rhs) NOEXCEPT { return rhs.Equals(lhs); }
    friend bool operator !=(const token_type& lhs, const TLazyToken& rhs) NOEXCEPT { return not (lhs == rhs); }

    friend bool operator ==(const TLazyToken& lhs, const stringview_type & rhs) NOEXCEPT { return lhs.Equals(rhs); }
    friend bool operator !=(const TLazyToken& lhs, const stringview_type & rhs) NOEXCEPT { return not (lhs == rhs); }
    friend bool operator ==(const stringview_type & lhs, const TLazyToken& rhs) NOEXCEPT { return rhs.Equals(lhs); }
    friend bool operator !=(const stringview_type & lhs, const TLazyToken& rhs) NOEXCEPT { return not (lhs == rhs); }

    friend bool operator < (const TLazyToken& lhs, const token_type& rhs) NOEXCEPT { return lhs.Less(rhs); }
    friend bool operator >=(const TLazyToken& lhs, const token_type& rhs) NOEXCEPT { return not (lhs < rhs); }
    friend bool operator < (const token_type& lhs, const TLazyToken& rhs) NOEXCEPT { return lhs.Less(rhs.MakeView()); }
    friend bool operator >=(const token_type& lhs, const TLazyToken& rhs) NOEXCEPT { return not (lhs < rhs); }

private:
    stringview_type _str;
    hash_t _hash{ 0 };
};
//----------------------------------------------------------------------------
PPE_ASSUME_TEMPLATE_AS_POD(
    COMMA_PROTECT(TLazyToken<_Tag COMMA _Char COMMA _Sensitive COMMA _TokenTraits>),
    typename _Tag,
    typename _Char,
    ECase    _Sensitive,
    typename _TokenTraits )
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, ECase _Sensitive, typename _TokenTraits>
bool operator >>(const TBasicStringConversion<_Char>& iss, TLazyToken<_Tag, _Char, _Sensitive, _TokenTraits>* token) {
    Assert(token);

    if (TLazyToken<_Tag, _Char, _Sensitive, _TokenTraits>::IsValidToken(iss.Input)) {
        *token = iss.Input;
        return true;
    }

    return false;
}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, ECase _Sensitive, typename _TokenTraits>
FTextWriter& operator <<(FTextWriter& oss, const TLazyToken<_Tag, _Char, _Sensitive, _TokenTraits>& token) {
    if (not token.empty())
        oss << token.MakeView();
    return oss;
}
//----------------------------------------------------------------------------
template <typename _Tag, typename _Char, ECase _Sensitive, typename _TokenTraits>
FWTextWriter& operator <<(FWTextWriter& oss, const TLazyToken<_Tag, _Char, _Sensitive, _TokenTraits>& token) {
    if (not token.empty())
        oss << token.MakeView();
    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#include "Container/Token-inl.h"
