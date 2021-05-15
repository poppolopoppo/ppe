#pragma once

#include "Diagnostic/Exception.h"

#define USE_PPE_ASSERT (USE_PPE_ASSERTIONS && USE_PPE_DEBUG)
#define USE_PPE_ASSERT_RELEASE (!USE_PPE_FINAL_RELEASE && !USE_PPE_PROFILING)

#define WITH_PPE_ASSERT_FALLBACK_TO_ASSUME 1 // when enabled every assert becomes an __assume() when !USE_PPE_DEBUG // %_NOCOMMIT%
#define WITH_PPE_ASSERT_RELEASE_FALLBACK_TO_ASSUME 1 // when enabled every assert release becomes an __assume() when !USE_PPE_FINAL_RELEASE // %_NOCOMMIT%

#if defined(__INTELLISENSE__) || defined(_PREFAST_)
#   define WITH_PPE_ASSERT_ASSUME_FOR_INTELLISENSE (1) // fall back to assume for Intellisense, so /analyze can understand the predicates
#else
#   define WITH_PPE_ASSERT_ASSUME_FOR_INTELLISENSE (0)
#endif

#if defined(NDEBUG) && USE_PPE_ASSERT
#   undef WITH_PPE_ASSERT
#   error "there is someone messing with the project configuration"
#endif

// #TODO: this is a dirt hack, must define PPE_DEBUG_CRASH macro earlier
#ifndef PPE_DEBUG_CRASH
#   define PPE_ASSERT_LIGHTWEIHGT_CRASH() ::abort()
#else
#   define PPE_ASSERT_LIGHTWEIHGT_CRASH() PPE_DEBUG_CRASH()
#endif

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
typedef bool (*FAssertHandler)(const wchar_t* msg, const wchar_t *file, unsigned line);
//----------------------------------------------------------------------------
#if USE_PPE_ASSERT
//----------------------------------------------------------------------------
class PPE_CORE_API FAssertException : public FException {
public:
    FAssertException(const char *msg, const wchar_t *file, unsigned line);
    virtual ~FAssertException();

    const wchar_t *File() const { return _file; }
    unsigned Line() const { return _line; }

#if USE_PPE_EXCEPTION_DESCRIPTION
    virtual FWTextWriter& Description(FWTextWriter& oss) const override final;
#endif

private:
    const wchar_t *_file;
    unsigned _line;
};

PPE_CORE_API void AssertionFailed(const wchar_t* msg, const wchar_t *file, unsigned line);
PPE_CORE_API void SetAssertionHandler(FAssertHandler handler);

#if WITH_PPE_ASSERT_ASSUME_FOR_INTELLISENSE
#   define AssertMessage(_Message, ...) AnalysisAssume(!!(__VA_ARGS__))
#   define Assert_Lightweight(...) AnalysisAssume(!!(__VA_ARGS__))
#   define AssertMessage_NoAssume(_Message, ...) NOOP()
#else
#   define AssertMessage(_Message, ...) \
    ( Likely(!!(__VA_ARGS__)) ? void(0) : []{ ::PPE::AssertionFailed(_Message, WIDESTRING(__FILE__), __LINE__); }() )
#   define Assert_Lightweight(...) \
    ( Likely(!!(__VA_ARGS__)) ? void(0) : PPE_ASSERT_LIGHTWEIHGT_CRASH() ) // when we need to break immediately
#   define AssertMessage_NoAssume(_Message, ...) AssertMessage(_Message, COMMA_PROTECT(__VA_ARGS__))
#endif

#   define Verify(...) AssertMessage(WIDESTRING(#__VA_ARGS__), COMMA_PROTECT(__VA_ARGS__))

//----------------------------------------------------------------------------
#else
//----------------------------------------------------------------------------
inline CONSTEXPR void AssertionFailed(const wchar_t *, const wchar_t *, unsigned ) {}
inline CONSTEXPR void SetAssertionHandler(FAssertHandler ) {}

#   if WITH_PPE_ASSERT_ASSUME_FOR_INTELLISENSE
#       define AssertMessage(_Message, ...) AnalysisAssume(!!(__VA_ARGS__))
#   elif WITH_PPE_ASSERT_FALLBACK_TO_ASSUME
#       define AssertMessage(_Message, ...) Assume(!!(__VA_ARGS__))
#   else
#       define AssertMessage(_Message, ...) NOOP()
#   endif

#   define AssertMessage_NoAssume(_Message, ...) NOOP()

#   define Assert_Lightweight(...) NOOP()

#   define Verify(...) (void)(__VA_ARGS__)

//----------------------------------------------------------------------------
#endif //!WITH_PPE_ASSERT
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#define Assert(...) AssertMessage(WSTRINGIZE(__VA_ARGS__), COMMA_PROTECT(__VA_ARGS__))
#define Assert_NoAssume(...) AssertMessage_NoAssume(WSTRINGIZE(__VA_ARGS__), COMMA_PROTECT(__VA_ARGS__))

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
typedef bool (*FAssertReleaseHandler)(const wchar_t* msg, const wchar_t* file, unsigned line);
//----------------------------------------------------------------------------
#if USE_PPE_ASSERT_RELEASE
//----------------------------------------------------------------------------
class PPE_CORE_API FAssertReleaseException : public FException {
public:
    FAssertReleaseException(const char* msg, const wchar_t* file, unsigned line);
    virtual ~FAssertReleaseException();

    const wchar_t *File() const { return _file; }
    unsigned Line() const { return _line; }

#if USE_PPE_EXCEPTION_DESCRIPTION
    virtual FWTextWriter& Description(FWTextWriter& oss) const override final;
#endif

private:
    const wchar_t *_file;
    unsigned _line;
};

PPE_CORE_API void AssertionReleaseFailed(const wchar_t* msg, const wchar_t *file, unsigned line);
PPE_CORE_API void SetAssertionReleaseHandler(FAssertReleaseHandler handler);

NORETURN inline void AssertionReleaseFailed_NoReturn(const wchar_t* msg, const wchar_t* file, unsigned line) {
    AssertionReleaseFailed(msg, file, line);
    abort();
}

#if WITH_PPE_ASSERT_ASSUME_FOR_INTELLISENSE
#   define AssertReleaseMessage(_Message, ...) AnalysisAssume(!!(__VA_ARGS__))
#   define AssertReleaseFailed(_Message) PPE_DEBUG_CRASH()
#   define AssertReleaseMessage_NoAssume(_Message, ...) NOOP()
#else
#   define AssertReleaseMessage(_Message, ...) \
    ( Likely(!!(__VA_ARGS__)) ? void(0) : []{ ::PPE::AssertionReleaseFailed(_Message, WIDESTRING(__FILE__), __LINE__); }() )
#   define AssertReleaseFailed(_Message) \
    ::PPE::AssertionReleaseFailed_NoReturn(_Message, WIDESTRING(__FILE__), __LINE__)
#   define AssertReleaseMessage_NoAssume(_Message, ...) AssertReleaseMessage(_Message, COMMA_PROTECT(__VA_ARGS__))
#endif

#   define VerifyRelease(...) AssertReleaseMessage(WIDESTRING(#__VA_ARGS__), COMMA_PROTECT(__VA_ARGS__))

//----------------------------------------------------------------------------
#else
//----------------------------------------------------------------------------
inline CONSTEXPR void AssertionReleaseFailed(const wchar_t*, const wchar_t*, unsigned ) {}
NORETURN inline void AssertionReleaseFailed_NoReturn(const wchar_t*, const wchar_t*, unsigned ) { abort(); }
inline CONSTEXPR void SetAssertionReleaseHandler(FAssertReleaseHandler ) {}

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

#define AssertRelease(...) AssertReleaseMessage(WSTRINGIZE(__VA_ARGS__), COMMA_PROTECT(__VA_ARGS__))
#define AssertRelease_NoAssume(...) AssertReleaseMessage_NoAssume(WSTRINGIZE(__VA_ARGS__), COMMA_PROTECT(__VA_ARGS__))

#define AssertNotReached() AssertReleaseFailed(L"unreachable state")
#define AssertNotImplemented() AssertReleaseFailed(L"not implemented")

#if USE_PPE_ASSERT
#   define ARG0_IF_ASSERT(...) __VA_ARGS__
#   define ARGS_IF_ASSERT(...) COMMA __VA_ARGS__
#   define ONLY_IF_ASSERT(_Code) _Code
#else
#   define ARG0_IF_ASSERT(...)
#   define ARGS_IF_ASSERT(...)
#   define ONLY_IF_ASSERT(_Code) NOOP()
#endif

#if USE_PPE_ASSERT_RELEASE
#   define ONLY_IF_ASSERT_RELEASE(_Code) _Code
#else
#   define ONLY_IF_ASSERT_RELEASE(_Code) NOOP()
#endif

#define AssertFinalMessage(_Message, ...) \
    ( Likely(!!(__VA_ARGS__)) ? void(0) : AssertReleaseFailed(_Message) )
#define AssertFinal(...) AssertFinalMessage(WSTRINGIZE(__VA_ARGS__), COMMA_PROTECT(__VA_ARGS__))

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
void Copy_AssertEquals(T *dst, const T& src, const T& old) {
    Assert(dst && *dst == old);
    *dst = src;
}
//----------------------------------------------------------------------------
template <typename T>
void Move_AssertEquals(T *dst, T&& src, const T& old) {
    Assert(dst && *dst == old);
    *dst = std::move(src);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
