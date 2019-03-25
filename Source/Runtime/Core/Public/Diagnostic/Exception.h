#pragma once

#include "IO/TextWriter_fwd.h"

#include <exception>
#include <stdexcept>

#if !(USE_PPE_FINAL_RELEASE || USE_PPE_PROFILING)
#   define USE_PPE_EXCEPTION_CALLSTACK 1 //%_NOCOMMIT%
#else
#   define USE_PPE_EXCEPTION_CALLSTACK 0
#endif

#if !USE_PPE_FINAL_RELEASE
#   define USE_PPE_EXCEPTION_DESCRIPTION 1 //%_NOCOMMIT%
#else
#   define USE_PPE_EXCEPTION_DESCRIPTION 0
#endif

#if USE_PPE_EXCEPTION_DESCRIPTION
#   define PPE_DEFAULT_EXCEPTION_DESCRIPTION(_Name) \
    virtual PPE::FWTextWriter& Description(FWTextWriter& oss) const override final { \
        return PPE::FException::DefaultDescription(oss, WSTRINGIZE(_Name), *this); \
    }
#else
#   define PPE_DEFAULT_EXCEPTION_DESCRIPTION(_Name)
#endif

namespace PPE {
#if USE_PPE_EXCEPTION_CALLSTACK
class FDecodedCallstack;
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_CORE_API FException {
public:
    // don't use FStringView to avoid cyclic dependencies in includes
    FException(const char* what) noexcept;
    virtual ~FException();

    const char* What() const { return _what; }
#if USE_PPE_EXCEPTION_CALLSTACK
    size_t SiteHash() const { return _siteHash; }
    FDecodedCallstack Callstack() const;
#endif

#if USE_PPE_EXCEPTION_DESCRIPTION
    virtual FWTextWriter& Description(FWTextWriter& oss) const = 0;
    static FWTextWriter& DefaultDescription(FWTextWriter& oss, const wchar_t* name, const FException& e);
#endif

private:
    const char* _what;
#if USE_PPE_EXCEPTION_CALLSTACK
    size_t _siteHash;
    void* _callstack[8];
#endif
};
//----------------------------------------------------------------------------
class PPE_CORE_API FFatalException : FException {
public:
    FFatalException(const char* what) : FException(what) {}
};
//----------------------------------------------------------------------------
PPE_CORE_API FTextWriter& operator <<(FTextWriter& oss, const FException& e);
PPE_CORE_API FWTextWriter& operator <<(FWTextWriter& oss, const FException& e);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
