#pragma once

#include "Diagnostic/Exception.h"
#include "HAL/PlatformMacros.h"

#ifndef USE_PPE_ASSERT
#   define USE_PPE_ASSERT (USE_PPE_ASSERTIONS && USE_PPE_DEBUG)
#endif

#ifndef USE_PPE_ASSERT_RELEASE
#   define USE_PPE_ASSERT_RELEASE (!USE_PPE_FINAL_RELEASE && !USE_PPE_PROFILING)
#endif

#define WITH_PPE_ASSERT_DEBUG_SECTION               0 // when enabled assertion failed functions are compiled in an isolated code section // %_NOCOMMIT%
#define WITH_PPE_ASSERT_FALLBACK_TO_ASSUME          1 // when enabled every assert becomes an __assume() when !USE_PPE_DEBUG // %_NOCOMMIT%
#define WITH_PPE_ASSERT_RELEASE_FALLBACK_TO_ASSUME  1 // when enabled every assert release becomes an __assume() when !USE_PPE_FINAL_RELEASE // %_NOCOMMIT%
#define WITH_PPE_ASSERT_WRAP_IN_LAMBDA              0 // when enabled every assert is wrapped inside a lambda, allowing constexpr // %_NOCOMMIT%

#if defined(__INTELLISENSE__) || defined(_PREFAST_)
#   define WITH_PPE_ASSERT_ASSUME_FOR_INTELLISENSE (1) // fall back to assume for Intellisense, so /analyze can understand the predicates
#else
#   define WITH_PPE_ASSERT_ASSUME_FOR_INTELLISENSE (0)
#endif

#if (USE_PPE_ASSERT || USE_PPE_ASSERT_RELEASE) && WITH_PPE_ASSERT_DEBUG_SECTION
    // We'll put all assert implementation code into a separate section in the linked
    // executable. This code should never execute so using a separate section keeps
    // it well off the hot path and hopefully out of the instruction cache. It also
    // facilitates reasoning about the makeup of a compiled/linked binary.
#   define PPE_DEBUG_SECTION PPE_DECLSPEC_CODE_SECTION(".ppedbg")
#else
    // On ARM we can't do this because the executable will require jumps larger
    // than the branch instruction can handle. Clang will only generate
    // the trampolines in the .text segment of the binary. If the ppedbg segment
    // is present it will generate code that it cannot link.
#   define PPE_DEBUG_SECTION
#endif // USE_PPE_ASSERT || USE_PPE_ASSERT_RELEASE

#if defined(NDEBUG) && USE_PPE_ASSERT
#   undef WITH_PPE_ASSERT
#   error "there is someone messing with the project configuration"
#endif

#define PPE_ASSERT_LIGHTWEIGHT_CRASH() (PPE_DEBUG_BREAK(), PPE_DEBUG_CRASH())

#if (USE_PPE_ASSERT || USE_PPE_ASSERT_RELEASE)
#   if WITH_PPE_ASSERT_WRAP_IN_LAMBDA
#       define PPE_ASSERT_LAMBDA_WRAPPER(...) []() NO_INLINE PPE_DEBUG_SECTION { __VA_ARGS__; }()
#   else
#       define PPE_ASSERT_LAMBDA_WRAPPER(...) __VA_ARGS__
#   endif
#endif

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
using FAssertionHandler = bool (*)(const char* msg, const char* file, unsigned line, bool isEnsure);
using FReleaseAssertionHandler = bool (*)(const char* msg, const char* file, unsigned line);
//----------------------------------------------------------------------------
#if USE_PPE_ASSERT
//----------------------------------------------------------------------------
class FAssertException : public FException {
public:
    PPE_CORE_API FAssertException(const char *msg, const char *file, unsigned line);
    PPE_CORE_API virtual ~FAssertException() override;

    const char *File() const { return _file; }
    unsigned Line() const { return _line; }

#if USE_PPE_EXCEPTION_DESCRIPTION
    PPE_CORE_API virtual FTextWriter& Description(FTextWriter& oss) const override final;
#endif

private:
    const char *_file;
    unsigned _line;
};

PPE_CORE_API NO_INLINE void PPE_DEBUG_SECTION AssertionFailed(const char* msg, const char *file, unsigned line);
PPE_CORE_API NO_INLINE void PPE_DEBUG_SECTION EnsureFailed(const char* msg, const char *file, unsigned line);
PPE_CORE_API FAssertionHandler SetAssertionHandler(FAssertionHandler handler) NOEXCEPT;
PPE_CORE_API FAssertionHandler SetAssertionHandlerForCurrentThread(FAssertionHandler handler) NOEXCEPT;

#if WITH_PPE_ASSERT_ASSUME_FOR_INTELLISENSE
#   define AssertMessage(_Message, ...) AnalysisAssume(!!(__VA_ARGS__))
#   define Assert_Lightweight(...) AnalysisAssume(!!(__VA_ARGS__))
#   define AssertMessage_NoAssume(_Message, ...) NOOP()
#   define EnsureMessage(_Message, ...) (Likely(!!(__VA_ARGS__)))
#else
#   define AssertMessage(_Message, ...) \
    ( Likely(!!(__VA_ARGS__)) ? void(0) : PPE_ASSERT_LAMBDA_WRAPPER(::PPE::AssertionFailed(_Message, __FILE__, __LINE__)) )
#   define Assert_Lightweight(...) \
    ( Likely(!!(__VA_ARGS__)) ? void(0) : PPE_ASSERT_LIGHTWEIGHT_CRASH() ) // when we need to break immediately
#   define AssertMessage_NoAssume(_Message, ...) AssertMessage(_Message, COMMA_PROTECT(__VA_ARGS__))
#   define EnsureMessage(_Message, ...) \
    ( Likely(!!(__VA_ARGS__)) ? true : (PPE_ASSERT_LAMBDA_WRAPPER(::PPE::EnsureFailed(_Message, __FILE__, __LINE__)), false) )
#endif

#   define Verify(...) AssertMessage(STRINGIZE(__VA_ARGS__), COMMA_PROTECT(__VA_ARGS__))

//----------------------------------------------------------------------------
#else
//----------------------------------------------------------------------------
inline CONSTEXPR void AssertionFailed(const char *, const char *, unsigned ) {}
inline CONSTEXPR FAssertionHandler SetAssertionHandler(FAssertionHandler) { return nullptr; }
inline CONSTEXPR FAssertionHandler SetAssertionHandlerForCurrentThread(FAssertionHandler) { return nullptr; }

#   if WITH_PPE_ASSERT_ASSUME_FOR_INTELLISENSE
#       define AssertMessage(_Message, ...) AnalysisAssume(!!(__VA_ARGS__))
#   elif WITH_PPE_ASSERT_FALLBACK_TO_ASSUME
#       define AssertMessage(_Message, ...) Assume(!!(__VA_ARGS__))
#   else
#       define AssertMessage(_Message, ...) NOOP()
#   endif

#   define AssertMessage_NoAssume(_Message, ...) NOOP()

#   define Assert_Lightweight(...) NOOP()

#   define EnsureMessage(_Message, ...) ( Assume(!!(__VA_ARGS__)), Likely(!!(__VA_ARGS__)) )

#   define Verify(...) (void)(__VA_ARGS__)

//----------------------------------------------------------------------------
#endif //!WITH_PPE_ASSERT
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#define Assert(...) AssertMessage(STRINGIZE(__VA_ARGS__), COMMA_PROTECT(__VA_ARGS__))
#define Assert_NoAssume(...) AssertMessage_NoAssume(STRINGIZE(__VA_ARGS__), COMMA_PROTECT(__VA_ARGS__))
#define Ensure(...) EnsureMessage(STRINGIZE(__VA_ARGS__), COMMA_PROTECT(__VA_ARGS__))

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if USE_PPE_ASSERT_RELEASE
//----------------------------------------------------------------------------
class FAssertReleaseException : public FException {
public:
    PPE_CORE_API FAssertReleaseException(const char* msg, const char* file, unsigned line);
    virtual ~FAssertReleaseException() override;

    const char *File() const { return _file; }
    unsigned Line() const { return _line; }

#if USE_PPE_EXCEPTION_DESCRIPTION
    PPE_CORE_API virtual FTextWriter& Description(FTextWriter& oss) const override final;
#endif

private:
    const char *_file;
    unsigned _line;
};

PPE_CORE_API NO_INLINE void PPE_DEBUG_SECTION AssertionReleaseFailed(const char* msg, const char *file, unsigned line);
PPE_CORE_API FReleaseAssertionHandler SetAssertionReleaseHandler(FReleaseAssertionHandler handler) NOEXCEPT;
PPE_CORE_API FReleaseAssertionHandler SetAssertionReleaseHandlerForCurrentThread(FReleaseAssertionHandler handler) NOEXCEPT;

NORETURN inline void PPE_DEBUG_SECTION AssertionReleaseFailed_NoReturn(const char* msg, const char* file, unsigned line) {
    AssertionReleaseFailed(msg, file, line);
    abort();
}

#if WITH_PPE_ASSERT_ASSUME_FOR_INTELLISENSE
#   define AssertReleaseMessage(_Message, ...) AnalysisAssume(!!(__VA_ARGS__))
#   define AssertReleaseFailed(_Message) PPE_DEBUG_CRASH()
#   define AssertReleaseMessage_NoAssume(_Message, ...) NOOP()
#else
#   define AssertReleaseMessage(_Message, ...) \
    ( Likely(!!(__VA_ARGS__)) ? void(0) : PPE_ASSERT_LAMBDA_WRAPPER(::PPE::AssertionReleaseFailed(_Message, __FILE__, __LINE__)) )
#   define AssertReleaseFailed(_Message) ::PPE::AssertionReleaseFailed_NoReturn(_Message, __FILE__, __LINE__)
#   define AssertReleaseMessage_NoAssume(_Message, ...) AssertReleaseMessage(_Message, COMMA_PROTECT(__VA_ARGS__))
#endif

#   define VerifyRelease(...) AssertReleaseMessage(STRINGIZE(__VA_ARGS__), COMMA_PROTECT(__VA_ARGS__))

//----------------------------------------------------------------------------
#else
//----------------------------------------------------------------------------
inline CONSTEXPR void AssertionReleaseFailed(const char*, const char*, unsigned ) {}
NORETURN inline void AssertionReleaseFailed_NoReturn(const char*, const char*, unsigned ) { abort(); }
inline CONSTEXPR FReleaseAssertionHandler SetAssertionReleaseHandler(FReleaseAssertionHandler) { return nullptr; }
inline CONSTEXPR FReleaseAssertionHandler SetAssertionReleaseHandlerForCurrentThread(FReleaseAssertionHandler) { return nullptr; }

#   if WITH_PPE_ASSERT_ASSUME_FOR_INTELLISENSE
#       define AssertReleaseMessage(_Message, ...) AnalysisAssume(!!(__VA_ARGS__))
#   elif WITH_PPE_ASSERT_RELEASE_FALLBACK_TO_ASSUME
#       define AssertReleaseMessage(_Message, ...) Assume(!!(__VA_ARGS__))
#   else
#       define AssertReleaseMessage(_Message, ...) NOOP()
#   endif

#   define AssertReleaseMessage_NoAssume(_Message, ...) NOOP()
#   define AssertReleaseFailed(_Message) ::PPE::AssertionReleaseFailed_NoReturn(nullptr, nullptr, 0)

#   define VerifyRelease(...) (void)(__VA_ARGS__)

//----------------------------------------------------------------------------
#endif //!WITH_PPE_ASSERT_RELEASE
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#define AssertRelease(...) AssertReleaseMessage(STRINGIZE(__VA_ARGS__), COMMA_PROTECT(__VA_ARGS__))
#define AssertRelease_NoAssume(...) AssertReleaseMessage_NoAssume(STRINGIZE(__VA_ARGS__), COMMA_PROTECT(__VA_ARGS__))

#define AssertNotReached() AssertReleaseFailed("unreachable state")
#define AssertNotImplemented() AssertReleaseFailed("not implemented")

#if USE_PPE_ASSERT
#   define ARG0_IF_ASSERT(...) __VA_ARGS__
#   define ARGS_IF_ASSERT(...) COMMA __VA_ARGS__
#   define ONLY_IF_ASSERT(...) __VA_ARGS__
#else
#   define ARG0_IF_ASSERT(...)
#   define ARGS_IF_ASSERT(...)
#   define ONLY_IF_ASSERT(...) NOOP()
#endif

#if USE_PPE_ASSERT_RELEASE
#   define ONLY_IF_ASSERT_RELEASE(...) __VA_ARGS__
#else
#   define ONLY_IF_ASSERT_RELEASE(...) NOOP()
#endif

#define AssertFinalMessage(_Message, ...) \
    ( Likely(!!(__VA_ARGS__)) ? void(0) : AssertReleaseFailed(_Message) )
#define AssertFinal(...) AssertFinalMessage(STRINGIZE(__VA_ARGS__), COMMA_PROTECT(__VA_ARGS__))
