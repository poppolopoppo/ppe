#pragma once

#include "Meta/Config.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <type_traits>

using u8    = std::uint8_t;
using u16   = std::uint16_t;
using u32   = std::uint32_t;
using u64   = std::uint64_t;

using i8    = std::int8_t;
using i16   = std::int16_t;
using i32   = std::int32_t;
using i64   = std::int64_t;

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if     defined(ARCH_X64)
#   if !(defined(_M_X64) || defined(__x86_64__))
#       error "invalid architecture"
#   endif
#   define CODE3264(_X32, _X64) _X64
#elif   defined(ARCH_X86)
#   if !(defined(_M_IX86) || defined(__i386__))
#       error "invalid architecture"
#   endif
#   define CODE3264(_X32, _X64) _X32
#else
#   error "unknown architecture !"
#endif
//----------------------------------------------------------------------------
#if (defined(__cplusplus) && __cplusplus >= 202002L) || (defined(_HAS_CXX20) && _HAS_CXX20 == 1)
#   define PPE_HAS_CXX20 1
#else
#   define PPE_HAS_CXX20 0
#endif
//----------------------------------------------------------------------------
#if (defined(__cplusplus) && __cplusplus >= 201703L) || (defined(_HAS_CXX17) && _HAS_CXX17 == 1)
#   define PPE_HAS_CXX17 1
#else
#   define PPE_HAS_CXX17 (PPE_HAS_CXX20)
#endif
//----------------------------------------------------------------------------
#if (defined(__cplusplus) && __cplusplus >= 201402L) || (defined(_HAS_CXX14) && _HAS_CXX14 == 1)
#   define PPE_HAS_CXX14 1
#else
#   define PPE_HAS_CXX14 (PPE_HAS_CXX20|PPE_HAS_CXX17)
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifdef CPP_VISUALSTUDIO
#   ifndef not
#       define not !
#   endif
#   ifndef and
#       define and &&
#   endif
#   ifndef or
#       define or ||
#   endif
#endif
//----------------------------------------------------------------------------
#define EXPAND(x) x
#define EXPAND_VA(X, ...)  X, ##__VA_ARGS__
//----------------------------------------------------------------------------
#define COMMA_3 ,
#define COMMA_2 EXPAND( COMMA_3 )
#define COMMA_1 EXPAND( COMMA_2 )
#define COMMA COMMA_1
#define COMMA_PROTECT(...) __VA_ARGS__
//----------------------------------------------------------------------------
#define STRINGIZE_2(...) # __VA_ARGS__
#define STRINGIZE_1(...) EXPAND( STRINGIZE_2(__VA_ARGS__) )
#define STRINGIZE_0(...) EXPAND( STRINGIZE_1(__VA_ARGS__) )
#define STRINGIZE(...) EXPAND( STRINGIZE_0(__VA_ARGS__) )
//----------------------------------------------------------------------------
#define WIDESTRING_2(_X) L##_X
#define WIDESTRING_1(_X) WIDESTRING_2(_X)
#define WIDESTRING_0(_X) WIDESTRING_1(_X)
#define WIDESTRING(_X) WIDESTRING_0(_X)
//----------------------------------------------------------------------------
#define WSTRINGIZE(...) WIDESTRING(STRINGIZE(__VA_ARGS__))
//----------------------------------------------------------------------------
    #define CONCAT_I(_X, _Y) _X ## _Y
    #define CONCAT_OO(_ARGS) CONCAT_I ## _ARGS
    #define CONCAT(_X, _Y) CONCAT_I(_X, _Y) //CONCAT_OO((_X, _Y))
    #define CONCAT3(_X, _Y, _Z) CONCAT(_X, CONCAT(_Y, _Z))
//----------------------------------------------------------------------------
#define ANONYMIZE(_X) CONCAT(_X, __LINE__)
//----------------------------------------------------------------------------
#define STATIC_ASSERT(...) static_assert(COMMA_PROTECT(__VA_ARGS__), #__VA_ARGS__)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if defined(CPP_VISUALSTUDIO) && _MSC_VER >= 1900
#   define NOALIAS      __declspec(noalias)
#   define NOEXCEPT     noexcept
#   define NOOP(...)    (void)0
#   define NORETURN     [[noreturn]]
#   define RESTRICT     __declspec(restrict)
#   define STDCALL      __stdcall
#   define VECTORCALL   __vectorcall
#   define CONSTF
#   define PUREF
#   define THREAD_LOCAL thread_local
#   define STATIC_CONST_INTEGRAL(_TYPE, _NAME, ...) static CONSTEXPR _TYPE _NAME = (_TYPE)(__VA_ARGS__)
#   define EMPTY_BASES  __declspec(empty_bases)
#elif defined (CPP_VISUALSTUDIO)
#   define NOALIAS      __declspec(noalias)
#   define NOEXCEPT     __declspec(nothrow)
#   define NOOP(...)    __noop(__VA_ARGS__)
#   define NORETURN     __declspec(noreturn)
#   define RESTRICT     __declspec(restrict)
#   define STDCALL      __stdcall
#   define VECTORCALL   __vectorcall
#   define CONSTF
#   define PUREF
#   define THREAD_LOCAL __declspec(thread)
#   define STATIC_CONST_INTEGRAL(_TYPE, _NAME, ...) enum : _TYPE { _NAME = (_TYPE)(__VA_ARGS__) }
#   define EMPTY_BASES
#elif defined(CPP_CLANG)
#   define NOALIAS
#   define NOEXCEPT     noexcept
#   define NOOP(...)    (void)0
#   define NORETURN     [[noreturn]]
#   if defined(_MSC_VER) && (defined(__INTELLISENSE__) || defined(_PREFAST_))
#       define CONSTF
#       define PUREF
#   else
#       define CONSTF       __attribute__((const))
#       define PUREF        __attribute__((pure))
#   endif
#   define THREAD_LOCAL thread_local
#   define STATIC_CONST_INTEGRAL(_TYPE, _NAME, ...) static constexpr _TYPE _NAME = (_TYPE)(__VA_ARGS__)
#   if defined(_MSC_VER)
#       define EMPTY_BASES  __declspec(empty_bases)
#       define RESTRICT     __declspec(restrict)
#       define STDCALL      __stdcall
#       define VECTORCALL   __vectorcall
#   else
#       define EMPTY_BASES
#       define RESTRICT
#       define STDCALL
#       define VECTORCALL
#   endif
#else
#   error "unsupported compiler"
#endif
#if (!defined(_DEBUG) || defined(NDEBUG) || USE_PPE_FASTDEBUG) && !(USE_PPE_MEMORY_DEBUGGING || USE_PPE_SANITIZER)
#   if defined(_MSC_VER)
#       define FORCE_INLINE __forceinline
#   elif defined(__clang__) || defined(__gcc__)
#       define FORCE_INLINE inline __attribute__((always_inline))
#   else
#       error "unsupported compiler"
#   endif
#else
#   define FORCE_INLINE inline // don't want force inline when debugging
#endif
#if defined(_MSC_VER)
#   define NO_INLINE __declspec(noinline)
#elif defined(__clang__) || defined(__gcc__)
#   define NO_INLINE __attribute__((noinline))
#else
#   error "unsupported compiler"
#endif
#define SIMD_INLINE FORCE_INLINE VECTORCALL
//----------------------------------------------------------------------------
#if defined(CPP_VISUALSTUDIO)
#   define Likely(...) __VA_ARGS__
#   define Unlikely(...) __VA_ARGS__
#   define Assume(...) __assume(__VA_ARGS__)
#   if 1 // expands to do { ... } while(0) with pragmas....
#       define AnalysisAssume(...) __analysis_assume(__VA_ARGS__)
#   else // should do the same thing, without bullshit
#       if defined(_PREFAST_) || defined(__INTELLISENSE__)
#           define AnalysisAssume(...) (__noop(!!(__VA_ARGS__)),__assume(!!(__VA_ARGS__)))
#       else
#           define AnalysisAssume(...) NOOP();
#       endif
#   endif
#   define PACKED_STRUCT(_NAME, ...) __pragma(pack(push, 1)) struct _NAME __VA_ARGS__ __pragma(pack(pop))
#elif ((__GNUC__ * 100 + __GNUC_MINOR__) >= 302) || (__INTEL_COMPILER >= 800) || defined(__clang__)
#   define Likely(...) (__builtin_expect (!!(__VA_ARGS__),1) )
#   define Unlikely(...) (__builtin_expect (!!(__VA_ARGS__),0) )
#   define Assume(...) ((void)Likely(__VA_ARGS__))
#   define AnalysisAssume(...) NOOP(!!(__VA_ARGS__))
#   define PACKED_STRUCT(_NAME, ...) struct __attribute__((__packed__)) _NAME __VA_ARGS__
#else
#   error "unsupported compiler"
#endif
//----------------------------------------------------------------------------
#if PPE_HAS_CXX14 || PPE_HAS_CXX17
#   define CONSTEXPR constexpr
#   define NOEXCEPT_IF(...) noexcept(__VA_ARGS__)
#else
#   define CONSTEXPR
#   define NOEXCEPT_IF(...)
#endif
#if PPE_HAS_CXX20
#   define CONSTEVAL consteval
#   define CONSTINIT constinit
#else
#   define CONSTEVAL CONSTEXPR
#   define CONSTINIT CONSTEXPR
#endif
//----------------------------------------------------------------------------
#if PPE_HAS_CXX17
//  http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0292r2.html
#   define IF_CONSTEXPR(...) if constexpr(__VA_ARGS__)
//  http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n4295.html
#   define FOLD_EXPR(...) ((__VA_ARGS__), ...)
//  https://en.cppreference.com/w/cpp/language/attributes/maybe_unused
#   define MAYBE_UNUSED [[maybe_unused]]
//  https://en.cppreference.com/w/cpp/language/attributes/fallthrough
#   define FALLTHROUGH() [[fallthrough]]
#else
#   define IF_CONSTEXPR(...) if (__VA_ARGS__)
//  Workaround from Jason Turner: https://youtu.be/nnY4e4faNp0?t=39m51s
#   define FOLD_EXPR(...) (void)std::initializer_list<int>{ ((__VA_ARGS__), 0)... }
#   define MAYBE_UNUSED
#   define FALLTHROUGH() NOOP()
#endif //!PPE_HAS_CXX17
//----------------------------------------------------------------------------
#if PPE_HAS_CXX17
//  https://en.cppreference.com/w/cpp/language/attributes/nodiscard
#   define NODISCARD [[nodiscard]]
#else
#   define NODISCARD
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if USE_PPE_EXCEPTIONS
#   if PPE_HAS_CXX17
#    define PPE_THROW()               noexcept(false)
#   else
#    define PPE_THROW()               throw ()
#   endif
#   define PPE_THROW_IT(...)          throw __VA_ARGS__
#   define PPE_THROW_VOID()           throw
#   define PPE_TRY                    try
#   define PPE_CATCH_BLOCK(...)       __VA_ARGS__
#   define PPE_CATCH(_SPEC)           catch(_SPEC)
#else
#   define PPE_THROW()
#   define PPE_THROW_IT(...)          PPE_DEBUG_CRASH()
#   define PPE_THROW_VOID()           PPE_DEBUG_CRASH()
#   define PPE_TRY                    if (true)
#   define PPE_CATCH_BLOCK(...)
#   define PPE_CATCH(spec)            if (false)
#endif
//----------------------------------------------------------------------------
#ifdef CPP_VISUALSTUDIO
#   define PPE_THROW_SPECIFIER(_Type)  _THROW1(_Type)
#else
#   define PPE_THROW_SPECIFIER(_Type)
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Force using (void*) overload :
#define INPLACE_NEW(_STORAGE, ...) new ((void*)(_STORAGE)) __VA_ARGS__
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// https://gcc.gnu.org/wiki/Visibility
#if defined(BUILD_LINK_STATIC)
#   define DLL_IMPORT
#   define DLL_EXPORT
#   define DLL_NOINLINE
#   define DLL_LOCAL
#elif defined(BUILD_LINK_DYNAMIC)
#   if defined(_WIN32) || defined(__CYGWIN__)
#       define DLL_IMPORT __declspec(dllimport)
#       define DLL_EXPORT __declspec(dllexport)
#       define DLL_LOCAL
#   else
#       define DLL_IMPORT __attribute__ ((visibility ("default")))
#       define DLL_EXPORT __attribute__ ((visibility ("default")))
#       define DLL_LOCAL  __attribute__ ((visibility ("hidden")))
#   endif
#   define DLL_NOINLINE NO_INLINE
#else
// #   error "inconsistent configuration" // <- headers
#endif
//----------------------------------------------------------------------------
#define EXTERN_TEMPLATE_CLASS_DECL(_API) extern template class
#define EXTERN_TEMPLATE_CLASS_DEF(_API) template class _API
//----------------------------------------------------------------------------
#define EXTERN_TEMPLATE_STRUCT_DECL(_API) extern template struct
#define EXTERN_TEMPLATE_STRUCT_DEF(_API) template struct _API
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if defined(CPP_VISUALSTUDIO)
#   define PRAGMA_DISABLE_OPTIMIZATION_ACTUAL   __pragma(optimize("",off))
#   define PRAGMA_ENABLE_OPTIMIZATION_ACTUAL    __pragma(optimize("",on ))
#   ifdef __clang__ // clang-cl
#       define PRAGMA_DISABLE_RUNTIMECHECKS_ACTUAL \
            __pragma(runtime_checks("",off)) \
            __pragma(strict_gs_check(push,off))
#       define PRAGMA_RESTORE_RUNTIMECHECKS_ACTUAL \
            __pragma(runtime_checks("",restore)) \
            __pragma(strict_gs_check(pop))
#   else
#       define PRAGMA_DISABLE_RUNTIMECHECKS_ACTUAL \
            __pragma(runtime_checks("",off)) \
            __pragma(check_stack(off)) \
            __pragma(strict_gs_check(push,off))
#       define PRAGMA_RESTORE_RUNTIMECHECKS_ACTUAL \
            __pragma(runtime_checks("",restore)) \
            __pragma(check_stack()) \
            __pragma(strict_gs_check(pop))
#   endif
#elif defined(CPP_CLANG)
#   define PRAGMA_DISABLE_OPTIMIZATION_ACTUAL   _Pragma("clang optimize off")
#   define PRAGMA_ENABLE_OPTIMIZATION_ACTUAL    _Pragma("clang optimize on")
#   define PRAGMA_DISABLE_RUNTIMECHECKS_ACTUAL  //#TODO
#   define PRAGMA_RESTORE_RUNTIMECHECKS_ACTUAL  //#TODO
#else
#   error "need to implement PRAGMA_ENABLE/DISABLE_OPTIMIZATION !"
#endif
//----------------------------------------------------------------------------
#define PRAGMA_DISABLE_OPTIMIZATION PRAGMA_DISABLE_OPTIMIZATION_ACTUAL
#if USE_PPE_DEBUG
#   define PRAGMA_ENABLE_OPTIMIZATION PRAGMA_DISABLE_OPTIMIZATION_ACTUAL
#else
#   define PRAGMA_ENABLE_OPTIMIZATION PRAGMA_ENABLE_OPTIMIZATION_ACTUAL
#endif
//----------------------------------------------------------------------------
#define PRAGMA_DISABLE_RUNTIMECHECKS PRAGMA_DISABLE_RUNTIMECHECKS_ACTUAL
#define PRAGMA_RESTORE_RUNTIMECHECKS PRAGMA_RESTORE_RUNTIMECHECKS_ACTUAL
//----------------------------------------------------------------------------
#if defined(CPP_VISUALSTUDIO)
#   define PRAGMA_INITSEG_LIB \
    __pragma(warning(disable: 4073)) \
    __pragma(init_seg(lib))
#   define PRAGMA_INITSEG_COMPILER \
    __pragma(warning(disable: 4074)) \
    __pragma(init_seg(compiler))
#   define INITSEG_LIB_PRIORITY
#   define INITSEG_COMPILER_PRIORITY
#elif defined(CPP_CLANG) || defined(CPP_GCC)
#   define PRAGMA_INITSEG_LIB
#   define PRAGMA_INITSEG_COMPILER
#   define INITSEG_LIB_PRIORITY __attribute__((init_priority (10000+__LINE__)))
#   define INITSEG_COMPILER_PRIORITY __attribute__((init_priority(100+__LINE__)))
#else
#   error "need to implement PRAGMA_INITSEG_LIB/COMPILER !"
#endif
//----------------------------------------------------------------------------
#if defined(_MSC_VER)
#   define PPE_PRETTY_FUNCTION __FUNCSIG__
#elif defined(CPP_GCC) or defined(CPP_CLANG)
#   define PPE_PRETTY_FUNCTION __PRETTY_FUNCTION__
#else
#   error "need to implement PPE_PRETTY_FUNCTION !"
#   define PPE_PRETTY_FUNCTION __func__
#endif
//----------------------------------------------------------------------------
#if defined(__cpp_rtti)
#   define PPE_TYPEID_NAME(T) (typeid(T).name())
#else
#   define PPE_TYPEID_NAME(T) (static_cast<const char*>(nullptr))
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
using byte      = i8;
using ubyte     = u8;
//using short   = i16;
using ushort    = u16;
using word      = i32;
using uword     = u32;
//----------------------------------------------------------------------------
struct uint128_t {
    u64 lo, hi;

    CONSTEXPR static uint128_t Zero() { return uint128_t{ 0, 0 }; }

    CONSTEXPR friend bool operator ==(const uint128_t& lhs, const uint128_t& rhs) { return ((lhs.hi == rhs.hi) && (lhs.lo == rhs.lo)); }
    CONSTEXPR friend bool operator !=(const uint128_t& lhs, const uint128_t& rhs) { return not operator ==(lhs, rhs); }

    CONSTEXPR friend bool operator < (const uint128_t& lhs, const uint128_t& rhs) { return lhs.hi == rhs.hi ? lhs.lo < rhs.lo : lhs.hi < rhs.hi; }
    CONSTEXPR friend bool operator >=(const uint128_t& lhs, const uint128_t& rhs) { return not operator < (lhs, rhs); }

    friend void swap(uint128_t& lhs, uint128_t& rhs) NOEXCEPT { std::swap(lhs.lo, rhs.lo); std::swap(lhs.hi, rhs.hi); }

};
using u128 = uint128_t;
//----------------------------------------------------------------------------
struct uint256_t {
    uint128_t lo, hi;

    CONSTEXPR static uint256_t Zero() { return uint256_t{ uint128_t::Zero(), uint128_t::Zero() }; }

    CONSTEXPR friend bool operator ==(const uint256_t& lhs, const uint256_t& rhs) { return ((lhs.hi == rhs.hi) && (lhs.lo == rhs.lo)); }
    CONSTEXPR friend bool operator !=(const uint256_t& lhs, const uint256_t& rhs) { return not operator ==(lhs, rhs); }

    CONSTEXPR friend bool operator < (const uint256_t& lhs, const uint256_t& rhs) { return lhs.hi == rhs.hi ? lhs.lo < rhs.lo : lhs.hi < rhs.hi; }
    CONSTEXPR friend bool operator >=(const uint256_t& lhs, const uint256_t& rhs) { return not operator < (lhs, rhs); }

    friend void swap(uint256_t& lhs, uint256_t& rhs) NOEXCEPT { swap(lhs.lo, rhs.lo); swap(lhs.hi, rhs.hi); }

};
using u256 = uint256_t;
//----------------------------------------------------------------------------
// for hash functions :
PACKED_STRUCT(uint96_t,  { u64  lo; u32 hi; });
using u96 = uint96_t;
PACKED_STRUCT(uint160_t, { u128 lo; u32 hi; });
using u160 = uint160_t;
PACKED_STRUCT(uint192_t, { u128 lo; u64 hi; });
using u192 = uint192_t;
PACKED_STRUCT(uint224_t, { u128 lo; u96 hi; });
using u224 = uint224_t;
//----------------------------------------------------------------------------
#define STATICCAST_LITERAL_OP(SRC, DST) \
    CONSTEXPR DST operator "" CONCAT(_, DST)(SRC value) { \
        return static_cast<DST>(value); \
    }
STATICCAST_LITERAL_OP(unsigned long long, size_t)
STATICCAST_LITERAL_OP(unsigned long long, u8)
STATICCAST_LITERAL_OP(unsigned long long, u16)
STATICCAST_LITERAL_OP(unsigned long long, u32)
STATICCAST_LITERAL_OP(unsigned long long, u64)
STATICCAST_LITERAL_OP(unsigned long long, i8)
STATICCAST_LITERAL_OP(unsigned long long, i16)
STATICCAST_LITERAL_OP(unsigned long long, i32)
STATICCAST_LITERAL_OP(unsigned long long, i64)
#undef STATICCAST_LITERAL_OP
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
