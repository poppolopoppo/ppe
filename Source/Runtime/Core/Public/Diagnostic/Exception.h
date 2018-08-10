#pragma once

#include "IO/TextWriter_fwd.h"

#include <exception>
#include <stdexcept>

#if !(defined(FINAL_RELEASE) || defined(PROFILING_ENABLED))
#   define USE_PPE_EXCEPTION_CALLSTACK 1 //%_NOCOMMIT%
#else
#   define USE_PPE_EXCEPTION_CALLSTACK 0
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

    const char* What() const { return _what; }
#if USE_PPE_EXCEPTION_CALLSTACK
    size_t SiteHash() const { return _siteHash; }
    FDecodedCallstack Callstack() const;
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
