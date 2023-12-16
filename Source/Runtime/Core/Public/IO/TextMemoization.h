#pragma once

#include "Core_fwd.h"

#include "Allocator/Allocation.h"
#include "Container/HashHelpers.h"
#include "Container/HashSet.h"
#include "IO/String_fwd.h"
#include "IO/Text.h"
#include "Meta/InPlace.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, ECase _Sensitive, typename _Allocator = ALLOCATOR(Text)>
class TBasicTextMemoization : private _Allocator {
public:
    using allocator_type = _Allocator;
    using text_type = TBasicText<_Char, allocator_type>;
    using hasher_type = TTextHasher<_Char, _Sensitive>;
    using equal_to_type = TTextEqualTo<_Char, _Sensitive>;
    using string_literal_type = TBasicStringLiteral<_Char>;
    using string_view_type = TBasicStringView<_Char>;

    STATIC_CONST_INTEGRAL(size_t, DefaultMaxLengthForMemoization, 200); // don't memoize large strings, as they are

    TBasicTextMemoization() NOEXCEPT = default;

    explicit TBasicTextMemoization(Meta::FForceInit) NOEXCEPT
    :   TBasicTextMemoization(Meta::MakeForceInit<allocator_type>())
    {}

    explicit TBasicTextMemoization(allocator_type&& allocator, size_t maxLengthForMemoization = DefaultMaxLengthForMemoization) NOEXCEPT
    :   allocator_type(std::move(allocator))
    ,   _set(Allocator_())
    ,   _maxLengthForMemoization(maxLengthForMemoization)
    {}
    explicit TBasicTextMemoization(const allocator_type& allocator, size_t maxLengthForMemoization = DefaultMaxLengthForMemoization) NOEXCEPT
    :   allocator_type(allocator)
    ,   _set(Allocator_())
    ,   _maxLengthForMemoization(maxLengthForMemoization)
    {}

    TBasicTextMemoization(TBasicTextMemoization&& rvalue) NOEXCEPT
    :   allocator_type(std::move(rvalue.Allocator_()))
    ,   _set(std::move(rvalue._set))
    {}
    TBasicTextMemoization& operator =(TBasicTextMemoization&&) = delete;

    TBasicTextMemoization(const TBasicTextMemoization&) = delete;
    TBasicTextMemoization& operator =(const TBasicTextMemoization&) = delete;

    ~TBasicTextMemoization() = default;

    NODISCARD size_t MaxLengthForMemoization() const { return _maxLengthForMemoization; }
    NODISCARD size_t NumMemoizedTexts() const { return _set.size(); }

    void SetMaxLengthForMemoization(size_t n) { _maxLengthForMemoization = n; }

    NODISCARD text_type Append(string_literal_type literal) const NOEXCEPT {
        return { literal }; // string literals are never copied
    }

    NODISCARD text_type Append(string_view_type str) {
        if (str.size() <= text_type::SmallCapacity_ ||
            str.size() > _maxLengthForMemoization)
            return {Allocator_(), str};

        const hash_t hash = TStringViewHasher<_Char, _Sensitive>{}(str);
        const auto[it, inserted] = _set.try_emplace_like(str, hash, Allocator_(), str);

        return text_type::MakeForeignText(it->ConstChar());
    }

    void clear_ReleaseMemory() {
        _set.clear_ReleaseMemory();
    }

private:
    allocator_type& Allocator_() NOEXCEPT { return (*this); }

    THashSet<
        text_type,
        hasher_type,
        equal_to_type,
        allocator_type>
        _set;

    size_t _maxLengthForMemoization = DefaultMaxLengthForMemoization;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
