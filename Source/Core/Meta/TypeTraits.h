#pragma once

#include <type_traits>

#define CORE_ASSUME_TYPE_AS_POD(T, ...) \
    namespace Meta { \
        template < __VA_ARGS__ > \
        struct TIsPod< T > : public std::integral_constant<bool, true> {}; \
    } //!Meta
#define CORE_ASSERT_TYPE_IS_POD(T, ...) \
    static_assert(::Core::Meta::TIsPod<T>::value, STRINGIZE(T) " is not a POD type");

namespace Core {
namespace Meta {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, T _Value>
using TIntegralConstant = typename std::integral_constant<T, _Value>::type;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
using TGreater = std::greater<T>;
//----------------------------------------------------------------------------
template <typename T>
using TGreaterEqual = std::greater_equal<T>;
//----------------------------------------------------------------------------
template <typename T>
using TLess = std::less<T>;
//----------------------------------------------------------------------------
template <typename T>
using TLessEqual = std::less_equal<T>;
//----------------------------------------------------------------------------
template <typename T>
using TEqualTo = std::equal_to<T>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
using TDecay = typename std::decay<T>::type;
//----------------------------------------------------------------------------
template <typename T>
using TAddConst = typename std::add_const<T>::type;
//----------------------------------------------------------------------------
template <typename T, bool _Const>
using TAddConstIFN = typename std::conditional<_Const, TAddConst<T>, T>::type;
//----------------------------------------------------------------------------
template <typename T>
using TRemoveConst = typename std::remove_const<T>::type;
//----------------------------------------------------------------------------
template <typename T>
using TAddReference = typename std::add_lvalue_reference<T>::type;
//----------------------------------------------------------------------------
template <typename T>
using TRemoveReference = typename std::remove_reference<T>::type;
//----------------------------------------------------------------------------
template <typename T>
using TAddPointer = typename std::add_pointer<T>::type;
//----------------------------------------------------------------------------
template <typename T>
using TRemovePointer = typename std::remove_pointer<T>::type;
//----------------------------------------------------------------------------
template <bool _Test, typename T = void>
using TEnableIf = typename std::enable_if< _Test, T >::type;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Replaces std::is_pod<>, can be overrided for specific types
//----------------------------------------------------------------------------
template <typename T>
struct TIsPod : public std::is_pod<T> {
    using std::is_pod<T>::value;
    using typename std::is_pod<T>::value_type;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// TODO: use C++17 fold expressions when available
// http://stackoverflow.com/questions/17339789/how-to-call-a-function-on-all-variadic-template-args?lq=1
namespace details {
struct FFoldExpression {
    template <typename... T>
    FFoldExpression(T&&...) {}
};
} //!details
//----------------------------------------------------------------------------
#define FOLD_EXPR(...) \
    Core::Meta::details::FFoldExpression{ 0, ((__VA_ARGS__), void(), 0)... }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if 0
// Uses SFINAE to determine if T has a compatible constructor
namespace details {
template<
    class T,
    typename... _Args,
    class = decltype(T(std::declval<_Args>()...))
>   std::true_type  _has_constructor(T&&, _Args&&... args);
    std::false_type _has_constructor(...);
} //!details
template <typename T, typename... _Args>
struct has_constructor : decltype(
    details::_has_constructor(
        std::declval<T>(),
        std::declval<_Args>()...
    )) {};
#else
template <typename T, typename... _Args>
using has_constructor = typename std::is_constructible<T, _Args...>::type;
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Very interesting, but did not work everywhere :'(
// http://stackoverflow.com/questions/10711952/how-to-detect-existence-of-a-class-using-sfinae
//----------------------------------------------------------------------------
/*! The template `has_destructor<T>` exports a
boolean constant `value that is true iff `T` has
a public destructor.

N.B. A compile error will occur if T has non-public destructor.
*/
template< typename T>
struct has_destructor {
    /* Has destructor :) */
    template <typename A>
    static std::true_type test(decltype(std::declval<A>().~A()) *) {
        return std::true_type();
    }

    /* Has no destructor :( */
    template<typename A>
    static std::false_type test(...) {
        return std::false_type();
    }

    /* This will be either `std::true_type` or `std::false_type` */
    typedef decltype(test<T>(0)) type;

    static constexpr bool value = type::value; /* Which is it? */
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FNoInit {};
struct FForceInit {};
//----------------------------------------------------------------------------
template <typename T>
using has_forceinit_constructor = has_constructor<T, Meta::FForceInit>;
//----------------------------------------------------------------------------
namespace details {
template <typename T>
FORCE_INLINE void Construct_(T* p, FForceInit, std::false_type) { new ((void*)p) T(); }
template <typename T>
FORCE_INLINE void Construct_(T* p, FForceInit init, std::true_type) { new ((void*)p) T(init); }
} //!details
//----------------------------------------------------------------------------
template <typename T>
void Construct(T* p, FForceInit init) {
    Likely(p);
    details::Construct_(p, init, typename has_forceinit_constructor<T>::type{});
}
//----------------------------------------------------------------------------
template <typename T>
void Construct(T* p) {
    Likely(p);
    ::new ((void*)p) T();
}
//----------------------------------------------------------------------------
template <typename T>
void Construct(T* p, T&& rvalue) {
    Likely(p);
    ::new ((void*)p) T(std::move(rvalue));
}
//----------------------------------------------------------------------------
template <typename T>
void Construct(T* p, const T& other) {
    Likely(p);
    ::new ((void*)p) T(other);
}
//----------------------------------------------------------------------------
template <typename T, typename... _Args>
void Construct(T* p, _Args&&... args) {
    Likely(p);
    ::new((void*)p) T{ std::forward<_Args>(args)... };
}
//----------------------------------------------------------------------------
template <typename T>
typename std::enable_if<Meta::TIsPod<T>::value >::type Destroy(T* ) {
    // PODs don't have a destructor
}
//----------------------------------------------------------------------------
template <typename T>
typename std::enable_if< not Meta::TIsPod<T>::value >::type Destroy(T* p) {
    Likely(p);
    typedef char type_must_be_complete[sizeof(T) ? 1 : -1];
    (void) sizeof(type_must_be_complete);
    p->~T();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define INSTANTIATE_CLASS_TYPEDEF(_NAME, ...) \
    class _NAME : public __VA_ARGS__ { \
        typedef __VA_ARGS__ parent_type; \
    public: \
        using parent_type::parent_type; \
        using parent_type::operator=; \
        \
        _NAME() = default; \
        \
        _NAME(const _NAME& ) = default; \
        _NAME& operator =(const _NAME& ) = default; \
        \
        _NAME(_NAME&& ) = default; \
        _NAME& operator =(_NAME&& ) = default; \
        \
        template < \
            typename... _Args, \
            typename = typename std:: < has_constructor<parent_type, _Args...>::value > \
        > \
        explicit _NAME(_Args&&... args) \
            : parent_type(std::forward<_Args>(args)...) {} \
        \
        template <typename _Arg> \
        _NAME& operator=(_Arg&& arg) { \
            parent_type::operator =(std::forward<_Arg>(arg)); \
            return *this; \
        } \
    }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace Core
