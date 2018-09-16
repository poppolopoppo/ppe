#pragma once

#include "Diagnostic/Exception.h"

#if USE_PPE_ASSERTIONS && USE_PPE_DEBUG
#   define WITH_PPE_ASSERT
#endif

#define WITH_PPE_ASSERT_FALLBACK_TO_ASSUME 1 // when enabled every assert becomes an __assume() when !USE_PPE_DEBUG // %_NOCOMMIT%
#define WITH_PPE_ASSERT_RELEASE_FALLBACK_TO_ASSUME 1 // when enabled every assert release becomes an __assume() when !FINAL_RELEASE // %_NOCOMMIT%

#if USE_PPE_ASSERTIONS && !defined(FINAL_RELEASE) && !defined(PROFILING_ENABLED)
#   define WITH_PPE_ASSERT_RELEASE
#endif

#if defined(NDEBUG) && defined(WITH_PPE_ASSERT)
#   undef WITH_PPE_ASSERT
#   error "there is someone messing with the project configuration"
#endif

#define STATIC_ASSERT(...) static_assert(COMMA_PROTECT(__VA_ARGS__), #__VA_ARGS__)

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
typedef bool (*FAssertHandler)(const wchar_t* msg, const wchar_t *file, unsigned line);
//----------------------------------------------------------------------------
#ifdef WITH_PPE_ASSERT
//----------------------------------------------------------------------------
class PPE_CORE_API FAssertException : public FException {
public:
    FAssertException(const char *msg, const wchar_t *file, unsigned line);
    virtual ~FAssertException();

    const wchar_t *File() const { return _file; }
    unsigned Line() const { return _line; }

private:
    const wchar_t *_file;
    unsigned _line;
};

PPE_CORE_API void AssertionFailed(const wchar_t* msg, const wchar_t *file, unsigned line);
PPE_CORE_API void SetAssertionHandler(FAssertHandler handler);

#   define AssertMessage(_Expression, _Message) \
    (void)( (!!(_Expression)) || (PPE::AssertionFailed(_Message, WIDESTRING(__FILE__), __LINE__), 0) )

#   define AssertMessage_NoAssume(_Expression, _Message) AssertMessage(_Expression, _Message)

#   define Verify(...) AssertMessage(COMMA_PROTECT(__VA_ARGS__), WIDESTRING(#__VA_ARGS__))

//----------------------------------------------------------------------------
#else
//----------------------------------------------------------------------------
inline void AssertionFailed(const wchar_t *, const wchar_t *, unsigned ) {}
inline void SetAssertionHandler(FAssertHandler ) {}

#   if WITH_PPE_ASSERT_FALLBACK_TO_ASSUME
#       define AssertMessage(_Expression, _Message) Assume(_Expression)
#   else
#       define AssertMessage(_Expression, _Message) NOOP()
#   endif

#   define AssertMessage_NoAssume(_Expression, _Message) NOOP()

#   define Verify(...) (void)(__VA_ARGS__)

//----------------------------------------------------------------------------
#endif //!WITH_PPE_ASSERT
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#define Assert(...) AssertMessage(COMMA_PROTECT(__VA_ARGS__), WIDESTRING(#__VA_ARGS__))
#define Assert_NoAssume(...) AssertMessage_NoAssume(COMMA_PROTECT(__VA_ARGS__), WIDESTRING(#__VA_ARGS__))

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
typedef bool (*FAssertReleaseHandler)(const wchar_t* msg, const wchar_t* file, unsigned line);
//----------------------------------------------------------------------------
#ifdef WITH_PPE_ASSERT_RELEASE
//----------------------------------------------------------------------------
class PPE_CORE_API FAssertReleaseException : public FException {
public:
    FAssertReleaseException(const char* msg, const wchar_t* file, unsigned line);
    virtual ~FAssertReleaseException();

    const wchar_t *File() const { return _file; }
    unsigned Line() const { return _line; }

private:
    const wchar_t *_file;
    unsigned _line;
};

PPE_CORE_API void AssertionReleaseFailed(const wchar_t* msg, const wchar_t *file, unsigned line);
inline void NORETURN AssertionReleaseFailed_NoReturn(const wchar_t* msg, const wchar_t *file, unsigned line) { AssertionReleaseFailed(msg, file, line); }
PPE_CORE_API void SetAssertionReleaseHandler(FAssertReleaseHandler handler);

#   define AssertReleaseMessage(_Expression, _Message) \
    (void)( (!!(_Expression)) || (PPE::AssertionReleaseFailed(_Message, WIDESTRING(__FILE__), __LINE__), 0) )

#   define AssertReleaseFailed(_Message) \
    PPE::AssertionReleaseFailed_NoReturn(_Message, WIDESTRING(__FILE__), __LINE__)

#   define AssertReleaseMessage_NoAssume(_Expression, _Message) AssertReleaseMessage(_Expression, _Message)

#   define VerifyRelease(...) AssertReleaseMessage(COMMA_PROTECT(__VA_ARGS__), WIDESTRING(#__VA_ARGS__))

//----------------------------------------------------------------------------
#else
//----------------------------------------------------------------------------
inline void AssertionReleaseFailed(const wchar_t*, const wchar_t*, unsigned ) {}
inline void NORETURN AssertionReleaseFailed_NoReturn(const wchar_t*, const wchar_t*, unsigned ) { abort(); }
inline void SetAssertionReleaseHandler(FAssertReleaseHandler ) {}

#   if WITH_PPE_ASSERT_RELEASE_FALLBACK_TO_ASSUME
#       define AssertReleaseMessage(_Expression, _Message)  Assume(_Expression)
#   else
#       define AssertReleaseMessage(_Expression, _Message)  NOOP()
#   endif

#   define AssertReleaseMessage_NoAssume(_Expression, _Message)  NOOP()
#   define AssertReleaseFailed(_Message) PPE::AssertionReleaseFailed_NoReturn(nullptr, nullptr, 0)

#   define VerifyRelease(...) (void)(__VA_ARGS__)

//----------------------------------------------------------------------------
#endif //!WITH_PPE_ASSERT_RELEASE
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#define AssertRelease(...) AssertReleaseMessage(COMMA_PROTECT(__VA_ARGS__), WIDESTRING(#__VA_ARGS__))
#define AssertRelease_NoAssume(...) AssertReleaseMessage_NoAssume(COMMA_PROTECT(__VA_ARGS__), WIDESTRING(#__VA_ARGS__))

#define AssertNotReached() AssertReleaseFailed(L"unreachable state")
#define AssertNotImplemented() AssertReleaseFailed(L"not implemented")

#ifdef WITH_PPE_ASSERT
#   define ONLY_IF_ASSERT(_Code) _Code
#else
#   define ONLY_IF_ASSERT(_Code) NOOP()
#endif

#ifdef WITH_PPE_ASSERT_RELEASE
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
