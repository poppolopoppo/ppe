#pragma once

#include "Vulkan/VulkanCommon.h"

#include "RHIException.h"

#define USE_PPE_RHIVULKAN_CHECKS (!(USE_PPE_FINAL_RELEASE || USE_PPE_PROFILING))

#if USE_PPE_RHIVULKAN_CHECKS

#   define VK_CALL(...) { \
        const VkResult ANONYMIZE(__vkResult) = ( __VA_ARGS__ ); \
        ::PPE::RHI::VulkanCheckNoErrors( \
            ANONYMIZE(__vkResult), \
            WSTRINGIZE(__VA_ARGS__), \
            WSTRINGIZE(EXPAND(PPE_PRETTY_FUNCTION)), \
            WSTRINGIZE(__FILE__),  __LINE__ ); \
    }
#   define VK_CHECK_EX(_RESULT, ...) { \
        const VkResult ANONYMIZE(__vkResult) = ( __VA_ARGS__ ); \
        if (not ::PPE::RHI::VulkanCheckIfErrors( \
            ANONYMIZE(__vkResult), \
            WSTRINGIZE(__VA_ARGS__), \
            WSTRINGIZE(EXPAND(PPE_PRETTY_FUNCTION)), \
            WSTRINGIZE(__FILE__),  __LINE__ )) \
            return (_RESULT); \
    }
#   define VK_CHECK(...) VK_CHECK_EX(::PPE::Default, __VA_ARGS__)

#else

#   define VK_CALL(...) { (void)(__VA_ARGS__); }
#   define VK_CHECK(...) { if ((__VA_ARGS__) != VK_SUCCESS) return ::PPE::Default; }

#endif

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FVulkanError {
    long Code;
    FVulkanError() : Code(0) {}
    FVulkanError(long code) : Code(code) {}
};
//----------------------------------------------------------------------------
class PPE_RHIVULKAN_API FVulkanException : public FRHIException {
protected: // use more specialized exceptions instead
    FVulkanException(const char* what);
public:
    FVulkanException(const char* what, long errorCode);
    virtual ~FVulkanException() override;

    const FVulkanError& ErrorCode() const { return _errorCode; }

#if USE_PPE_EXCEPTION_DESCRIPTION
    virtual FWTextWriter& Description(FWTextWriter& oss) const override final;
#endif

private:
    FVulkanError _errorCode;
};
//----------------------------------------------------------------------------
#if USE_PPE_RHIVULKAN_CHECKS
PPE_RHIVULKAN_API void VulkanCheckNoErrors( VkResult result,
    FWStringView call, FWStringView func, FWStringView file, u32 line );
NODISCARD PPE_RHIVULKAN_API bool VulkanCheckIfErrors( VkResult result,
    FWStringView call, FWStringView func, FWStringView file, u32 line ) NOEXCEPT;
#endif
//----------------------------------------------------------------------------
PPE_RHIVULKAN_API FTextWriter& operator <<(FTextWriter& oss, const FVulkanError& error);
PPE_RHIVULKAN_API FWTextWriter& operator <<(FWTextWriter& oss, const FVulkanError& error);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
