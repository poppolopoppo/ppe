#pragma once

#include "RHI_fwd.h"

#include "RHIException.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FVulkanError {
    long Code;
    FVulkanError() : Code(0) {}
    FVulkanError(long code) : Code(code) {}
};
//----------------------------------------------------------------------------
class PPE_RHI_API FVulkanException : public FRHIException {
protected: // use more specialized exceptions instead
    FVulkanException(const char* what);
    FVulkanException(const char* what, long errorCode);
public:
    virtual ~FVulkanException();

    const FVulkanError& ErrorCode() const { return _errorCode; }

#if USE_PPE_EXCEPTION_DESCRIPTION
    virtual FWTextWriter& Description(FWTextWriter& oss) const override final;
#endif

private:
    FVulkanError _errorCode;
};
//----------------------------------------------------------------------------
PPE_RHI_API FTextWriter& operator <<(FTextWriter& oss, const FVulkanError& error);
PPE_RHI_API FWTextWriter& operator <<(FWTextWriter& oss, const FVulkanError& error);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RHI_API FVulkanDeviceException : public FVulkanException {
public:
    FVulkanDeviceException(const char* what, long errorCode) : FVulkanException(what, errorCode) {}
};
//----------------------------------------------------------------------------
class PPE_RHI_API FVulkanGroupException : public FVulkanException {
public:
    FVulkanGroupException(const char* what, long errorCode) : FVulkanException(what, errorCode) {}
};
//----------------------------------------------------------------------------
class PPE_RHI_API FVulkanInstanceException : public FVulkanException {
public:
    FVulkanInstanceException(const char* what, long errorCode) : FVulkanException(what, errorCode) {}
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
