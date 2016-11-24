#pragma once

#include <stdio.h>
#include <tchar.h>

#include <algorithm>
#include <atomic>
#include <memory>
#include <type_traits>

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if     defined(_MSC_VER)
#   define OS_WINDOWS
#   define CPP_VISUALSTUDIO
#elif   defined(__GNUC__)
#   define CPP_GCC
#   define OS_LINUX
#elif   defined(__CLANG__)
#   define CPP_CLANG
#else
#   error "unsupported compiler"
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if     defined(ARCH_X64)
#   ifndef _M_X64
#       error "invalid architecture"
#   endif
#elif   defined(ARCH_X86)
#   ifndef _M_IX86
#       error "invalid architecture"
#   endif
#else
#   error "unknown architecture !"
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// VS2013 and VS2015 are still not C++-11 compliant ...
//static_assert(__cplusplus > 199711L, "Program requires C++11 capable compiler");
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifndef not
#   define not !
#endif
//----------------------------------------------------------------------------
#define COMMA ,
#define COMMA_PROTECT(...) __VA_ARGS__
//----------------------------------------------------------------------------
#define STRINGIZE_2(_X) # _X
#define STRINGIZE_1(_X) STRINGIZE_2(_X)
#define STRINGIZE_0(_X) STRINGIZE_1(_X)
#define STRINGIZE(_X) STRINGIZE_0(_X)
//----------------------------------------------------------------------------
#define WIDESTRING_2(_X) L ## _X
#define WIDESTRING_1(_X) WIDESTRING_2(_X)
#define WIDESTRING_0(_X) WIDESTRING_1(_X)
#define WIDESTRING(_X) WIDESTRING_0(_X)
//----------------------------------------------------------------------------
#define WSTRINGIZE(_X) WIDESTRING(STRINGIZE(_X))
//----------------------------------------------------------------------------
#define CONCAT_I(_X, _Y) _X ## _Y
#define CONCAT_OO(_ARGS) CONCAT_I ## _ARGS
#define CONCAT(_X, _Y) CONCAT_I(_X, _Y) //CONCAT_OO((_X, _Y))
//----------------------------------------------------------------------------
#define NOOP(...)   __noop(__VA_ARGS__)
#define UNUSED(x)   (void)(x)
//----------------------------------------------------------------------------
#define lengthof(_ARRAY) ((sizeof(_ARRAY))/(sizeof(_ARRAY[0])))
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if defined(__clang__)
#   define NOALIAS
#   define NOEXCEPT     noexcept
#   define RESTRICT     __declspec(restrict)
#   define STDCALL      __stdcall
#   define THREAD_LOCAL thread_local
#   define STATIC_CONST_INTEGRAL(_TYPE, _NAME, _DEFAULT_VALUE) static constexpr _TYPE _NAME = (_TYPE)(_DEFAULT_VALUE)
#elif defined(_MSC_VER) && _MSC_VER >= 1900
#   define NOALIAS      __declspec(noalias)
#   define NOEXCEPT     noexcept
#   define RESTRICT     __declspec(restrict)
#   define STDCALL      __stdcall
#   define THREAD_LOCAL thread_local
#   define STATIC_CONST_INTEGRAL(_TYPE, _NAME, _DEFAULT_VALUE) static constexpr _TYPE _NAME = (_TYPE)(_DEFAULT_VALUE)
#elif defined (_MSC_VER)
#   define NOALIAS      __declspec(noalias)
#   define NOEXCEPT     __declspec(nothrow)
#   define RESTRICT     __declspec(restrict)
#   define STDCALL      __stdcall
#   define THREAD_LOCAL __declspec(thread)
#   define STATIC_CONST_INTEGRAL(_TYPE, _NAME, _DEFAULT_VALUE) enum : _TYPE { _NAME = (_TYPE)(_DEFAULT_VALUE) }
#else
#   error "unsupported compiler"
#endif
#define FORCE_INLINE    __forceinline
#define NO_INLINE       __declspec(noinline)
//----------------------------------------------------------------------------
#ifdef _MSC_VER
#   define Assume(expr) __assume((expr))
#elif ((__GNUC__ * 100 + __GNUC_MINOR__) >= 302) || (__INTEL_COMPILER >= 800) || defined(__clang__)
#   define Assume(expr)    (__builtin_expect ((expr),1) )
#else
#   error "unsupported compiler"
#endif
#define Likely(expr)     Assume((expr))
#define Unlikely(expr)   Assume(!(expr))
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define CORE_ENABLE_EXCEPTIONS
//----------------------------------------------------------------------------
#ifdef CORE_ENABLE_EXCEPTIONS
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
#if     defined(STATIC_LINK)
#   define DLL_IMPORT
#   define DLL_EXPORT
#elif   defined(DYNAMIC_LINK)
#   if     defined(CPP_VISUALSTUDIO)
#       define DLL_IMPORT __declspec(dllimport)
#       define DLL_EXPORT __declspec(dllexport)
#   elif   defined(CPP_CLANG)
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

#ifdef EXPORT_CORE
#   define CORE_API DLL_EXPORT
#else
#   define CORE_API DLL_IMPORT
#endif

#include "Core/Meta/Aliases.h"
#include "Core/Meta/Alignment.h"
#include "Core/Meta/Assert.h"
#include "Core/Meta/BitCount.h"
#include "Core/Meta/Cast.h"
#include "Core/Meta/Delete.h"
#include "Core/Meta/Enum.h"
#include "Core/Meta/ForRange.h"
#include "Core/Meta/Hash_fwd.h"
#include "Core/Meta/Iterator.h"
#include "Core/Meta/NumericLimits.h"
#include "Core/Meta/OneTimeInitialize.h"
#include "Core/Meta/StronglyTyped.h"
#include "Core/Meta/ThreadResource.h"
#include "Core/Meta/TypeTraits.h"
#include "Core/Meta/Warnings.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// CoreStartup is the entry and exit point encapsulating every call to Core::.
// Constructed with the same lifetime than the program (or application if segregated).
//----------------------------------------------------------------------------
class CORE_API CoreStartup {
public:
    static void Start(void *applicationHandle, int nShowCmd, size_t argc, const wchar_t** argv);
    static void Shutdown();

    static void ClearAll_UnusedMemory();

    CoreStartup(void *applicationHandle, int nShowCmd, size_t argc, const wchar_t** argv) {
        Start(applicationHandle, nShowCmd, argc, argv);
    }

    ~CoreStartup() {
        Shutdown();
    }
};
//----------------------------------------------------------------------------
// Called for each module on start
#ifndef FINAL_RELEASE
struct CORE_API OnModuleStart {
    const wchar_t* const ModuleName;
    OnModuleStart(const wchar_t* moduleName);
    ~OnModuleStart();
};
#define CORE_MODULE_START(_Name) \
    const Core::OnModuleStart onModuleStart(WSTRINGIZE(_Name))
#else
#define CORE_MODULE_START(_Name) NOOP()
#endif
//----------------------------------------------------------------------------
// Called for each module on shutdown
#ifndef FINAL_RELEASE
struct CORE_API OnModuleShutdown {
    const wchar_t* const ModuleName;
    OnModuleShutdown(const wchar_t* moduleName);
    ~OnModuleShutdown();
};
#define CORE_MODULE_SHUTDOWN(_Name) \
    const Core::OnModuleShutdown onModuleShutdown(WSTRINGIZE(_Name))
#else
#define CORE_MODULE_SHUTDOWN(_Name) NOOP()
#endif
//----------------------------------------------------------------------------
// Called for each module on ClearAll_UnusedMemory
#ifndef FINAL_RELEASE
struct CORE_API OnModuleClearAll {
    const wchar_t* const ModuleName;
    OnModuleClearAll(const wchar_t* moduleName);
    ~OnModuleClearAll();
};
#define CORE_MODULE_CLEARALL(_Name) \
    const Core::OnModuleClearAll onModuleClearAll(WSTRINGIZE(_Name))
#else
#define CORE_MODULE_CLEARALL(_Name) NOOP()
#endif
//----------------------------------------------------------------------------
#ifndef FINAL_RELEASE
void CORE_API CheckMemory();
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
