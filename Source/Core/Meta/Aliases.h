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
#ifndef _HAS_CXX14
#   define _HAS_CXX14 0
#endif
//----------------------------------------------------------------------------
#ifndef _HAS_CXX17
#   define _HAS_CXX17 0
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifndef not
#   define not !
#endif
//----------------------------------------------------------------------------
#define EXPAND(x) x
//----------------------------------------------------------------------------
#define COMMA ,
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
#define WSTRINGIZE(_X) WIDESTRING(STRINGIZE(_X))
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
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if defined(CPP_CLANG)
#   define NOALIAS
#   define NOEXCEPT     noexcept
#   define NORETURN     __declspec(noreturn)
#   define RESTRICT     __declspec(restrict)
#   define STDCALL      __stdcall
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
#   define THREAD_LOCAL thread_local
#   define STATIC_CONST_INTEGRAL(_TYPE, _NAME, ...) static constexpr _TYPE _NAME = (_TYPE)(__VA_ARGS__)
#   define EMPTY_BASES  __declspec(empty_bases)
#elif defined (CPP_VISUALSTUDIO)
#   define NOALIAS      __declspec(noalias)
#   define NOEXCEPT     __declspec(nothrow)
#   define NORETURN     __declspec(noreturn)
#   define RESTRICT     __declspec(restrict)
#   define STDCALL      __stdcall
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
#define SIMD_INLINE     FORCE_INLINE _vectorcall
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
#if _HAS_CXX17
//  http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0292r2.html
#   define IF_CONSTEXPR(...) if constexpr(__VA_ARGS__)
//  http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n4295.html
#   define FOLD_EXPR(...) ((__VA_ARGS__), ...)
#else
#   define IF_CONSTEXPR if (__VA_ARGS__)
//  Workaround from Jason Turner: https://youtu.be/nnY4e4faNp0?t=39m51s
#   define FOLD_EXPR(...) (void)std::initializer_list<int>{ ((__VA_ARGS__), 0)... }
#endif //!_HAS_CXX17
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifdef USE_CORE_EXCEPTIONS
#    define CORE_THROW()                throw ()
#    define CORE_THROW_IT(_EX)          throw _EX
#    define CORE_THROW_VOID()           throw
#    define CORE_TRY                    try
#    define CORE_CATCH_BLOCK(...)       __VA_ARGS__
#    define CORE_CATCH(_SPEC)           catch(_SPEC)
#else
#    define CORE_THROW()
#    define CORE_THROW_IT(_EX)          CORE_ABORT(STRINGIZE(_EX))
#    define CORE_THROW_VOID()           CORE_ABORT("throw")
#    define CORE_TRY                    if (true)
#    define CORE_CATCH_BLOCK(...)
#    define CORE_CATCH(spec)            if (false)
#endif
//----------------------------------------------------------------------------
#ifdef CPP_VISUALSTUDIO
#   define CORE_THROW_SPECIFIER(_Type)  _THROW1(_Type)
#else
#   define CORE_THROW_SPECIFIER(_Type)
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
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if     defined(CPP_VISUALSTUDIO)
#   define PRAGMA_DISABLE_OPTIMIZATION __pragma(optimize("",off))
#   define PRAGMA_ENABLE_OPTIMIZATION  __pragma(optimize("",on ))
#elif   defined(CPP_CLANG)
#   define PRAGMA_DISABLE_OPTIMIZATION __pragma(clang optimize off)
#   define PRAGMA_ENABLE_OPTIMIZATION  __pragma(clang optimize on )
#else
#   error "need to implement pragma optimize !"
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
#   error "need to implement pragma initseg lib !"
#endif

namespace Core {
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
template <typename _Lhs, typename _Rhs>
constexpr decltype(std::declval<_Lhs>()+std::declval<_Rhs>()) Max(_Lhs a, _Rhs b) { return (a < b) ? b : a; }
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs>
constexpr decltype(std::declval<_Lhs>()+std::declval<_Rhs>()) Min(_Lhs a, _Rhs b) { return (a < b) ? a : b; }
//----------------------------------------------------------------------------
template <typename T, typename _Min, typename _Max>
constexpr decltype(std::declval<T>() + std::declval<_Min>() + std::declval<_Max>()) Clamp(T value, _Min vmin, _Max vmax) {
    return Min(vmax, Max(vmin, value));
}
//----------------------------------------------------------------------------
template <typename _A, typename _B, typename _C>
constexpr decltype(std::declval<_A>() + std::declval<_B>() + std::declval<_C>()) Max3(_A a, _B b, _C c) { return (a < b) ? Max(b, c) : Max(a, c); }
//----------------------------------------------------------------------------
template <typename _A, typename _B, typename _C>
constexpr decltype(std::declval<_A>() + std::declval<_B>() + std::declval<_C>()) Min3(_A a, _B b, _C c) { return (a < b) ? Min(a, c) : Min(b, c); }
//----------------------------------------------------------------------------
typedef struct uint128_t {
    u64 lo, hi;

    static uint128_t Zero() { return uint128_t{ 0, 0 }; }

    friend bool operator ==(const uint128_t& lhs, const uint128_t& rhs) { return lhs.hi == rhs.hi && lhs.lo == rhs.lo; }
    friend bool operator !=(const uint128_t& lhs, const uint128_t& rhs) { return !operator ==(lhs, rhs); }

    friend bool operator < (const uint128_t& lhs, const uint128_t& rhs) { return lhs.hi == rhs.hi ? lhs.lo < rhs.lo : lhs.hi < rhs.hi; }
    friend bool operator >=(const uint128_t& lhs, const uint128_t& rhs) { return !operator < (lhs, rhs); }

    friend void swap(uint128_t& lhs, uint128_t& rhs) { std::swap(lhs.lo, rhs.lo); std::swap(lhs.hi, rhs.hi); }

}   u128;
//----------------------------------------------------------------------------
typedef struct uint256_t {
    uint128_t lo, hi;

    static uint256_t Zero() { return uint256_t{ uint128_t::Zero(), uint128_t::Zero() }; }

    friend bool operator ==(const uint256_t& lhs, const uint256_t& rhs) { return lhs.hi == rhs.hi && lhs.lo == rhs.lo; }
    friend bool operator !=(const uint256_t& lhs, const uint256_t& rhs) { return !operator ==(lhs, rhs); }

    friend bool operator < (const uint256_t& lhs, const uint256_t& rhs) { return lhs.hi == rhs.hi ? lhs.lo < rhs.lo : lhs.hi < rhs.hi; }
    friend bool operator >=(const uint256_t& lhs, const uint256_t& rhs) { return !operator < (lhs, rhs); }

    friend void swap(uint256_t& lhs, uint256_t& rhs) { std::swap(lhs.lo, rhs.lo); std::swap(lhs.hi, rhs.hi); }

}   u256;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
