#pragma once

#include "Meta/Aliases.h"
#include "Meta/Config.h"

#include <functional>
#include <type_traits>

#if USE_PPE_MEMORY_DEBUGGING
#   define USE_PPE_MEMORY_CHECK_INITIALIZATION  (1) // %_NOCOMMIT%
#   define USE_PPE_MEMORY_CHECK_NECROPHILIA     (1) // %_NOCOMMIT%
#else
#   define USE_PPE_MEMORY_CHECK_INITIALIZATION  (USE_PPE_ASSERT) // %_NOCOMMIT%
#   define USE_PPE_MEMORY_CHECK_NECROPHILIA     (USE_PPE_ASSERT) // %_NOCOMMIT%
#endif

// Can override POD detection in PPE codebase, respects ADL
#define PPE_ASSERT_TYPE_IS_POD(T) \
    static_assert(::PPE::Meta::is_pod_v<T>, STRINGIZE(T) " is not a POD type");
#define PPE_ASSUME_TYPE_AS_POD(...) \
    CONSTEXPR bool is_pod_type(__VA_ARGS__*) NOEXCEPT { return true; }
#define PPE_ASSUME_FRIEND_AS_POD(...) \
    friend PPE_ASSUME_TYPE_AS_POD(__VA_ARGS__)
#define PPE_ASSUME_TEMPLATE_AS_POD(T, ...) \
    template <__VA_ARGS__> PPE_ASSUME_TYPE_AS_POD(T)
#define PPE_ASSUME_TEMPLATE_FRIEND_AS_POD(T, ...) \
    template <__VA_ARGS__> PPE_ASSUME_FRIEND_AS_POD(T)

// Can override POINTER detection in PPE codebase, respects ADL
#define PPE_ASSERT_TYPE_IS_POINTER(T) \
    static_assert(::PPE::Meta::is_pointer_v<T>, STRINGIZE(T) " is not a POINTER type");
#define PPE_ASSUME_TYPE_AS_POINTER(...) \
    CONSTEXPR bool is_pointer_type(__VA_ARGS__*) NOEXCEPT { return true; }
#define PPE_ASSUME_FRIEND_AS_POINTER(...) \
    friend PPE_ASSUME_TYPE_AS_POINTER(__VA_ARGS__)
#define PPE_ASSUME_TEMPLATE_AS_POINTER(T, ...) \
    template <__VA_ARGS__> PPE_ASSUME_TYPE_AS_POINTER(T)
#define PPE_ASSUME_TEMPLATE_FRIEND_AS_POINTER(T, ...) \
    template <__VA_ARGS__> PPE_ASSUME_FRIEND_AS_POINTER(T)

namespace PPE {
namespace Meta {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FNonCopyable {
    FNonCopyable() NOEXCEPT = default;
    FNonCopyable(const FNonCopyable&) = delete;
    FNonCopyable& operator =(const FNonCopyable&) = delete;
    FNonCopyable(FNonCopyable&&) = default;
    FNonCopyable& operator =(FNonCopyable&&) = default;
};
//----------------------------------------------------------------------------
struct FNonMovable {
    FNonMovable() NOEXCEPT = default;
    FNonMovable(const FNonMovable&) = default;
    FNonMovable& operator =(const FNonMovable&) = default;
    FNonMovable(FNonMovable&&) = delete;
    FNonMovable& operator =(FNonMovable&&) = delete;
};
//----------------------------------------------------------------------------
struct FNonCopyableNorMovable {
    FNonCopyableNorMovable() NOEXCEPT = default;
    FNonCopyableNorMovable(const FNonCopyableNorMovable&) = delete;
    FNonCopyableNorMovable& operator =(const FNonCopyableNorMovable&) = delete;
    FNonCopyableNorMovable(FNonCopyableNorMovable&&) = delete;
    FNonCopyableNorMovable& operator =(FNonCopyableNorMovable&&) = delete;
};
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
struct TType final { using type = T; };
template <typename T>
CONSTEXPR const TType<T> Type{};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
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
template <typename A, size_t _Align2>
struct TCheckSizeAligned {
    template <size_t _Sz, size_t _Align>
    struct TTest {
        STATIC_CONST_INTEGRAL(bool, value, (_Sz % _Align) == 0);
        static_assert(value, "_Sz should be align on _Align !");
    };
    STATIC_CONST_INTEGRAL(bool, value, TTest<sizeof(A), _Align2>::value);
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
// Type semantics
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
// Functional operators
//----------------------------------------------------------------------------
template <typename T = void >
using TPlus = std::plus<T>;
//----------------------------------------------------------------------------
template <typename T = void >
using TMinus = std::minus<T>;
//----------------------------------------------------------------------------
template <typename T = void >
using TMultiplies = std::multiplies<T>;
//----------------------------------------------------------------------------
template <typename T = void >
using TDivides = std::divides<T>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// is_pod_type(T*) can be overloaded for specific types, respects ADL
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR bool is_pod_type(T*) NOEXCEPT { return std::is_trivial_v<T> && std::is_standard_layout_v<T>; }
template <typename T>
CONSTEXPR bool is_pod_v{ is_pod_type(static_cast<T*>(nullptr)) };
//----------------------------------------------------------------------------
template <typename T>
using has_trivial_constructor = std::bool_constant<std::is_trivially_constructible<T>::value || is_pod_v<T> >;
//----------------------------------------------------------------------------
template <typename T>
using has_trivial_destructor = std::bool_constant<std::is_trivially_destructible<T>::value || is_pod_v<T> >;
//----------------------------------------------------------------------------
template <typename T>
using has_trivial_copy = std::bool_constant<(std::is_trivially_copy_constructible_v<T> && std::is_trivially_copy_assignable_v<T>) ||
    is_pod_v<T> >;
//----------------------------------------------------------------------------
template <typename T>
using has_trivial_move = std::bool_constant<(std::is_trivially_move_constructible<T>::value && std::is_trivially_destructible<T>::value) ||
    is_pod_v<T> >;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// is_pointer_type(T*) can be overloaded for specific types, respects ADL
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR bool is_pointer_type(T*) NOEXCEPT { return std::is_pointer_v<T>; }
template <typename T>
CONSTEXPR bool is_pointer_v{ is_pointer_type(static_cast<T*>(nullptr)) };
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Uses SFINAE to determine if T has a compatible constructor
//----------------------------------------------------------------------------

template <typename T, typename... _Args>
using has_constructor = typename std::is_constructible<T, _Args...>::type;
template <typename T>
using has_default_constructor = typename std::is_constructible<T>::type;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// SFINAE for detecting if a type implements an operator ==
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
struct no_equals_t_ {};
template <typename U, typename V>
no_equals_t_ operator ==(const U&, const V&); // dummy for SFINAE
template <typename U, typename V = U>
using has_equals_ = std::bool_constant<
    not std::is_same_v<
        no_equals_t_,
        decltype(std::declval<const U&>() == std::declval<const V&>())
    >
>;
//----------------------------------------------------------------------------
} //!details
//----------------------------------------------------------------------------
template <typename U, typename V = U>
CONSTEXPR bool has_equals_v = details::has_equals_<U, V>::value;
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

    static CONSTEXPR bool value = type::value; /* Which is it? */
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Test is a class/struct is defining a using/function/typedef/value/enum ...
// Ex:  // test for a using/typedef
//      template <typename T> using_value_type_t = typename T::value_type;
//      STATIC_ASSERT(Meta::has_defined_v<using_value_type_t, my_type_t>);
//----------------------------------------------------------------------------
namespace details {
template <template<class> class _Using, typename T, class = _Using<T>>
std::true_type has_defined_(int);
template <template<class> class _Using, typename T>
std::false_type has_defined_(...);
template <typename T>
using get_type_t_ = typename T::type;
} //!details
//----------------------------------------------------------------------------
template <template<class> class _Using, typename T>
using has_defined_t = decltype(details::has_defined_<_Using, T>(0));
//----------------------------------------------------------------------------
template <template<class> class _Using, typename T>
CONSTEXPR bool has_defined_v = has_defined_t<_Using, T>::value;
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR bool has_type_v = has_defined_t<details::get_type_t_, T>::value;
//----------------------------------------------------------------------------
template <typename U, typename V>
CONSTEXPR bool has_common_type_v = has_type_v<std::common_type<U, V>>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
template <template<class> class _Using, class _Fallback, typename T>
_Using<T> optional_definition_(int);
template <template<class> class _Using, class _Fallback, typename T>
_Fallback optional_definition_(...);
} //!details
//----------------------------------------------------------------------------
template <template<class> class _Using, class _Fallback, typename T>
using optional_definition_t = decltype(details::optional_definition_<_Using, _Fallback, T>(0));
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Optional control for data initialization
//----------------------------------------------------------------------------
struct FNoInit final {}; // ctor with that arg must *NOT* initialize the data
struct FForceInit final {}; // ctor with that arg must initialize the data
//----------------------------------------------------------------------------
CONSTEXPR FNoInit NoInit{};
CONSTEXPR FForceInit ForceInit{};
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
CONSTEXPR T ForceInitType(TType<T>) NOEXCEPT {
    return details::ForceInit_<T>(typename has_forceinit_constructor<T>::type{});
}
// simpler interface wrapping overloadable ForceInitType()
template <typename T>
CONSTEXPR T MakeForceInit() NOEXCEPT {
    return ForceInitType(Type<T>);
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
CONSTEXPR T NoInitType(TType<T>) NOEXCEPT {
    return details::NoInit_<T>(typename has_noinit_constructor<T>::type{});
}
// simpler interface wrapping overloadable NoInitType()
template <typename T>
CONSTEXPR T MakeNoInit() NOEXCEPT {
    return NoInitType(Type<T>);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Tricking the compiler to make implicit conversion with template argument deduction
// https://stackoverflow.com/questions/45765205/template-function-argument-deduction-with-an-implicit-conversion
template<class T>
using TDontDeduce = typename TType<T>::type;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
template <typename T>
FORCE_INLINE void Construct_(T* p, FNoInit, std::false_type) { INPLACE_NEW(p, T){}; }
template <typename T>
FORCE_INLINE void Construct_(T* p, FNoInit noInit, std::true_type) { INPLACE_NEW(p, T){ noInit }; }
} //!details
template <typename T>
void Construct(T* p, FNoInit noInit) {
    Assume(p);
    details::Construct_(p, noInit, typename has_noinit_constructor<T>::type{});
}
//----------------------------------------------------------------------------
namespace details {
template <typename T>
FORCE_INLINE void Construct_(T* p, FForceInit, std::false_type) { INPLACE_NEW(p, T){}; }
template <typename T>
FORCE_INLINE void Construct_(T* p, FForceInit init, std::true_type) { INPLACE_NEW(p, T){ init }; }
} //!details
template <typename T>
void Construct(T* p, FForceInit init) {
    Assume(p);
#if USE_PPE_MEMORY_DEBUGGING
    ::memset(p, 0xCC, sizeof(T)); // uninitialized field detection
#endif
    details::Construct_(p, init, typename has_forceinit_constructor<T>::type{});
}
//----------------------------------------------------------------------------
template <typename T>
void Construct(T* p) {
    Assume(p);
    typedef char type_must_be_complete[sizeof(T) ? 1 : -1];
    (void) sizeof(type_must_be_complete);
#if USE_PPE_MEMORY_CHECK_INITIALIZATION
    ::memset(p, 0xCC, sizeof(T)); // uninitialized field detection
#endif
    INPLACE_NEW(p, T)();
}
//----------------------------------------------------------------------------
template <typename T>
void Construct(T* p, T&& rvalue) {
    Assume(p);
#if USE_PPE_MEMORY_CHECK_INITIALIZATION
    ::memset(p, 0xCC, sizeof(T)); // uninitialized field detection
#endif
    INPLACE_NEW(p, T)(std::move(rvalue));
}
//----------------------------------------------------------------------------
template <typename T>
void Construct(T* p, const T& other) {
    Assume(p);
#if USE_PPE_MEMORY_CHECK_INITIALIZATION
    ::memset(p, 0xCC, sizeof(T)); // uninitialized field detection
#endif
    INPLACE_NEW(p, T)(other);
}
//----------------------------------------------------------------------------
template <typename T, typename... _Args>
void Construct(T* p, _Args&&... args) {
    Assume(p);
#if USE_PPE_MEMORY_CHECK_INITIALIZATION
    ::memset(p, 0xCC, sizeof(T)); // uninitialized field detection
#endif
    INPLACE_NEW(p, T){ std::forward<_Args>(args)... };
}
//----------------------------------------------------------------------------
template <typename T>
Meta::TEnableIf<not std::is_const_v<T>> Destroy(T* p) NOEXCEPT {
    Assume(p);
    typedef char type_must_be_complete[sizeof(T) ? 1 : -1];
    (void) sizeof(type_must_be_complete);
    p->~T();
#if USE_PPE_MEMORY_CHECK_NECROPHILIA
    ::memset(p, 0xDD, sizeof(T)); // necrophilia detection
#endif
}
//----------------------------------------------------------------------------
template <typename T = void>
struct TDefaultConstructor {
    void operator ()(T* p) const NOEXCEPT {
        Meta::Construct(p, ForceInit);
    }
};
template <>
struct TDefaultConstructor<void> {
    template <typename T>
    void operator ()(T* p) const NOEXCEPT {
        Meta::Construct(p, ForceInit);
    }
};
template <typename T>
constexpr TDefaultConstructor<T> DefaultConstructor;
//----------------------------------------------------------------------------
template <typename T = void>
struct TDestructor {
    void operator ()(T* p) const NOEXCEPT {
        Meta::Destroy(p);
    }
};
template <>
struct TDestructor<void> {
    template <typename T>
    void operator ()(T* p) const NOEXCEPT {
        Meta::Destroy(p);
    }
};
template <typename T>
constexpr TDestructor<T> Destructor;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
template <typename T>
T default_value_(std::bool_constant<false>, int) { return MakeForceInit<T>(); }
template <typename T, T _Unknown = T::Unknown>
T default_value_(std::bool_constant<true>, int) { return _Unknown; }
template <typename T>
T default_value_(std::bool_constant<true>, ...) { return T(0); }
} //!details
template <typename T>
CONSTEXPR T DefaultValue() NOEXCEPT {
    return details::default_value_<T>(std::bool_constant<
        std::is_floating_point<T>::value ||
        std::is_integral<T>::value ||
        std::is_pointer<T>::value ||
        std::is_enum<T>::value
        >{}, 0 );
}
//----------------------------------------------------------------------------
struct FDefaultValue final {
    template <typename T>
    CONSTEXPR operator T () const { return DefaultValue<T>(); }
    template <typename T>
    CONSTEXPR friend bool operator ==(T lhs, FDefaultValue rhs) { return (T(rhs) == lhs); }
    template <typename T>
    CONSTEXPR friend bool operator !=(T lhs, FDefaultValue rhs) { return (not operator ==(lhs, rhs)); }
    template <typename T>
    CONSTEXPR friend bool operator ==(FDefaultValue lhs, T rhs) { return (T(lhs) == rhs); }
    template <typename T>
    CONSTEXPR friend bool operator !=(FDefaultValue lhs, T rhs) { return (not operator ==(lhs, rhs)); }
};
CONSTEXPR FDefaultValue Default{};
//----------------------------------------------------------------------------
struct FZeroValue final {
    template <typename T>
    CONSTEXPR operator T () const { return static_cast<T>(0); }
    template <typename T>
    CONSTEXPR friend bool operator ==(T lhs, FZeroValue rhs) { return (T(rhs) == lhs); }
    template <typename T>
    CONSTEXPR friend bool operator !=(T lhs, FZeroValue rhs) { return (not operator ==(lhs, rhs)); }
    template <typename T>
    CONSTEXPR friend bool operator ==(FZeroValue lhs, T rhs) { return (T(lhs) == rhs); }
    template <typename T>
    CONSTEXPR friend bool operator !=(FZeroValue lhs, T rhs) { return (not operator ==(lhs, rhs)); }
};
CONSTEXPR FZeroValue Zero{};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
using TOptionalReference = TConditional<is_pointer_v<T>, T, TAddPointer<T> >;
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR T* MakeOptionalRef(T& ref) { return std::addressof(ref); }
template <typename T>
CONSTEXPR const T* MakeOptionalRef(const T& cref) { return std::addressof(cref); }
template <typename T>
CONSTEXPR T* MakeOptionalRef(T* ptr) { return ptr; }
template <typename T>
CONSTEXPR const T* MakeOptionalRef(const T* cptr) { return cptr; }
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR auto& DerefPtr(T& ref) {
    IF_CONSTEXPR(is_pointer_v<T>) {
        return *ref;
    }
    else {
        return ref;
    }
}
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR T& DerefPtr(T* ptr) {
    return *ptr;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Unlike STL variants theses operators allow to compare _Lhs with a compatible _Rhs
//----------------------------------------------------------------------------
#define PPE_META_OPERATOR(_NAME, _OP) \
    template <typename _Lhs = void> \
    struct _NAME { \
        template <typename _Rhs> \
        CONSTEXPR bool operator()(const _Lhs& lhs, const _Rhs& rhs) const NOEXCEPT { \
            return (lhs _OP rhs); \
        } \
    }; \
    template <> \
    struct _NAME<void> { \
        template <typename _Lhs, typename _Rhs> \
        CONSTEXPR bool operator()(const _Lhs& lhs, const _Rhs& rhs) const NOEXCEPT { \
            return (lhs _OP rhs); \
        } \
    }
//----------------------------------------------------------------------------
PPE_META_OPERATOR(TGreater, > );
PPE_META_OPERATOR(TGreaterEqual, >= );
PPE_META_OPERATOR(TLess, < );
PPE_META_OPERATOR(TLessEqual, <= );
PPE_META_OPERATOR(TEqualTo, == );
//----------------------------------------------------------------------------
namespace details {
template <typename _Ptr, template <typename T> class _Op>
struct TDerefOperator_ : _Op<decltype( *std::declval<const _Ptr&>() )> {
    using value_type = decltype(*std::declval<_Ptr>());
    using parent_type = _Op<value_type>;
    STATIC_ASSERT(is_pointer_v<_Ptr>);
    CONSTEXPR bool operator()(const _Ptr& lhs, const _Ptr& rhs) const NOEXCEPT {
        return ( (lhs == rhs) || ((!!lhs & !!rhs) && parent_type::operator()(*lhs, *rhs)) );
    }
    CONSTEXPR bool operator()(const _Ptr& lhs, const value_type& rhs) const NOEXCEPT {
        return (!!lhs && parent_type::operator()(*lhs, rhs));
    }
    CONSTEXPR bool operator()(const value_type& lhs, const _Ptr& rhs) const NOEXCEPT {
        return (!!rhs && parent_type::operator()(lhs, *rhs));
    }
};
} //!details
//----------------------------------------------------------------------------
template <typename _Ptr = void>
using TDerefGreater = details::TDerefOperator_< _Ptr, TGreater >;
template <typename _Ptr = void>
using TDerefGreaterEqual = details::TDerefOperator_< _Ptr, TGreaterEqual >;
template <typename _Ptr = void>
using TDerefLess = details::TDerefOperator_< _Ptr, TLess >;
template <typename _Ptr = void>
using TDerefLessEqual = details::TDerefOperator_< _Ptr, TLessEqual >;
template <typename _Ptr = void>
using TDerefEqualTo = details::TDerefOperator_< _Ptr, TEqualTo >;
//----------------------------------------------------------------------------
#undef PPE_META_OPERATOR
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define INSTANTIATE_CLASS_TYPEDEF(_API, _NAME, ...) \
    class _API _NAME final : public __VA_ARGS__ { \
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
            typename = ::PPE::Meta::TEnableIf< Meta::has_constructor<parent_type, _Args...>::value > \
        > \
        explicit _NAME(_Args&&... args) NOEXCEPT \
            : parent_type(std::forward<_Args>(args)...) {} \
        \
        template <typename _Arg> \
        _NAME& operator=(_Arg&& arg) NOEXCEPT { \
            parent_type::operator =(std::forward<_Arg>(arg)); \
            return *this; \
        } \
    }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta

using Meta::Default;
using Meta::ForceInit;
using Meta::Zero;

PPE_ASSERT_TYPE_IS_POD(u128);
PPE_ASSERT_TYPE_IS_POD(u256);

} //!namespace PPE
