#pragma once

#include <functional>
#include <type_traits>

#define PPE_ASSUME_TYPE_AS_POD(T, ...) \
    namespace Meta { \
        template < __VA_ARGS__ > \
        struct TIsPod< T > : public std::integral_constant<bool, true> {}; \
    } //!Meta
#define PPE_ASSERT_TYPE_IS_POD(T, ...) \
    static_assert(::PPE::Meta::TIsPod<T>::value, STRINGIZE(T) " is not a POD type");

namespace PPE {
namespace Meta {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
using TArray = T[_Dim];
//----------------------------------------------------------------------------
template <typename T, T _Value>
using TIntegralConstant = typename std::integral_constant<T, _Value>::type;
//----------------------------------------------------------------------------
// Function overloading helpers to use dummy parameters for specialization :
template <typename T>
struct TType {};
//----------------------------------------------------------------------------
// This ugly piece of crap is just used in STATIC_ASSERT() to get to see the
// actual size of the types when the assertion fails :
template <typename A, typename B>
struct TCheckSameSize {
    template <size_t X, size_t Y>
    struct TTest {
        STATIC_CONST_INTEGRAL(bool, value, (X == Y));
        static_assert(value, "A and B should have the same size !");
    };
    STATIC_CONST_INTEGRAL(bool, value, TTest<sizeof(A), sizeof(B)>::value);
};
template <typename _Size, typename _Capacity>
struct TCheckFitInSize {
    template <size_t X, size_t Y>
    struct TTest {
        STATIC_CONST_INTEGRAL(bool, value, (X <= Y));
        static_assert(value, "_Size should be inferior or equal to _Capacity !");
    };
    STATIC_CONST_INTEGRAL(bool, value, TTest<sizeof(_Size), sizeof(_Capacity)>::value);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Unlike STL variants theses operators allow to compare _Lhs with a compatible _Rhs
//----------------------------------------------------------------------------
#define PPE_META_OPERATOR(_NAME, _OP) \
    template <typename _Lhs> \
    struct _NAME { \
        template <typename _Rhs> \
        CONSTEXPR bool operator()(const _Lhs& lhs, const _Rhs& rhs) const { \
            return (rhs _OP lhs); \
        } \
    }
//----------------------------------------------------------------------------
PPE_META_OPERATOR(TGreater,        > );
PPE_META_OPERATOR(TGreaterEqual,   >=);
PPE_META_OPERATOR(TLess,           < );
PPE_META_OPERATOR(TLessEqual,      <=);
PPE_META_OPERATOR(TEqualTo,        ==);
//----------------------------------------------------------------------------
#undef PPE_META_OPERATOR
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
template <bool _Test, typename _True, typename _False>
using TConditional = typename std::conditional< _Test, _True, _False >::type;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
using TPointer = TAddPointer< TRemoveConst< TDecay<T> > >;
//----------------------------------------------------------------------------
template <typename T>
using TConstPointer = TAddPointer< TAddConst< TDecay<T> > >;
//----------------------------------------------------------------------------
template <typename T>
using TReference = TAddReference< TRemoveConst< TDecay<T> > >;
//----------------------------------------------------------------------------
template <typename T>
using TConstReference = TAddReference< TAddConst< TDecay<T> > >;
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
template <typename... _Args>
constexpr bool TIsPod_v{ (TIsPod<_Args>::value && ...) };
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
    static CONSTEXPR std::true_type test(decltype(std::declval<A>().~A()) *) {
        return std::true_type();
    }

    /* Has no destructor :( */
    template<typename A>
    static CONSTEXPR std::false_type test(...) {
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
inline constexpr FNoInit NoInit{};
inline constexpr FForceInit ForceInit{};
//----------------------------------------------------------------------------
template <typename T>
using has_noinit_constructor = has_constructor<T, Meta::FNoInit>;
template <typename T>
using has_forceinit_constructor = has_constructor<T, Meta::FForceInit>;
//----------------------------------------------------------------------------
namespace details {
template <typename T>
FORCE_INLINE CONSTEXPR T ForceInit_(std::false_type) { return T{}; }
template <typename T>
FORCE_INLINE CONSTEXPR T ForceInit_(std::true_type) { return T{ ForceInit }; }
} //!details
// can be overload for custom types
template <typename T>
T ForceInitType(TType<T>) {
    return details::ForceInit_<T>(typename has_forceinit_constructor<T>::type{});
}
// simpler interface wrapping overloadable ForceInitType()
template <typename T>
T MakeForceInit() {
    return ForceInitType(TType<T>{});
}
//----------------------------------------------------------------------------
namespace details {
template <typename T>
FORCE_INLINE CONSTEXPR T NoInit_(std::false_type) { return T{}; }
template <typename T>
FORCE_INLINE CONSTEXPR T NoInit_(std::true_type) { return T{ NoInit }; }
} //!details
// can be overload for custom types
template <typename T>
T NoInitType(TType<T>) {
    return details::NoInit_<T>(typename has_noinit_constructor<T>::type{});
}
// simpler interface wrapping overloadable NoInitType()
template <typename T>
T MakeNoInit() {
    return NoInitType(TType<T>{});
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
template <typename T>
FORCE_INLINE void Construct_(T* p, FNoInit, std::false_type) { new ((void*)p) T{}; }
template <typename T>
FORCE_INLINE void Construct_(T* p, FNoInit noInit, std::true_type) { new ((void*)p) T{ noInit }; }
} //!details
template <typename T>
void Construct(T* p, FNoInit noInit) {
    Assume(p);
    details::Construct_(p, noInit, typename has_noinit_constructor<T>::type{});
}
//----------------------------------------------------------------------------
namespace details {
template <typename T>
FORCE_INLINE void Construct_(T* p, FForceInit, std::false_type) { new ((void*)p) T{}; }
template <typename T>
FORCE_INLINE void Construct_(T* p, FForceInit init, std::true_type) { new ((void*)p) T{ init }; }
} //!details
template <typename T>
void Construct(T* p, FForceInit init) {
    Assume(p);
    details::Construct_(p, init, typename has_forceinit_constructor<T>::type{});
}
//----------------------------------------------------------------------------
template <typename T>
void Construct(T* p) {
    Assume(p);
    INPLACE_NEW(p, T)();
}
//----------------------------------------------------------------------------
template <typename T>
void Construct(T* p, T&& rvalue) {
    Assume(p);
    INPLACE_NEW(p, T)(std::move(rvalue));
}
//----------------------------------------------------------------------------
template <typename T>
void Construct(T* p, const T& other) {
    Assume(p);
    INPLACE_NEW(p, T)(other);
}
//----------------------------------------------------------------------------
template <typename T, typename... _Args>
void Construct(T* p, _Args&&... args) {
    Assume(p);
    INPLACE_NEW(p, T){ std::forward<_Args>(args)... };
}
//----------------------------------------------------------------------------
template <typename T>
typename std::enable_if<Meta::TIsPod<T>::value >::type Destroy(T* ) {
    // PODs don't have a destructor
    typedef char type_must_be_complete[sizeof(T) ? 1 : -1];
    (void) sizeof(type_must_be_complete);
}
//----------------------------------------------------------------------------
template <typename T>
typename std::enable_if< not Meta::TIsPod<T>::value >::type Destroy(T* p) {
    Assume(p);
    typedef char type_must_be_complete[sizeof(T) ? 1 : -1];
    (void) sizeof(type_must_be_complete);
    p->~T();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define INSTANTIATE_CLASS_TYPEDEF(_API, _NAME, ...) \
    class _API _NAME : public __VA_ARGS__ { \
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
            typename = PPE::Meta::TEnableIf< Meta::has_constructor<parent_type, _Args...>::value > \
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
} //!namespace PPE
