#pragma once

#include "Core/IO/TextWriter_fwd.h"

#include <exception>
#include <stdexcept>

#if !(defined(FINAL_RELEASE) || defined(PROFILING_ENABLED))
#   define USE_CORE_EXCEPTION_CALLSTACK 1 //%_NOCOMMIT%
#else
#   define USE_CORE_EXCEPTION_CALLSTACK 0
#endif

namespace Core {
#if USE_CORE_EXCEPTION_CALLSTACK
class FDecodedCallstack;
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class CORE_API FException {
public:
    // don't use FStringView to avoid cyclic dependencies in includes
    FException(const char* what) noexcept;

    const char* What() const { return _what; }
#if USE_CORE_EXCEPTION_CALLSTACK
    size_t SiteHash() const { return _siteHash; }
    FDecodedCallstack Callstack() const;
#endif

private:
    const char* _what;
#if USE_CORE_EXCEPTION_CALLSTACK
    size_t _siteHash;
    void* _callstack[8];
#endif
};
//----------------------------------------------------------------------------
class CORE_API FFatalException : FException {
public:
    FFatalException(const char* what) : FException(what) {}
};
//----------------------------------------------------------------------------
CORE_API FTextWriter& operator <<(FTextWriter& oss, const FException& e);
CORE_API FWTextWriter& operator <<(FWTextWriter& oss, const FException& e);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
