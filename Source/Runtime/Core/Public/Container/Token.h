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
#define BEGIN_BASICTOKEN_CLASS_DEF(_NAME, _CHAR, _CASESENSITIVE, _TRAITS) \
    class _NAME : public PPE::TToken< \
        _NAME, \
        _CHAR, \
        _CASESENSITIVE, \
        _TRAITS \
    > { \
    public: \
        typedef PPE::TToken< \
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

    struct FEntry {
        FEntry* Next;
        size_t HashValue;
#ifdef WITH_PPE_ASSERT
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

    explicit TToken(const TBasicString<_Char>& content)
        : TToken(content.MakeView()) {}
    TToken& operator = (const TBasicString<_Char>& content) {
        return operator =(content.MakeView());
    }

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

    static void Start() { factory_type::Create(); }
    static void Shutdown() { factory_type::Destroy(); }

    static hash_t HashValue(const stringview_type& str) {
        return hasher_type{}(str);
    }

private:
    typedef FTokenFactory::FEntry handle_type;
    typedef Meta::TIndirectSingleton<FTokenFactory, _Tag> factory_type;

    const handle_type* _handle;

    static const handle_type* FindOrAdd_(const stringview_type& str);
};
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
