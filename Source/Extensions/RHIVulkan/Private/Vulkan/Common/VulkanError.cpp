// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "RHI_fwd.h"

#include "Vulkan/Common/VulkanError.h"

#include "Diagnostic/Logger.h"
#include "IO/String.h"
#include "IO/StringBuilder.h"
#include "IO/StringView.h"
#include "IO/Format.h"

#include "Vulkan/VulkanIncludes.h"

namespace PPE {
namespace RHI {
EXTERN_LOG_CATEGORY(PPE_RHI_API, RHI)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FVulkanException::FVulkanException(const char* what)
:   FVulkanException(what, VK_SUCCESS)
{}
//----------------------------------------------------------------------------
FVulkanException::FVulkanException(const char* what, long errorCode)
:   FRHIException(what)
,   _errorCode(errorCode) {
#if !USE_PPE_FINAL_RELEASE
    LOG(RHI, Error, L"vulkan exception: {0} failed with {1}", MakeCStringView(what), FVulkanError(_errorCode));
    FLUSH_LOG();
#endif
}
//----------------------------------------------------------------------------
FVulkanException::~FVulkanException() = default;
//----------------------------------------------------------------------------
#if USE_PPE_EXCEPTION_DESCRIPTION
FWTextWriter& FVulkanException::Description(FWTextWriter& oss) const {
    return oss
        << MakeCStringView(What()) << L": "
        << FVulkanError(_errorCode);
}
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char>
static TBasicTextWriter<_Char>& VkResultToTextWriter_(TBasicTextWriter<_Char>& oss, const FVulkanError& error) {
    switch (static_cast<VkResult>(error.Code)) {
    case VK_SUCCESS: return oss << STRING_LITERAL(_Char, "VK_SUCCESS");
    case VK_NOT_READY: return oss << STRING_LITERAL(_Char, "VK_NOT_READY");
    case VK_TIMEOUT: return oss << STRING_LITERAL(_Char, "VK_TIMEOUT");
    case VK_EVENT_SET: return oss << STRING_LITERAL(_Char, "VK_EVENT_SET");
    case VK_EVENT_RESET: return oss << STRING_LITERAL(_Char, "VK_EVENT_RESET");
    case VK_INCOMPLETE: return oss << STRING_LITERAL(_Char, "VK_INCOMPLETE");
    case VK_ERROR_OUT_OF_HOST_MEMORY: return oss << STRING_LITERAL(_Char, "VK_ERROR_OUT_OF_HOST_MEMORY");
    case VK_ERROR_OUT_OF_DEVICE_MEMORY: return oss << STRING_LITERAL(_Char, "VK_ERROR_OUT_OF_DEVICE_MEMORY");
    case VK_ERROR_INITIALIZATION_FAILED: return oss << STRING_LITERAL(_Char, "VK_ERROR_INITIALIZATION_FAILED");
    case VK_ERROR_DEVICE_LOST: return oss << STRING_LITERAL(_Char, "VK_ERROR_DEVICE_LOST");
    case VK_ERROR_MEMORY_MAP_FAILED: return oss << STRING_LITERAL(_Char, "VK_ERROR_MEMORY_MAP_FAILED");
    case VK_ERROR_LAYER_NOT_PRESENT: return oss << STRING_LITERAL(_Char, "VK_ERROR_LAYER_NOT_PRESENT");
    case VK_ERROR_EXTENSION_NOT_PRESENT: return oss << STRING_LITERAL(_Char, "VK_ERROR_EXTENSION_NOT_PRESENT");
    case VK_ERROR_FEATURE_NOT_PRESENT: return oss << STRING_LITERAL(_Char, "VK_ERROR_FEATURE_NOT_PRESENT");
    case VK_ERROR_INCOMPATIBLE_DRIVER: return oss << STRING_LITERAL(_Char, "VK_ERROR_INCOMPATIBLE_DRIVER");
    case VK_ERROR_TOO_MANY_OBJECTS: return oss << STRING_LITERAL(_Char, "VK_ERROR_TOO_MANY_OBJECTS");
    case VK_ERROR_FORMAT_NOT_SUPPORTED: return oss << STRING_LITERAL(_Char, "VK_ERROR_FORMAT_NOT_SUPPORTED");
    case VK_ERROR_FRAGMENTED_POOL: return oss << STRING_LITERAL(_Char, "VK_ERROR_FRAGMENTED_POOL");
    case VK_ERROR_UNKNOWN: return oss << STRING_LITERAL(_Char, "VK_ERROR_UNKNOWN");
    case VK_ERROR_OUT_OF_POOL_MEMORY: return oss << STRING_LITERAL(_Char, "VK_ERROR_OUT_OF_POOL_MEMORY");
    case VK_ERROR_INVALID_EXTERNAL_HANDLE: return oss << STRING_LITERAL(_Char, "VK_ERROR_INVALID_EXTERNAL_HANDLE");
    case VK_ERROR_FRAGMENTATION: return oss << STRING_LITERAL(_Char, "VK_ERROR_FRAGMENTATION");
    case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS: return oss << STRING_LITERAL(_Char, "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS");
    case VK_ERROR_SURFACE_LOST_KHR: return oss << STRING_LITERAL(_Char, "VK_ERROR_SURFACE_LOST_KHR");
    case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: return oss << STRING_LITERAL(_Char, "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR");
    case VK_SUBOPTIMAL_KHR: return oss << STRING_LITERAL(_Char, "VK_SUBOPTIMAL_KHR");
    case VK_ERROR_OUT_OF_DATE_KHR: return oss << STRING_LITERAL(_Char, "VK_ERROR_OUT_OF_DATE_KHR");
    case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR: return oss << STRING_LITERAL(_Char, "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR");
    case VK_ERROR_VALIDATION_FAILED_EXT: return oss << STRING_LITERAL(_Char, "VK_ERROR_VALIDATION_FAILED_EXT");
    case VK_ERROR_INVALID_SHADER_NV: return oss << STRING_LITERAL(_Char, "VK_ERROR_INVALID_SHADER_NV");
    case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT: return oss << STRING_LITERAL(_Char, "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT");
    case VK_ERROR_NOT_PERMITTED_EXT: return oss << STRING_LITERAL(_Char, "VK_ERROR_NOT_PERMITTED_EXT");
    case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT: return oss << STRING_LITERAL(_Char, "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT");
    case VK_THREAD_IDLE_KHR: return oss << STRING_LITERAL(_Char, "VK_THREAD_IDLE_KHR");
    case VK_THREAD_DONE_KHR: return oss << STRING_LITERAL(_Char, "VK_THREAD_DONE_KHR");
    case VK_OPERATION_DEFERRED_KHR: return oss << STRING_LITERAL(_Char, "VK_OPERATION_DEFERRED_KHR");
    case VK_OPERATION_NOT_DEFERRED_KHR: return oss << STRING_LITERAL(_Char, "VK_OPERATION_NOT_DEFERRED_KHR");
    case VK_ERROR_PIPELINE_COMPILE_REQUIRED_EXT: return oss << STRING_LITERAL(_Char, "VK_ERROR_PIPELINE_COMPILE_REQUIRED_EXT");
    case VK_RESULT_MAX_ENUM: return oss << STRING_LITERAL(_Char, "VK_RESULT_MAX_ENUM");
    }

    AssertNotImplemented();
}
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, const FVulkanError& error) {
    return VkResultToTextWriter_(oss, error);
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, const FVulkanError& error) {
    return VkResultToTextWriter_(oss, error);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if USE_PPE_RHIVULKAN_CHECKS
NODISCARD static bool VulkanCheckErrors_( VkResult result,
    const FConstChar& call, const FConstChar& func, const FWStringView& file, u32 line ) NOEXCEPT {
    Unused(call);
    Unused(func);
    Unused(file);
    Unused(line);
    if (Likely(VK_SUCCESS == result))
        return true;

    LOG(RHI, Error, L"vulkan error: got result {0}\n{1}({2}:1): {3}\n\tin function: {4}",
        FVulkanError{ result }, file, line, call, func );

    return false;
}
#endif
//----------------------------------------------------------------------------
#if USE_PPE_RHIVULKAN_CHECKS
PPE_RHIVULKAN_API void VulkanCheckNoErrors( VkResult result,
    const FConstChar& call, const FConstChar& func, const FWStringView& file, u32 line ) {
    if (Unlikely(not VulkanCheckErrors_(result, call, func, file, line)))
        PPE_THROW_IT(FVulkanException(call, static_cast<u32>(result)));
}
#endif
//----------------------------------------------------------------------------
#if USE_PPE_RHIVULKAN_CHECKS
NODISCARD PPE_RHIVULKAN_API bool VulkanCheckIfErrors( VkResult result,
    const FConstChar& call, const FConstChar& func, const FWStringView& file, u32 line ) NOEXCEPT {
    return VulkanCheckErrors_(result, call, func, file, line);
}
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
