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

    using FPushBackFunc = void (*)(void*, T&&);

    TAppendable() = default;

    CONSTEXPR TAppendable(FNoFunction)
    :   TAppendable(nullptr, [](void*, T&&) NOEXCEPT {})
    {}

    CONSTEXPR TAppendable(void* userData, FPushBackFunc pushBack)
    :   _userData(userData)
    ,   _pushBack(pushBack) {
        Assert_NoAssume(_pushBack);
    }

    TAppendable(const TAppendable&) = default;
    TAppendable& operator =(const TAppendable&) = default;

    TAppendable(TAppendable&&) = default;
    TAppendable& operator =(TAppendable&&) = default;

    CONSTEXPR void* UserData() const { return _userData; }

    CONSTEXPR void push_back(const T& value) const { _pushBack(_userData, T(value)); }
    CONSTEXPR void push_back(T&& rvalue) const { _pushBack(_userData, std::move(rvalue)); }

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
    void* _userData{ nullptr };
    FPushBackFunc _pushBack{ nullptr };
};
//----------------------------------------------------------------------------
template <typename _Char>
TAppendable<_Char> MakeAppendable(TBasicString<_Char>& str) {
    return { &str, [](void* userData, _Char&& rvalue) NOEXCEPT {
        static_cast<TBasicString<_Char>*>(userData)->push_back(std::move(rvalue));
    } };
}
//----------------------------------------------------------------------------
template <typename T, bool _IsPod>
TAppendable<T> MakeAppendable(TStack<T, _IsPod>& stack) {
    return { &stack, [](void* userData, T&& rvalue) NOEXCEPT {
        static_cast<TStack<T, _IsPod>*>(userData)->Push(std::move(rvalue));
    } };
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
TAppendable<T> MakeAppendable(TSparseArray<T, _Allocator>& sparse) {
    return { &sparse, [](void* userData, T&& rvalue) NOEXCEPT {
        static_cast<TSparseArray<T, _Allocator>*>(userData)->Emplace(std::move(rvalue));
    } };
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
TAppendable<T> MakeAppendable(TVector<T, _Allocator>& vector) {
    return { &vector, [](void* userData, T&& rvalue) NOEXCEPT {
        static_cast<TVector<T, _Allocator>*>(userData)->push_back(std::move(rvalue));
    } };
}
//----------------------------------------------------------------------------
template <typename T, typename _Lambda,
    decltype(std::declval<_Lambda&>()(std::declval<T&&>()))* = nullptr >
TAppendable<T> MakeAppendable(_Lambda& lambda) {
    return { &lambda, [](void* userData, T&& rvalue) NOEXCEPT {
        (*static_cast<_Lambda*>(userData))(std::move(rvalue));
    } };
}
//----------------------------------------------------------------------------
template <typename T, typename _Lambda,
    decltype(std::declval<const _Lambda&>()(std::declval<T&&>()))* = nullptr >
TAppendable<T> MakeAppendable(const _Lambda& lambda) {
    return { const_cast<_Lambda*>(&lambda), [](void* userData, T&& rvalue) NOEXCEPT {
        (*static_cast<const _Lambda*>(userData))(std::move(rvalue));
    } };
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
