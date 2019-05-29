#pragma once

#include "Diagnostic/Exception.h"

#define USE_PPE_ASSERT (USE_PPE_ASSERTIONS && USE_PPE_DEBUG)
#define USE_PPE_ASSERT_RELEASE (!USE_PPE_FINAL_RELEASE && !USE_PPE_PROFILING)

#define WITH_PPE_ASSERT_FALLBACK_TO_ASSUME 1 // when enabled every assert becomes an __assume() when !USE_PPE_DEBUG // %_NOCOMMIT%
#define WITH_PPE_ASSERT_RELEASE_FALLBACK_TO_ASSUME 1 // when enabled every assert release becomes an __assume() when !USE_PPE_FINAL_RELEASE // %_NOCOMMIT%

#if defined(NDEBUG) && USE_PPE_ASSERT
#   undef WITH_PPE_ASSERT
#   error "there is someone messing with the project configuration"
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

#   define AssertMessage(_Message, ...) \
    (void)( (!!(__VA_ARGS__)) || (PPE::AssertionFailed(_Message, WIDESTRING(__FILE__), __LINE__), 0) )

#   define AssertMessage_NoAssume(_Message, ...) AssertMessage(_Message, COMMA_PROTECT(__VA_ARGS__))

#   define Verify(...) AssertMessage(WIDESTRING(#__VA_ARGS__), COMMA_PROTECT(__VA_ARGS__))

//----------------------------------------------------------------------------
#else
//----------------------------------------------------------------------------
inline void AssertionFailed(const wchar_t *, const wchar_t *, unsigned ) {}
inline void SetAssertionHandler(FAssertHandler ) {}

#   if WITH_PPE_ASSERT_FALLBACK_TO_ASSUME
#       define AssertMessage(_Message, ...) Assume(__VA_ARGS__)
#   else
#       define AssertMessage(_Message, ...) NOOP()
#   endif

#   define AssertMessage_NoAssume(_Message, ...) NOOP()

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
inline void NORETURN AssertionReleaseFailed_NoReturn(const wchar_t* msg, const wchar_t *file, unsigned line) { AssertionReleaseFailed(msg, file, line); }
PPE_CORE_API void SetAssertionReleaseHandler(FAssertReleaseHandler handler);

#   define AssertReleaseMessage(_Message, ...) \
    (void)( (!!(__VA_ARGS__)) || (PPE::AssertionReleaseFailed(_Message, WIDESTRING(__FILE__), __LINE__), 0) )

#   define AssertReleaseFailed(_Message) \
    PPE::AssertionReleaseFailed_NoReturn(_Message, WIDESTRING(__FILE__), __LINE__)

#   define AssertReleaseMessage_NoAssume(_Message, ...) AssertReleaseMessage(_Message, COMMA_PROTECT(__VA_ARGS__))

#   define VerifyRelease(...) AssertReleaseMessage(WIDESTRING(#__VA_ARGS__), COMMA_PROTECT(__VA_ARGS__))

//----------------------------------------------------------------------------
#else
//----------------------------------------------------------------------------
inline void AssertionReleaseFailed(const wchar_t*, const wchar_t*, unsigned ) {}
inline void NORETURN AssertionReleaseFailed_NoReturn(const wchar_t*, const wchar_t*, unsigned ) { abort(); }
inline void SetAssertionReleaseHandler(FAssertReleaseHandler ) {}

#   if WITH_PPE_ASSERT_RELEASE_FALLBACK_TO_ASSUME
#       define AssertReleaseMessage(_Message, ...)  Assume(__VA_ARGS__)
#   else
#       define AssertReleaseMessage(_Message, ...)  NOOP()
#   endif

#   define AssertReleaseMessage_NoAssume(_Message, ...)  NOOP()
#   define AssertReleaseFailed(_Message) PPE::AssertionReleaseFailed_NoReturn(nullptr, nullptr, 0)

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
#   define ONLY_IF_ASSERT(_Code) _Code
#else
#   define ONLY_IF_ASSERT(_Code) NOOP()
#endif

#if USE_PPE_ASSERT_RELEASE
#   define ONLY_IF_ASSERT_RELEASE(_Code) _Code
#else
#   define ONLY_IF_ASSERT_RELEASE(_Code) NOOP()
#endif

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
