#pragma once

#include "Core_fwd.h"

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
    virtual PPE::FTextWriter& Description(FTextWriter& oss) const override final { \
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
class FException : public std::exception {
public:
    // don't use FStringView to avoid cyclic dependencies in includes
    PPE_CORE_API FException(const char* what) noexcept;
    virtual ~FException() noexcept override = default;

    FException(const FException& ) noexcept = default;
    FException(FException&& ) noexcept = default;

    FException& operator =(const FException& ) noexcept = default;
    FException& operator =(FException&& ) noexcept = default;

    const char* What() const { return std::exception::what(); }
#if USE_PPE_EXCEPTION_CALLSTACK
    size_t SiteHash() const { return _siteHash; }
    PPE_CORE_API FDecodedCallstack Callstack() const;
#endif

#if USE_PPE_EXCEPTION_DESCRIPTION
    virtual FTextWriter& Description(FTextWriter& oss) const = 0;
    PPE_CORE_API static FTextWriter& DefaultDescription(FTextWriter& oss, const wchar_t* name, const FException& e);
#endif

private:
#if USE_PPE_EXCEPTION_CALLSTACK
    size_t _siteHash;
    void* _callstack[8];
#endif
};
//----------------------------------------------------------------------------
class FFatalException : FException {
public:
    FFatalException(const char* what) : FException(what) {}
};
//----------------------------------------------------------------------------
PPE_CORE_API FTextWriter& operator <<(FTextWriter& oss, const FException& e);
PPE_CORE_API FTextWriter& operator <<(FTextWriter& oss, const FException& e);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
