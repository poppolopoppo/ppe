#pragma once

#include <algorithm>
#include <stdint.h>
#include <type_traits>

typedef uint8_t     u8;
typedef uint16_t    u16;
typedef uint32_t    u32;
typedef uint64_t    u64;

typedef int8_t      i8;
typedef int16_t     i16;
typedef int32_t     i32;
typedef int64_t     i64;

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if     defined(ARCH_X64)
#   ifndef _M_X64
#       error "invalid architecture"
#   endif
#   define CODE3264(_X32, _X64) _X64
#elif   defined(ARCH_X86)
#   ifndef _M_IX86
#       error "invalid architecture"
#   endif
#   define CODE3264(_X32, _X64) _X32
#else
#   error "unknown architecture !"
#endif
//----------------------------------------------------------------------------
#ifndef _HAS_CXX17
#   define _HAS_CXX17 0
#endif
//----------------------------------------------------------------------------
#ifndef _HAS_CXX14
#   define _HAS_CXX14 (_HAS_CXX17) // CXX14 also available if CXX17
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifndef not
#   define not !
#endif
//----------------------------------------------------------------------------
#define EXPAND(x) x
#define EXPAND_VA(X, ...)  X, ##__VA_ARGS__
//----------------------------------------------------------------------------
#define COMMA_2 ,
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
#define NOOP(...) __noop(__VA_ARGS__)
#define UNUSED(_X) (void)(_X)
//----------------------------------------------------------------------------
#define ANONYMIZE(_X) CONCAT(_X, __LINE__)
//----------------------------------------------------------------------------
#define lengthof(_ARRAY) ((sizeof(_ARRAY))/(sizeof(_ARRAY[0])))
//----------------------------------------------------------------------------
constexpr size_t INDEX_NONE = size_t(-1);
//----------------------------------------------------------------------------
#define STATIC_ASSERT(...) static_assert(COMMA_PROTECT(__VA_ARGS__), #__VA_ARGS__)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if defined(CPP_CLANG)
#   define NOALIAS
#   define NOEXCEPT     noexcept
#   define NORETURN     __declspec(noreturn)
#   define RESTRICT     __declspec(restrict)
#   define STDCALL      __stdcall
#   define VECTORCALL   __vectorcall
#   define THREAD_LOCAL thread_local
#   define STATIC_CONST_INTEGRAL(_TYPE, _NAME, ...) static constexpr _TYPE _NAME = (_TYPE)(__VA_ARGS__)
#   if defined(_MSC_VER)
#       define EMPTY_BASES __declspec(empty_bases)
#   else
#       define EMPTY_BASES
#   endif
#elif defined(CPP_VISUALSTUDIO) && _MSC_VER >= 1900
#   define NOALIAS      __declspec(noalias)
#   define NOEXCEPT     noexcept
#   define NORETURN     __declspec(noreturn)
#   define RESTRICT     __declspec(restrict)
#   define STDCALL      __stdcall
#   define VECTORCALL   __vectorcall
#   define THREAD_LOCAL thread_local
#   define STATIC_CONST_INTEGRAL(_TYPE, _NAME, ...) static constexpr _TYPE _NAME = (_TYPE)(__VA_ARGS__)
#   define EMPTY_BASES  __declspec(empty_bases)
#elif defined (CPP_VISUALSTUDIO)
#   define NOALIAS      __declspec(noalias)
#   define NOEXCEPT     __declspec(nothrow)
#   define NORETURN     __declspec(noreturn)
#   define RESTRICT     __declspec(restrict)
#   define STDCALL      __stdcall
#   define VECTORCALL   __vectorcall
#   define THREAD_LOCAL __declspec(thread)
#   define STATIC_CONST_INTEGRAL(_TYPE, _NAME, ...) enum : _TYPE { _NAME = (_TYPE)(__VA_ARGS__) }
#   define EMPTY_BASES
#else
#   error "unsupported compiler"
#endif
#if !defined(_DEBUG) || defined(NDEBUG)
#   define FORCE_INLINE    __forceinline //__attribute__((always_inline))
#else
#   define FORCE_INLINE    inline // don't want force inline when debugging
#endif
#define NO_INLINE       __declspec(noinline)
#define SIMD_INLINE     FORCE_INLINE VECTORCALL
//----------------------------------------------------------------------------
#if ((__GNUC__ * 100 + __GNUC_MINOR__) >= 302) || (__INTEL_COMPILER >= 800) || defined(__clang__)
#   define Likely(...) (__builtin_expect (!!(__VA_ARGS__),1) )
#   define Unlikely(...) (__builtin_expect (!!(__VA_ARGS__),0) )
#   define Assume(...) Likely(__VA_ARGS__)
#elif defined(CPP_VISUALSTUDIO)
#   define Likely(...) __VA_ARGS__
#   define Unlikely(...) __VA_ARGS__
#   define Assume(...) __assume(__VA_ARGS__)
#else
#   error "unsupported compiler"
#endif
//----------------------------------------------------------------------------
#if _HAS_CXX14 || _HAS_CXX17
#   define CONSTEXPR constexpr
#else
#   define CONSTEXPR
#endif
//----------------------------------------------------------------------------
#if USE_PPE_DEBUG
#   define RELEASE_CONSTEXPR
#else
#   define RELEASE_CONSTEXPR CONSTEXPR
#endif
//----------------------------------------------------------------------------
#if _HAS_CXX17
//  http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0292r2.html
#   define IF_CONSTEXPR(...) if constexpr(__VA_ARGS__)
//  http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n4295.html
#   define FOLD_EXPR(...) ((__VA_ARGS__), ...)
#else
#   define IF_CONSTEXPR(...) if (__VA_ARGS__)
//  Workaround from Jason Turner: https://youtu.be/nnY4e4faNp0?t=39m51s
#   define FOLD_EXPR(...) (void)std::initializer_list<int>{ ((__VA_ARGS__), 0)... }
#endif //!_HAS_CXX17
//----------------------------------------------------------------------------
#if _HAS_CXX17
//  https://en.cppreference.com/w/cpp/language/attributes/nodiscard
#   define NODISCARD [[nodiscard]]
#else
#   define NODISCARD
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if USE_PPE_EXCEPTIONS
#   if _HAS_CXX17
#    define PPE_THROW()               noexcept(false)
#   else
#    define PPE_THROW()               throw ()
#   endif
#   define PPE_THROW_IT(_EX)          throw _EX
#   define PPE_THROW_VOID()           throw
#   define PPE_TRY                    try
#   define PPE_CATCH_BLOCK(...)       __VA_ARGS__
#   define PPE_CATCH(_SPEC)           catch(_SPEC)
#else
#   define PPE_THROW()
#   define PPE_THROW_IT(_EX)          PPE_ABORT(STRINGIZE(_EX))
#   define PPE_THROW_VOID()           PPE_ABORT("throw")
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
#if     defined(STATIC_LINK)
#   define DLL_IMPORT
#   define DLL_EXPORT
#elif   defined(DYNAMIC_LINK)
#   if     defined(CPP_CLANG)
#       define DLL_IMPORT __declspec(dllimport)
#       define DLL_EXPORT __declspec(dllexport)
#   elif   defined(CPP_VISUALSTUDIO)
#       define DLL_IMPORT __declspec(dllimport)
#       define DLL_EXPORT __declspec(dllexport)
#   elif   defined(CPP_GCC)
#       define DLL_IMPORT __attribute__ ((dllimport))
#       define DLL_EXPORT __attribute__ ((dllexport))
#   else
#       error "unsupported compiler"
#   endif
#else
#   error "inconsistent configuration"
#endif
//----------------------------------------------------------------------------
#ifdef DYNAMIC_LINK
#   define EXPORT_DELETED_FUNCTION = default
#else
#   define EXPORT_DELETED_FUNCTION = delete
#endif
//----------------------------------------------------------------------------
#define EXTERN_TEMPLATE_CLASS_DECL(_API) extern template class
#define EXTERN_TEMPLATE_CLASS_DEF(_API) template class _API
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if defined(CPP_VISUALSTUDIO)
#   define PRAGMA_DISABLE_OPTIMIZATION_ACTUAL __pragma(optimize("",off))
#   define PRAGMA_ENABLE_OPTIMIZATION_ACTUAL  __pragma(optimize("",on ))
#elif defined(CPP_CLANG)
#   define PRAGMA_DISABLE_OPTIMIZATION_ACTUAL __pragma(clang optimize off)
#   define PRAGMA_ENABLE_OPTIMIZATION_ACTUAL  __pragma(clang optimize on )
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
#if defined(CPP_VISUALSTUDIO)
#   define PRAGMA_INITSEG_LIB \
    __pragma(warning(disable: 4073)) \
    __pragma(init_seg(lib))
#   define PRAGMA_INITSEG_COMPILER \
    __pragma(warning(disable: 4074)) \
    __pragma(init_seg(compiler))
#else
#   error "need to implement PRAGMA_INITSEG_LIB/COMPILER !"
#endif
//----------------------------------------------------------------------------
#if defined(__clang__) || defined(__GNUC__)
#   define PPE_PRETTY_FUNCTION __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
#   define PPE_PRETTY_FUNCTION __FUNCSIG__
#else
#   error "need to implement PPE_PRETTY_FUNCTION !"
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
typedef i8 byte;
typedef u8 ubyte;
//----------------------------------------------------------------------------
//typedef i16 short;
typedef u16 ushort;
//----------------------------------------------------------------------------
typedef i32 word;
typedef u32 uword;
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs, class = decltype(std::declval<_Lhs>() < std::declval<_Rhs>()) >
CONSTEXPR auto Max(_Lhs a, _Rhs b) NOEXCEPT {
    return (a < b) ? b : a;
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs, class = decltype(std::declval<_Lhs>() < std::declval<_Rhs>()) >
CONSTEXPR auto Min(_Lhs a, _Rhs b) NOEXCEPT {
    return (a < b) ? a : b;
}
//----------------------------------------------------------------------------
template <typename T, typename U, class = decltype(std::declval<T>() < std::declval<U>()) >
CONSTEXPR auto Clamp(T value, U vmin, U vmax) NOEXCEPT {
    return Min(vmax, Max(vmin, value));
}
//----------------------------------------------------------------------------
template <typename _A, typename _B, typename _C, class = decltype(std::declval<_A>() < std::declval<_B>()) >
CONSTEXPR auto Max3(_A a, _B b, _C c) NOEXCEPT { return (a < b ? Max(b, c) : Max(a, c)); }
//----------------------------------------------------------------------------
template <typename _A, typename _B, typename _C, class = decltype(std::declval<_A>() < std::declval<_B>()) >
CONSTEXPR auto Min3(_A a, _B b, _C c) NOEXCEPT { return (a < b ? Min(a, c) : Min(b, c)); }
//----------------------------------------------------------------------------
typedef struct uint128_t {
    u64 lo, hi;

    CONSTEXPR static uint128_t Zero() { return uint128_t{ 0, 0 }; }

    CONSTEXPR friend bool operator ==(const uint128_t& lhs, const uint128_t& rhs) { return ((lhs.hi == rhs.hi) & (lhs.lo == rhs.lo)); }
    CONSTEXPR friend bool operator !=(const uint128_t& lhs, const uint128_t& rhs) { return !operator ==(lhs, rhs); }

    CONSTEXPR friend bool operator < (const uint128_t& lhs, const uint128_t& rhs) { return lhs.hi == rhs.hi ? lhs.lo < rhs.lo : lhs.hi < rhs.hi; }
    CONSTEXPR friend bool operator >=(const uint128_t& lhs, const uint128_t& rhs) { return !operator < (lhs, rhs); }

    friend void swap(uint128_t& lhs, uint128_t& rhs) { std::swap(lhs.lo, rhs.lo); std::swap(lhs.hi, rhs.hi); }

}   u128;
//----------------------------------------------------------------------------
typedef struct uint256_t {
    uint128_t lo, hi;

    CONSTEXPR static uint256_t Zero() { return uint256_t{ uint128_t::Zero(), uint128_t::Zero() }; }

    CONSTEXPR friend bool operator ==(const uint256_t& lhs, const uint256_t& rhs) { return ((lhs.hi == rhs.hi) & (lhs.lo == rhs.lo)); }
    CONSTEXPR friend bool operator !=(const uint256_t& lhs, const uint256_t& rhs) { return !operator ==(lhs, rhs); }

    CONSTEXPR friend bool operator < (const uint256_t& lhs, const uint256_t& rhs) { return lhs.hi == rhs.hi ? lhs.lo < rhs.lo : lhs.hi < rhs.hi; }
    CONSTEXPR friend bool operator >=(const uint256_t& lhs, const uint256_t& rhs) { return !operator < (lhs, rhs); }

    friend void swap(uint256_t& lhs, uint256_t& rhs) { std::swap(lhs.lo, rhs.lo); std::swap(lhs.hi, rhs.hi); }

}   u256;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
