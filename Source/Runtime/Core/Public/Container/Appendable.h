#pragma once

#include "Core_fwd.h"

#include "Misc/Function.h"

namespace PPE {
template <typename T, bool _IsPod>
class TStack;
template <typename T, typename _Allocator>
class TSparseArray;
template <typename T, typename _Allocator>
class TVector;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class TAppendable {
public:
    typedef T value_type;

    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;

    using FPushBackFunc = TFunctionRef<void(T&&)>;

    TAppendable() = default;

    CONSTEXPR TAppendable(Meta::FDefaultValue) NOEXCEPT
    :   TAppendable([](T&&) CONSTEXPR NOEXCEPT {})
    {}

    CONSTEXPR TAppendable(FPushBackFunc&& pushBack) NOEXCEPT
    :   _pushBack(std::move(pushBack)) {
        Assert_NoAssume(_pushBack.Valid());
    }

    TAppendable(const TAppendable&) = default;
    TAppendable& operator =(const TAppendable&) = default;

    TAppendable(TAppendable&&) = default;
    TAppendable& operator =(TAppendable&&) = default;

    CONSTEXPR void push_back(const T& value) const { _pushBack(T(value)); }
    CONSTEXPR void push_back(T&& rvalue) const { _pushBack(std::move(rvalue)); }

    template <typename... _Args, class = Meta::TEnableIf<std::is_constructible_v<T, _Args&&...>> >
    CONSTEXPR void emplace_back(_Args&&... args) const { push_back(T{ std::forward<_Args>(args)... }); }

    template <typename _It>
    CONSTEXPR void insert(_It first, _It last) {
        forrange(it, first, last) {
            emplace_back(*it);
        }
    }

    friend CONSTEXPR const TAppendable& operator <<(const TAppendable& appendable, const T& value) {
        appendable.push_back(value);
        return appendable;
    }
    friend CONSTEXPR const TAppendable& operator <<(const TAppendable& appendable, T&& rvalue) {
        appendable.push_back(std::move(rvalue));
        return appendable;
    }

    friend CONSTEXPR bool operator ==(const TAppendable& lhs, const TAppendable& rhs) {
        return (lhs._userData == rhs._userData && lhs._pushBack == rhs._pushBack);
    }
    friend CONSTEXPR bool operator !=(const TAppendable& lhs, const TAppendable& rhs) {
        return (not operator ==(lhs, rhs));
    }

    friend CONSTEXPR void swap(TAppendable& lhs, TAppendable& rhs) NOEXCEPT {
        std::swap(lhs._userData, rhs._userData);
        std::swap(lhs._pushBack, rhs._pushBack);
    }

private:
    FPushBackFunc _pushBack{};
};
//----------------------------------------------------------------------------
template <typename _Char>
TAppendable<_Char> MakeAppendable(TBasicString<_Char>& str) {
    return typename TAppendable<_Char>::FPushBackFunc( Meta::StaticFunction<&TBasicString<_Char>::push_back>, &str );
}
//----------------------------------------------------------------------------
template <typename T, bool _IsPod>
TAppendable<T> MakeAppendable(TStack<T, _IsPod>& stack) {
    return typename TAppendable<T>::FPushBackFunc( Meta::StaticFunction<&TStack<T, _IsPod>::template Push<T&&> >, &stack );
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
TAppendable<T> MakeAppendable(TSparseArray<T, _Allocator>& sparse) {
    return typename TAppendable<T>::FPushBackFunc( Meta::StaticFunction<&TSparseArray<T, _Allocator>::template Emplace<T&&> >, &sparse );
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
TAppendable<T> MakeAppendable(TVector<T, _Allocator>& vector) {
    return typename TAppendable<T>::FPushBackFunc( Meta::StaticFunction<&TVector<T, _Allocator>::template emplace_back<T&&> >, &vector );
}
//----------------------------------------------------------------------------
template <typename T, typename _Lambda,
    decltype(typename TAppendable<T>::FPushBackFunc{ std::declval<_Lambda&>() })* = nullptr >
TAppendable<T> MakeAppendable(_Lambda& lambda) {
    return typename TAppendable<T>::FPushBackFunc( lambda );
}
//----------------------------------------------------------------------------
template <typename T, typename _Lambda,
    decltype(std::declval<const _Lambda&>()(std::declval<T&&>()))* = nullptr >
TAppendable<T> MakeAppendable(const _Lambda& lambda) {
    return typename TAppendable<T>::FPushBackFunc( lambda );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
