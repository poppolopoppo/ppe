#pragma once

#include "Meta/Aliases.h"
#include "Meta/Config.h"

#include <cstring> // memset
#include <functional>
#include <type_traits>

#if USE_PPE_MEMORY_DEBUGGING
#   define USE_PPE_MEMORY_CHECK_INITIALIZATION  (!USE_PPE_SANITIZER) // %_NOCOMMIT%
#   define USE_PPE_MEMORY_CHECK_NECROPHILIA     (!USE_PPE_SANITIZER) // %_NOCOMMIT%
#else
#   define USE_PPE_MEMORY_CHECK_INITIALIZATION  (USE_PPE_DEBUG) // %_NOCOMMIT%
#   define USE_PPE_MEMORY_CHECK_NECROPHILIA     (USE_PPE_DEBUG) // %_NOCOMMIT%
#endif

// Can override POD detection in PPE codebase, respects ADL
#define PPE_ASSERT_TYPE_IS_POD(T) \
    static_assert(::PPE::Meta::is_pod_v<T>, STRINGIZE(T) " is not a POD type");
#define PPE_ASSUME_TYPE_AS_POD(...) \
    CONSTEVAL bool is_pod_type(__VA_ARGS__*) NOEXCEPT { return true; }
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
    CONSTEVAL bool is_pointer_type(__VA_ARGS__*) NOEXCEPT { return true; }
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
struct PPE_CORE_API FNonCopyable {
    FNonCopyable() NOEXCEPT = default;
    FNonCopyable(const FNonCopyable&) = delete;
    FNonCopyable& operator =(const FNonCopyable&) = delete;
    FNonCopyable(FNonCopyable&&) = default;
    FNonCopyable& operator =(FNonCopyable&&) = default;
};
//----------------------------------------------------------------------------
struct PPE_CORE_API FNonMovable {
    FNonMovable() NOEXCEPT = default;
    FNonMovable(const FNonMovable&) = default;
    FNonMovable& operator =(const FNonMovable&) = default;
    FNonMovable(FNonMovable&&) = delete;
    FNonMovable& operator =(FNonMovable&&) = delete;
};
//----------------------------------------------------------------------------
struct PPE_CORE_API FNonCopyableNorMovable {
    FNonCopyableNorMovable() NOEXCEPT = default;
    FNonCopyableNorMovable(const FNonCopyableNorMovable&) = delete;
    FNonCopyableNorMovable& operator =(const FNonCopyableNorMovable&) = delete;
    FNonCopyableNorMovable(FNonCopyableNorMovable&&) = delete;
    FNonCopyableNorMovable& operator =(FNonCopyableNorMovable&&) = delete;
};
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
using TArray = T[_Dim];
//----------------------------------------------------------------------------
template <typename T, T _Value>
using TIntegralConstant = typename std::integral_constant<T, _Value>::type;
//----------------------------------------------------------------------------
template <typename T>
inline CONSTEXPR bool AlwaysFalse = false;
//----------------------------------------------------------------------------
// Function overloading helpers to use dummy parameters for specialization :
//----------------------------------------------------------------------------
template <typename T>
struct TType final { using type = T; };
//----------------------------------------------------------------------------
template <typename T>
inline CONSTEXPR const TType<T> Type{};
//----------------------------------------------------------------------------
template <auto V>
struct TValue final { static constexpr auto value = V; };
//----------------------------------------------------------------------------
template <auto V>
inline CONSTEXPR const TValue<V> Value{};
//----------------------------------------------------------------------------
// Tricking the compiler to make implicit conversion with template argument deduction
// https://stackoverflow.com/questions/45765205/template-function-argument-deduction-with-an-implicit-conversion
//----------------------------------------------------------------------------
template<class T>
using TDontDeduce = typename TType<T>::type;
//----------------------------------------------------------------------------
// This ugly piece of crap is just used in STATIC_ASSERT() to get to see the
// actual size of the types when the assertion fails :
//----------------------------------------------------------------------------
template <auto A, auto B>
struct TCheckEquals {
    STATIC_CONST_INTEGRAL(bool, value, (A == B));
    static_assert(value, "A and B should be equal !");
};
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
// is_pod_type(T*) can be overloaded for specific types, respects ADL
//----------------------------------------------------------------------------
template <typename T>
CONSTEVAL bool is_pod_type(T*) NOEXCEPT { return std::is_trivial_v<T> && std::is_standard_layout_v<T>; }
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
// is_pointer_type(T*) can be overloaded for specific types, respects ADL
//----------------------------------------------------------------------------
template <typename T>
CONSTEVAL bool is_pointer_type(T*) NOEXCEPT { return std::is_pointer_v<T>; }
template <typename T>
CONSTEXPR bool is_pointer_v{ is_pointer_type(static_cast< Meta::TDecay<T> *>(nullptr)) };
//----------------------------------------------------------------------------
// Uses SFINAE to determine if T has a compatible constructor
//----------------------------------------------------------------------------
template <typename T, typename... _Args>
using has_constructor = typename std::is_constructible<T, _Args...>::type;
template <typename T>
using has_default_constructor = typename std::is_constructible<T>::type;
//----------------------------------------------------------------------------
// A type must be complete in order to have the sizeof operator applied to it,
// so we use SFINAE to define is_type_complete_v as true
// https://devblogs.microsoft.com/oldnewthing/20190710-00/?p=102678
// /!\ WARNING: the result will be cached by the compiler
//      => ONCE UNDEFINED, NEVER DEFINED
//----------------------------------------------------------------------------
template<typename, size_t _Seed = 0, typename = void>
CONSTEXPR bool is_type_complete_v = false;
//----------------------------------------------------------------------------
template<typename T, size_t _Seed>
CONSTEXPR bool is_type_complete_v<T, _Seed, std::void_t<decltype(sizeof(T))>> = true;
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
// Optional control for data initialization
//----------------------------------------------------------------------------
struct FNoInit final {}; // ctor with that arg must *NOT* initialize the data
struct FForceInit final {}; // ctor with that arg must initialize the data
//----------------------------------------------------------------------------
inline CONSTEXPR FNoInit NoInit{};
inline CONSTEXPR FForceInit ForceInit{};
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
#ifdef __clang__
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wdynamic-class-memaccess"
#endif
//----------------------------------------------------------------------------
template <typename T>
FORCE_INLINE void PoisonMemory(T* p, u8 pattern) {
#if USE_PPE_MEMORY_CHECK_INITIALIZATION || USE_PPE_MEMORY_CHECK_NECROPHILIA
    ::memset(p, pattern, sizeof(T)); // uninitialized field detection
#else
    (void)(p, pattern);
#endif
}
//----------------------------------------------------------------------------
template <typename T>
FORCE_INLINE void PoisonConstruct(T* p) {
#if USE_PPE_MEMORY_CHECK_INITIALIZATION
    PoisonMemory(p, 0xCC);
#else
    (void)(p);
#endif
}
//----------------------------------------------------------------------------
template <typename T>
FORCE_INLINE void PoisonDestroy(T* p) {
#if USE_PPE_MEMORY_CHECK_INITIALIZATION
    PoisonMemory(p, 0xDD);
#else
    (void)(p);
#endif
}
//----------------------------------------------------------------------------
#ifdef __clang__
#   pragma clang diagnostic pop
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
template <typename T>
FORCE_INLINE void Construct_(T* p, FNoInit, std::false_type) { INPLACE_NEW(p, T){}; }
template <typename T>
FORCE_INLINE void Construct_(T* p, FNoInit noInit, std::true_type) { INPLACE_NEW(p, T)(noInit); }
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
FORCE_INLINE void Construct_(T* p, FForceInit init, std::true_type) { INPLACE_NEW(p, T)(init); }
} //!details
template <typename T>
void Construct(T* p, FForceInit init) {
    Assume(p);
    PoisonConstruct(p);
    details::Construct_(p, init, typename has_forceinit_constructor<T>::type{});
}
//----------------------------------------------------------------------------
template <typename T>
void Construct(T* p) {
    Assume(p);
    typedef char type_must_be_complete[sizeof(T) ? 1 : -1];
    (void) sizeof(type_must_be_complete);
    PoisonConstruct(p);
    INPLACE_NEW(p, T)();
}
//----------------------------------------------------------------------------
template <typename T>
void Construct(T* p, T&& rvalue) {
    Assume(p);
    PoisonConstruct(p);
    INPLACE_NEW(p, T)(std::move(rvalue));
}
//----------------------------------------------------------------------------
template <typename T>
void Construct(T* p, const T& other) {
    Assume(p);
    PoisonConstruct(p);
    INPLACE_NEW(p, T)(other);
}
//----------------------------------------------------------------------------
template <typename T, typename... _Args>
void Construct(T* p, _Args&&... args) {
    Assume(p);
    PoisonConstruct(p);
    INPLACE_NEW(p, T){ std::forward<_Args>(args)... };
}
//----------------------------------------------------------------------------
template <typename T>
Meta::TEnableIf<not std::is_const_v<T>> Destroy(T* p) NOEXCEPT {
    Assume(p);
    typedef char type_must_be_complete[sizeof(T) ? 1 : -1];
    (void) sizeof(type_must_be_complete);
    p->~T();
    PoisonDestroy(p);
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
CONSTEXPR T default_value_(std::bool_constant<false>, int) { return MakeForceInit<T>(); }
template <typename T, T _Unknown = T::Unknown>
CONSTEXPR T default_value_(std::bool_constant<true>, int) { return _Unknown; }
template <typename T>
CONSTEXPR T default_value_(std::bool_constant<true>, ...) { return T(0); }
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
    template <typename T, class = TEnableIf<has_default_constructor<T>::value> >
    CONSTEXPR operator T () const { return DefaultValue<T>(); }
    template <typename T>
    CONSTEXPR friend bool operator ==(T lhs, FDefaultValue ) { return (DefaultValue<T>() == lhs); }
    template <typename T>
    CONSTEXPR friend bool operator !=(T lhs, FDefaultValue rhs) { return (not operator ==(lhs, rhs)); }
    template <typename T>
    CONSTEXPR friend bool operator ==(FDefaultValue , T rhs) { return (DefaultValue<T>() == rhs); }
    template <typename T>
    CONSTEXPR friend bool operator !=(FDefaultValue lhs, T rhs) { return (not operator ==(lhs, rhs)); }
};
inline CONSTEXPR FDefaultValue Default{};
//----------------------------------------------------------------------------
struct FZeroValue final {
    template <typename T>
    CONSTEXPR operator T () const { return static_cast<T>(0); }
    template <typename T>
    CONSTEXPR friend bool operator ==(T lhs, FZeroValue ) { return (static_cast<T>(0) == lhs); }
    template <typename T>
    CONSTEXPR friend bool operator !=(T lhs, FZeroValue rhs) { return (not operator ==(lhs, rhs)); }
    template <typename T>
    CONSTEXPR friend bool operator ==(FZeroValue , T rhs) { return (static_cast<T>(0) == rhs); }
    template <typename T>
    CONSTEXPR friend bool operator !=(FZeroValue lhs, T rhs) { return (not operator ==(lhs, rhs)); }
};
inline CONSTEXPR FZeroValue Zero{};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Avoid using T** when only needing T* for pointer semantics (GetIFP)
//----------------------------------------------------------------------------
template <typename T>
using TOptionalReference = TConditional<is_pointer_v<T>, T, TAddPointer<T> >;
//----------------------------------------------------------------------------
template <typename T>
CONSTEXPR T* MakeOptionalRef(T* ptr) { return ptr; }
template <typename T>
CONSTEXPR auto MakeOptionalRef(T& ref) {
    IF_CONSTEXPR(not is_pointer_v<T>)
        return std::addressof(ref);
    else
        return ref;
}
//----------------------------------------------------------------------------
// Generic pointer dereference, overloaded by pointer-like classes
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
template <typename... _Args>
FORCE_INLINE CONSTEXPR void Unused(_Args&& ...) NOEXCEPT {}
//----------------------------------------------------------------------------
template <typename T, u32 _Dim>
CONSTEXPR u32 lengthof(const T (&)[_Dim]) { return _Dim; }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define INSTANTIATE_CLASS_TYPEDEF(_API, _NAME, ...) \
    class _API _NAME final : public __VA_ARGS__ { \
    public: \
        using parent_type = __VA_ARGS__; \
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
        \
        parent_type& Inner() { return *this; } \
        const parent_type& Inner() const { return *this; } \
    }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta

using Meta::Default;
using Meta::ForceInit;
using Meta::Zero;
using Meta::Unused;
using Meta::lengthof;

PPE_ASSERT_TYPE_IS_POD(u128);
PPE_ASSERT_TYPE_IS_POD(u256);

} //!namespace PPE
