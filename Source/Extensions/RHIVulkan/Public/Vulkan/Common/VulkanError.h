#pragma once

#include "Vulkan/VulkanCommon.h"

#include "RHIException.h"

#define USE_PPE_RHIVULKAN_CHECKS (!(USE_PPE_FINAL_RELEASE || USE_PPE_PROFILING))

#if USE_PPE_RHIVULKAN_CHECKS

#   include "IO/StaticString.h"

#   define VK_CALL(...) do { \
        const VkResult ANONYMIZE(__vkResult) = ( __VA_ARGS__ ); \
        ::PPE::RHI::VulkanCheckNoErrors( \
            ANONYMIZE(__vkResult), \
            STRINGIZE(__VA_ARGS__), \
            PPE_PRETTY_FUNCTION, \
            WIDESTRING(__FILE__),  __LINE__ ); \
    } while(0)
#   define VK_CHECK_EX(_RESULT, ...) do { \
        const VkResult ANONYMIZE(__vkResult) = ( __VA_ARGS__ ); \
        if (not ::PPE::RHI::VulkanCheckIfErrors( \
            ANONYMIZE(__vkResult), \
            STRINGIZE(__VA_ARGS__), \
            PPE_PRETTY_FUNCTION, \
            WIDESTRING(__FILE__),  __LINE__ )) \
            return (_RESULT); \
    } while (0)
#   define VK_CHECK(...) VK_CHECK_EX(::PPE::Default, __VA_ARGS__)

#else

#   define VK_CALL(...) (void)(__VA_ARGS__)
#   define VK_CHECK(...) do { if ((__VA_ARGS__) != VK_SUCCESS) return ::PPE::Default; } while (0)

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
class FVulkanException : public FRHIException {
protected: // use more specialized exceptions instead
    PPE_RHIVULKAN_API FVulkanException(const char* what);
public:
    PPE_RHIVULKAN_API FVulkanException(const char* what, long errorCode);

    const FVulkanError& ErrorCode() const { return _errorCode; }

#if USE_PPE_EXCEPTION_DESCRIPTION
    PPE_RHIVULKAN_API virtual FTextWriter& Description(FTextWriter& oss) const override final;
#endif

private:
    FVulkanError _errorCode;
};
//----------------------------------------------------------------------------
#if USE_PPE_RHIVULKAN_CHECKS
PPE_RHIVULKAN_API void VulkanCheckNoErrors( VkResult result,
    const FConstChar& call, const FConstChar& func, FWStringLiteral file, u32 line );
NODISCARD PPE_RHIVULKAN_API bool VulkanCheckIfErrors( VkResult result,
    const FConstChar& call, const FConstChar& func, FWStringLiteral file, u32 line ) NOEXCEPT;
#endif
//----------------------------------------------------------------------------
PPE_RHIVULKAN_API FTextWriter& operator <<(FTextWriter& oss, const FVulkanError& error);
PPE_RHIVULKAN_API FWTextWriter& operator <<(FWTextWriter& oss, const FVulkanError& error);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
