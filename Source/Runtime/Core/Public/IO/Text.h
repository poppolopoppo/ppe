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
    using allocator_type = _Allocator;
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

    union {
        FInplaceString_ _inplace{};
        FExternalString_ _external;
    };

    template <typename _Char2, typename _Allocator2>
    friend class TBasicText;
    template <typename _Char2, ECase _Sensitive, typename _Allocator2>
    friend class TBasicTextMemoization;

public:
    STATIC_CONST_INTEGRAL(size_t, SmallCapacity, sizeof(FInplaceString_::Inline)/sizeof(_Char) - 1/* '\0' */);

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

    // this constructor must copy input and need to have a valid constructor, either user specified or default constructed
    TBasicText(TBasicConstChar<_Char> txt, allocator_type&& default_constructible_alloc = {}) : TBasicText(std::move(default_constructible_alloc)) { Assign(txt); }
    TBasicText(TBasicStringView<_Char> txt, allocator_type&& default_constructible_alloc = {}) : TBasicText(std::move(default_constructible_alloc)) { Assign(txt); }

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
    PPE_FAKEBOOL_OPERATOR_DECL() { return (not empty()); }

    bool empty() const NOEXCEPT { return (length() == 0); }

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
        if (txt.size() <= SmallCapacity)
            AssignSmall(txt);
        else
            AssignExternal(txt);
    }

    void Assign(TBasicStringLiteral<_Char> literal) {
        AssignLiteral(literal);
    }

    void AssignSmall(TBasicStringView<_Char> txt) NOEXCEPT {
        Assert(txt.size() <= SmallCapacity);
        Reset();

        _inplace.Source = Inplace;
        FPlatformMemory::Memcpy(&_inplace.Inline[0], txt.data(), txt.SizeInBytes());
    }

    void AssignExternal(TBasicStringView<_Char> txt) {
        Assert(txt.size() > SmallCapacity);

        Reset();

        const TMemoryView<_Char> cpy = allocator_traits::template AllocateT<_Char>(Allocator_(), txt.size() + 1/*'\0'*/);
        AssertReleaseMessage("text allocation failed", cpy.size() == txt.size() + 1);

        txt.CopyTo(cpy.ShiftBack());
        cpy.back() = STRING_LITERAL(_Char, '\0');

        _external = {
            .Source = External,
            .Length = checked_cast<u32>(txt.size()),
            .Storage = cpy.data(),
        };
    }

    void AssignLiteral(TBasicConstChar<_Char> const_char) NOEXCEPT {
        Reset();

        _external = {
            .Source = Literal,
            .Length = checked_cast<u32>(const_char.length()),
            .Storage = const_char.c_str(),
        };
    }

    void AssignLiteral(TBasicStringLiteral<_Char> literal) NOEXCEPT {
        Reset();

        _external = {
            .Source = Literal,
            .Length = checked_cast<u32>(literal.Length),
            .Storage = literal.Data,
        };
    }

    void Reset() {
        if (_inplace.Source == External) {
            const size_t len = length();
            Assert(len > SmallCapacity);
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

    NODISCARD CONSTF int Compare(const TBasicText& other) const NOEXCEPT { return FPlatformString::Cmp(c_str(), other.c_str()); }
    NODISCARD CONSTF int CompareI(const TBasicText& other) const NOEXCEPT { return FPlatformString::CmpI(c_str(), other.c_str()); }

    NODISCARD CONSTF bool Equals(const TBasicText& other) const NOEXCEPT {
        if (not IsInplace() && not other.IsInplace() && _external.Length != other._external.Length)
            return false;
        return (Compare(other) == 0);
    }
    NODISCARD CONSTF bool EqualsI(const TBasicText& other) const NOEXCEPT {
        if (not IsInplace() && not other.IsInplace() && _external.Length != other._external.Length)
            return false;
        return (CompareI(other) == 0);
    }

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

    friend bool operator ==(const TBasicText& lhs, const TBasicStringView<_Char>& rhs) NOEXCEPT { return PPE::Equals(lhs.MakeView(), rhs); }
    friend bool operator !=(const TBasicText& lhs, const TBasicStringView<_Char>& rhs) NOEXCEPT { return not operator ==(lhs, rhs); }

    friend bool operator < (const TBasicText& lhs, const TBasicStringView<_Char>& rhs) NOEXCEPT { return (PPE::Compare(lhs.MakeView(), rhs) < 0); }
    friend bool operator >=(const TBasicText& lhs, const TBasicStringView<_Char>& rhs) NOEXCEPT { return not operator < (lhs, rhs); }

    friend bool operator ==(const TBasicStringView<_Char>& lhs, const TBasicText& rhs) NOEXCEPT { return PPE::Equals(lhs, rhs.MakeView()); }
    friend bool operator !=(const TBasicStringView<_Char>& lhs, const TBasicText& rhs) NOEXCEPT { return not operator ==(lhs, rhs); }

    friend bool operator < (const TBasicStringView<_Char>& lhs, const TBasicText& rhs) NOEXCEPT { return (PPE::Compare(lhs, rhs.MakeView()) < 0); }
    friend bool operator >=(const TBasicStringView<_Char>& lhs, const TBasicText& rhs) NOEXCEPT { return not operator < (lhs, rhs); }

    friend bool operator ==(const TBasicText& lhs, const TBasicStringLiteral<_Char>& rhs) NOEXCEPT { return PPE::Equals(lhs.MakeView(), rhs.MakeView()); }
    friend bool operator !=(const TBasicText& lhs, const TBasicStringLiteral<_Char>& rhs) NOEXCEPT { return not operator ==(lhs, rhs); }

    friend bool operator < (const TBasicText& lhs, const TBasicStringLiteral<_Char>& rhs) NOEXCEPT { return (PPE::Compare(lhs.MakeView(), rhs.MakeView()) < 0); }
    friend bool operator >=(const TBasicText& lhs, const TBasicStringLiteral<_Char>& rhs) NOEXCEPT { return not operator < (lhs, rhs); }

    friend bool operator ==(const TBasicStringLiteral<_Char>& lhs, const TBasicText& rhs) NOEXCEPT { return PPE::Equals(lhs.MakeView(), rhs.MakeView()); }
    friend bool operator !=(const TBasicStringLiteral<_Char>& lhs, const TBasicText& rhs) NOEXCEPT { return not operator ==(lhs, rhs); }

    friend bool operator < (const TBasicStringLiteral<_Char>& lhs, const TBasicText& rhs) NOEXCEPT { return (PPE::Compare(lhs.MakeView(), rhs.MakeView()) < 0); }
    friend bool operator >=(const TBasicStringLiteral<_Char>& lhs, const TBasicText& rhs) NOEXCEPT { return not operator < (lhs, rhs); }

    NODISCARD bool AcquireDataUnsafe(FAllocatorBlock b, size_t sz) NOEXCEPT {
        if (allocator_traits::Acquire(*this, b)) {
            Reset();

            _external = {
                .Source = External,
                .Length = checked_cast<u32>(sz),
                .Storage = static_cast<const _Char*>(b.Data),
            };
            return true;
        }
        return false;
    }
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
