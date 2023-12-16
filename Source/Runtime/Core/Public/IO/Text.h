#pragma once

#include "Core_fwd.h"

#include "Allocator/Allocation.h"
#include "Container/HashHelpers.h"
#include "HAL/PlatformString.h"
#include "IO/ConstChar.h"
#include "IO/StringView.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename _Allocator = ALLOCATOR(Text)>
class TBasicText : _Allocator {
private:
    using allocator_traits = TAllocatorTraits<_Allocator>;

    enum EStringSource_ : _Char {
        Inplace     = 0,
        Literal     = 1,
        External    = 2,

        //__Reserved  = 3,
    };

    _Allocator& Allocator_() NOEXCEPT { return (*this); }
    const _Allocator& Allocator_() const NOEXCEPT { return (*this); }

    struct FExternalString_ {
        EStringSource_ Source{ External };
        u32 Length{ 0 };
        const _Char* Storage{ nullptr };
    };

    struct FInplaceString_ {
        EStringSource_ Source{ Inplace };
        _Char Inline[(sizeof(FExternalString_) - sizeof(EStringSource_))/sizeof(_Char)]{};
    };

    STATIC_CONST_INTEGRAL(size_t, SmallCapacity_, sizeof(FInplaceString_::Inline)/sizeof(_Char) - 1/* '\0' */);

    union {
        FInplaceString_ _inplace{};
        FExternalString_ _external;
    };

    template <typename _Char2, typename _Allocator2>
    friend class TBasicText;
    template <typename _Char2, ECase _Sensitive, typename _Allocator2>
    friend class TBasicTextMemoization;

public:
    CONSTEXPR TBasicText() = default;

    CONSTEXPR  explicit TBasicText(Meta::FForceInit) NOEXCEPT
    :   TBasicText(Meta::MakeForceInit<_Allocator>())
    {}

    explicit TBasicText(_Allocator&& allocator) NOEXCEPT
    :   _Allocator(std::move(allocator))
    {}
    explicit TBasicText(const _Allocator& allocator) NOEXCEPT
    :   _Allocator(allocator)
    {}

    TBasicText(TBasicStringLiteral<_Char> txt) NOEXCEPT : TBasicText(ForceInit) { Assign(txt); }

    template <size_t _Len>
    TBasicText(const _Char (&staticString)[_Len]) NOEXCEPT : TBasicText(TBasicStringLiteral<_Char>(staticString)) {}

    TBasicText(TBasicConstChar<_Char> txt) : TBasicText() { Assign(txt); }
    TBasicText(TBasicStringView<_Char> txt) : TBasicText() { Assign(txt); }

    TBasicText(_Allocator&& allocator, TBasicConstChar<_Char> txt) : TBasicText(std::move(allocator)) { Assign(txt); }
    TBasicText(const _Allocator& allocator, TBasicConstChar<_Char> txt) : TBasicText(allocator) { Assign(txt); }

    TBasicText(_Allocator&& allocator, TBasicStringView<_Char> txt) : TBasicText(std::move(allocator)) { Assign(txt); }
    TBasicText(const _Allocator& allocator, TBasicStringView<_Char> txt) : TBasicText(allocator) { Assign(txt); }

    TBasicText(TBasicText&& rvalue) NOEXCEPT
    :   _Allocator(std::move(rvalue.Allocator_()))
    ,   _external(rvalue._external) {
        rvalue._inplace = {};
    }
    TBasicText& operator =(TBasicText&& rvalue) NOEXCEPT {
        Reset();
        Allocator_() = std::move(rvalue.Allocator_());
        _external = rvalue._external;
        rvalue._inplace = {};
        return (*this);
    }

    TBasicText(const TBasicText& other)
    :   TBasicText(other.Allocator_()) {
        operator =(other);
    }
    TBasicText& operator =(const TBasicText& other) {
        Assign(other);
        return (*this);
    }

    ~TBasicText() {
        Reset();
    }

    NODISCARD CONSTEXPR bool IsInplace() const { return (_inplace.Source == Inplace); }
    NODISCARD CONSTEXPR bool IsLiteral() const { return (_inplace.Source == Literal); }
    NODISCARD CONSTEXPR bool IsExternal() const { return (_inplace.Source == External); }

    NODISCARD CONSTEXPR const _Char* c_str() const NOEXCEPT {
        return (_inplace.Source == Inplace
            ? &_inplace.Inline[0]
            : _external.Storage );
    }

    CONSTEXPR operator const _Char* () const NOEXCEPT { return c_str(); }
    PPE_FAKEBOOL_OPERATOR_DECL() { return (length() != 0); }

    size_t length() const NOEXCEPT {
        return (_inplace.Source == Inplace
            ? ConstChar().length()
            : _external.Length );
    }

    NODISCARD CONSTEXPR TBasicConstChar<_Char> ConstChar() const NOEXCEPT {
        return { c_str() };
    }

    NODISCARD CONSTEXPR TBasicStringView<_Char> MakeView() const NOEXCEPT {
        return { c_str(), length() };
    }

    template <typename _Allocator2>
    void Assign(const TBasicText<_Char, _Allocator2>& copy) {
        switch (copy._inplace.Source) {
        case Inplace:
            AssignSmall(copy.MakeView());
            break;
        case Literal:
            AssignLiteral(copy.ConstChar());
            break;
        case External:
            AssignExternal(copy.MakeView());
            break;
        default:
            AssertNotImplemented();
        }
    }

    void Assign(TBasicConstChar<_Char> cstr) {
        Assign(cstr.MakeView());
    }
    void Assign(TBasicStringView<_Char> txt) {
        if (txt.size() <= SmallCapacity_)
            AssignSmall(txt);
        else
            AssignExternal(txt);
    }

    void Assign(TBasicStringLiteral<_Char> literal) {
        AssignLiteral(literal);
    }

    void AssignSmall(TBasicStringView<_Char> txt) NOEXCEPT {
        Assert(txt.size() <= SmallCapacity_);
        Reset();

        _inplace.Source = Inplace;
        FPlatformMemory::Memcpy(&_inplace.Inline[0], txt.data(), txt.size());
    }

    void AssignExternal(TBasicStringView<_Char> txt) {
        Assert(txt.size() > SmallCapacity_);

        Reset();

        const TMemoryView<_Char> cpy = allocator_traits::template AllocateT<_Char>(Allocator_(), txt.size() + 1/*'\0'*/);
        AssertReleaseMessage("text allocation failed", cpy.size() == txt.size() + 1);

        txt.CopyTo(cpy.ShiftBack());
        cpy.back() = STRING_LITERAL(_Char, '\0');

        _external.Source = External;
        _external.Storage = cpy.data();
        _external.Length = checked_cast<u32>(txt.size());
    }

    void AssignLiteral(TBasicConstChar<_Char> const_char) NOEXCEPT {
        Reset();

        _external.Source = Literal;
        _external.Storage = const_char.c_str();
        _external.Length = checked_cast<u32>(const_char.length());
    }

    void AssignLiteral(TBasicStringLiteral<_Char> literal) NOEXCEPT {
        Reset();

        _external.Source = Literal;
        _external.Storage = literal.Data;
        _external.Length = checked_cast<u32>(literal.Length);
    }

    void Reset() {
        if (_inplace.Source == External) {
            const size_t len = length();
            Assert(len > SmallCapacity_);
            const TMemoryView<_Char> alloc{ const_cast<_Char*>(_external.Storage), len + 1 };
            allocator_traits::template DeallocateT(*this, alloc);
        }

        _inplace = {};
    }

    NODISCARD static CONSTEXPR TBasicText MakeForeignText(TBasicConstChar<_Char> foreign) NOEXCEPT {
        // foreign block is considered as a literal and won't be deallocated
        TBasicText result{ForceInit};
        result.AssignLiteral(foreign);
        return result;
    }

    NODISCARD static CONSTEXPR TBasicText MakeForeignText(TBasicStringLiteral<_Char> literal) NOEXCEPT {
        // foreign block is considered as a literal and won't be deallocated
        TBasicText result{ForceInit};
        result.AssignLiteral(literal);
        return result;
    }

    NODISCARD CONSTF bool Equals(const TBasicText& other) const NOEXCEPT {
        if (IsInplace() != other.IsInplace())
            return false;
        if (_external.Length != other._external.Length)
            return false;
        return (FPlatformString::Cmp(c_str(), other.c_str()) == 0);
    }
    NODISCARD CONSTF bool EqualsI(const TBasicText& other) const NOEXCEPT {
        if (IsInplace() != other.IsInplace())
            return false;
        if (_external.Length != other._external.Length)
            return false;
        return (FPlatformString::CmpI(c_str(), other.c_str()) == 0);
    }

    NODISCARD CONSTF int Compare(const TBasicText& other) const NOEXCEPT { return FPlatformString::Cmp(c_str(), other.c_str()); }
    NODISCARD CONSTF int CompareI(const TBasicText& other) const NOEXCEPT { return FPlatformString::CmpI(c_str(), other.c_str()); }

    NODISCARD CONSTF bool Less(const TBasicText& other) const NOEXCEPT { return (Compare(other) < 0); }
    NODISCARD CONSTF bool LessI(const TBasicText& other) const NOEXCEPT { return (CompareI(other) < 0); }

    NODISCARD CONSTF hash_t HashValue() const NOEXCEPT { return hash_string(MakeView()); }
    NODISCARD CONSTF hash_t HashValueI() const NOEXCEPT { return hash_stringI(MakeView()); }

    friend hash_t hash_value(const TBasicText& txt) NOEXCEPT { return txt.HashValue(); }

    friend bool operator ==(const TBasicText& lhs, const TBasicText& rhs) NOEXCEPT { return lhs.Equals(rhs); }
    friend bool operator !=(const TBasicText& lhs, const TBasicText& rhs) NOEXCEPT { return not operator ==(lhs, rhs); }

    friend bool operator < (const TBasicText& lhs, const TBasicText& rhs) NOEXCEPT { return lhs.Less(rhs); }
    friend bool operator >=(const TBasicText& lhs, const TBasicText& rhs) NOEXCEPT { return not operator < (lhs, rhs); }

    friend bool operator ==(const TBasicText& lhs, const TBasicConstChar<_Char>& rhs) NOEXCEPT { return lhs.Equals(MakeForeignText(rhs)); }
    friend bool operator !=(const TBasicText& lhs, const TBasicConstChar<_Char>& rhs) NOEXCEPT { return not operator ==(lhs, rhs); }

    friend bool operator < (const TBasicText& lhs, const TBasicConstChar<_Char>& rhs) NOEXCEPT { return lhs.Less(MakeForeignText(rhs)); }
    friend bool operator >=(const TBasicText& lhs, const TBasicConstChar<_Char>& rhs) NOEXCEPT { return not operator < (lhs, rhs); }

    friend bool operator ==(const TBasicConstChar<_Char>& lhs, const TBasicText& rhs) NOEXCEPT { return MakeForeignText(lhs).Equals(rhs); }
    friend bool operator !=(const TBasicConstChar<_Char>& lhs, const TBasicText& rhs) NOEXCEPT { return not operator ==(lhs, rhs); }

    friend bool operator < (const TBasicConstChar<_Char>& lhs, const TBasicText& rhs) NOEXCEPT { return MakeForeignText(lhs).Less(rhs); }
    friend bool operator >=(const TBasicConstChar<_Char>& lhs, const TBasicText& rhs) NOEXCEPT { return not operator < (lhs, rhs); }

    friend bool operator ==(const TBasicText& lhs, const TBasicStringView<_Char>& rhs) NOEXCEPT { return lhs.MakeView().Equals(rhs); }
    friend bool operator !=(const TBasicText& lhs, const TBasicStringView<_Char>& rhs) NOEXCEPT { return not operator ==(lhs, rhs); }

    friend bool operator < (const TBasicText& lhs, const TBasicStringView<_Char>& rhs) NOEXCEPT { return lhs.MakeView().Less(rhs); }
    friend bool operator >=(const TBasicText& lhs, const TBasicStringView<_Char>& rhs) NOEXCEPT { return not operator < (lhs, rhs); }

    friend bool operator ==(const TBasicStringView<_Char>& lhs, const TBasicText& rhs) NOEXCEPT { return lhs.Equals(rhs.MakeView()); }
    friend bool operator !=(const TBasicStringView<_Char>& lhs, const TBasicText& rhs) NOEXCEPT { return not operator ==(lhs, rhs); }

    friend bool operator < (const TBasicStringView<_Char>& lhs, const TBasicText& rhs) NOEXCEPT { return lhs.Less(rhs.MakeView()); }
    friend bool operator >=(const TBasicStringView<_Char>& lhs, const TBasicText& rhs) NOEXCEPT { return not operator < (lhs, rhs); }
};
//----------------------------------------------------------------------------
template <typename _Char, ECase _Sensitive>
struct TTextEqualTo {
    template <typename _Allocator>
    CONSTEXPR bool operator ()(const TBasicText<_Char, _Allocator>& lhs, const TBasicText<_Char, _Allocator>& rhs) const NOEXCEPT { return lhs.Equals(rhs); }
    template <typename _Allocator>
    CONSTEXPR bool operator ()(const TBasicText<_Char, _Allocator>& lhs, const TBasicConstChar<_Char>& rhs) const NOEXCEPT { return lhs.Equals(TBasicText<_Char>::MakeForeignText(rhs)); }
    template <typename _Allocator>
    CONSTEXPR bool operator ()(const TBasicConstChar<_Char>& lhs, const TBasicText<_Char, _Allocator>& rhs) const NOEXCEPT { return TBasicText<_Char>::MakeForeignText(lhs).Equals(rhs); }
    template <typename _Allocator>
    CONSTEXPR bool operator ()(const TBasicText<_Char, _Allocator>& lhs, const TBasicStringView<_Char>& rhs) const NOEXCEPT { return Equals(lhs.MakeView(), rhs); }
    template <typename _Allocator>
    CONSTEXPR bool operator ()(const TBasicStringView<_Char>& lhs, const TBasicText<_Char, _Allocator>& rhs) const NOEXCEPT { return Equals(lhs, rhs.MakeView()); }
};
template <typename _Char>
struct TTextEqualTo<_Char, ECase::Insensitive> {
    template <typename _Allocator>
    CONSTEXPR bool operator ()(const TBasicText<_Char, _Allocator>& lhs, const TBasicText<_Char, _Allocator>& rhs) const NOEXCEPT { return lhs.EqualsI(rhs); }
    template <typename _Allocator>
    CONSTEXPR bool operator ()(const TBasicText<_Char, _Allocator>& lhs, const TBasicConstChar<_Char>& rhs) const NOEXCEPT { return lhs.EqualsI(TBasicText<_Char>::MakeForeignText(rhs)); }
    template <typename _Allocator>
    CONSTEXPR bool operator ()(const TBasicConstChar<_Char>& lhs, const TBasicText<_Char, _Allocator>& rhs) const NOEXCEPT { return TBasicText<_Char>::MakeForeignText(lhs).EqualsI(rhs); }
    template <typename _Allocator>
    CONSTEXPR bool operator ()(const TBasicText<_Char, _Allocator>& lhs, const TBasicStringView<_Char>& rhs) const NOEXCEPT { return EqualsI(lhs.MakeView(), rhs); }
    template <typename _Allocator>
    CONSTEXPR bool operator ()(const TBasicStringView<_Char>& lhs, const TBasicText<_Char, _Allocator>& rhs) const NOEXCEPT { return EqualsI(lhs, rhs.MakeView()); }
};
//----------------------------------------------------------------------------
template <typename _Char, ECase _Sensitive>
struct TTextLess {
    template <typename _Allocator>
    CONSTEXPR bool operator ()(const TBasicText<_Char, _Allocator>& lhs, const TBasicText<_Char>& rhs) const NOEXCEPT { return lhs.Less(rhs); }
    template <typename _Allocator>
    CONSTEXPR bool operator ()(const TBasicText<_Char, _Allocator>& lhs, const TBasicConstChar<_Char>& rhs) const NOEXCEPT { return lhs.Less(TBasicText<_Char>::MakeForeignText(rhs)); }
    template <typename _Allocator>
    CONSTEXPR bool operator ()(const TBasicConstChar<_Char>& lhs, const TBasicText<_Char, _Allocator>& rhs) const NOEXCEPT { return TBasicText<_Char>::MakeForeignText(lhs).Less(rhs); }
    template <typename _Allocator>
    CONSTEXPR bool operator ()(const TBasicText<_Char, _Allocator>& lhs, const TBasicStringView<_Char>& rhs) const NOEXCEPT { return lhs.MakeView().Less(rhs); }
    template <typename _Allocator>
    CONSTEXPR bool operator ()(const TBasicStringView<_Char>& lhs, const TBasicText<_Char, _Allocator>& rhs) const NOEXCEPT { return lhs.Less(rhs.MakeView()); }
};
template <typename _Char>
struct TTextLess<_Char, ECase::Insensitive> {
    template <typename _Allocator>
    CONSTEXPR bool operator ()(const TBasicText<_Char, _Allocator>& lhs, const TBasicText<_Char, _Allocator>& rhs) const NOEXCEPT { return lhs.LessI(rhs); }
    template <typename _Allocator>
    CONSTEXPR bool operator ()(const TBasicText<_Char, _Allocator>& lhs, const TBasicConstChar<_Char>& rhs) const NOEXCEPT { return lhs.LessI(TBasicText<_Char>::MakeForeignText(rhs)); }
    template <typename _Allocator>
    CONSTEXPR bool operator ()(const TBasicConstChar<_Char>& lhs, const TBasicText<_Char, _Allocator>& rhs) const NOEXCEPT { return TBasicText<_Char>::MakeForeignText(lhs).LessI(rhs); }
    template <typename _Allocator>
    CONSTEXPR bool operator ()(const TBasicText<_Char, _Allocator>& lhs, const TBasicStringView<_Char>& rhs) const NOEXCEPT { return LessI(lhs.MakeView(), rhs); }
    template <typename _Allocator>
    CONSTEXPR bool operator ()(const TBasicStringView<_Char>& lhs, const TBasicText<_Char, _Allocator>& rhs) const NOEXCEPT { return LessI(lhs, rhs.MakeView()); }
};
//----------------------------------------------------------------------------
template <typename _Char, ECase _Sensitive>
struct TTextHasher {
    template <typename _Allocator>
    CONSTEXPR hash_t operator ()(const TBasicText<_Char, _Allocator>& txt) const NOEXCEPT {
        return txt.HashValue();
    }
};
template <typename _Char>
struct TTextHasher<_Char, ECase::Insensitive> {
    template <typename _Allocator>
    CONSTEXPR hash_t operator ()(const TBasicText<_Char, _Allocator>& txt) const NOEXCEPT {
        return txt.HashValueI();
    }
};
//----------------------------------------------------------------------------
template <typename _CharA, typename _CharB, typename _Allocator>
TBasicTextWriter<_CharA>& operator <<(TBasicTextWriter<_CharA>& oss, const TBasicText<_CharB, _Allocator>& txt) {
    return oss << txt.MakeView();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
