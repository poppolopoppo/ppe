
#include "stdafx.h"

#include "Vulkan/VulkanCommon.h"

#include "IO/ConstChar.h"
#include "IO/FormatHelpers.h"
#include "IO/TextWriter.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <typename _Char>
static TBasicTextWriter<_Char>& VendorToTextWriter_(TBasicTextWriter<_Char>& oss, EVulkanVendor vendor) {
    switch (vendor) {
    case EVulkanVendor::AMD: return oss << STRING_LITERAL(_Char, "AMD");
    case EVulkanVendor::ImgTec: return oss << STRING_LITERAL(_Char, "ImgTec");
    case EVulkanVendor::NVIDIA: return oss << STRING_LITERAL(_Char, "NVIDIA");
    case EVulkanVendor::ARM: return oss << STRING_LITERAL(_Char, "ARM");
    case EVulkanVendor::Qualcomm: return oss << STRING_LITERAL(_Char, "Qualcomm");
    case EVulkanVendor::INTEL: return oss << STRING_LITERAL(_Char, "INTEL");
    case EVulkanVendor::Unknown: return oss << STRING_LITERAL(_Char, "Unknown");
    }
    AssertNotImplemented();
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FStringView EVulkanVendor_Name(EVulkanVendor vendor) {
    switch (vendor) {
    case EVulkanVendor::AMD: return "AMD";
    case EVulkanVendor::ImgTec: return "ImgTec";
    case EVulkanVendor::NVIDIA: return "NVIDIA";
    case EVulkanVendor::ARM: return "ARM";
    case EVulkanVendor::Qualcomm: return "Qualcomm";
    case EVulkanVendor::INTEL: return "INTEL";
    default: return "Unknown";
    }
}
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, EVulkanVendor vendor) {
    return VendorToTextWriter_(oss, vendor);
}
FWTextWriter& operator <<(FWTextWriter& oss, EVulkanVendor vendor) {
    return VendorToTextWriter_(oss, vendor);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <typename _Char>
static TBasicTextWriter<_Char>& VersionToTextWriter_(TBasicTextWriter<_Char>& oss, RHI::EVulkanVersion version) {
    return oss
        << STRING_LITERAL(_Char, "VK_API_VERSION_")
        << VK_VERSION_MAJOR(static_cast<u32>(version))
        << STRING_LITERAL(_Char, '_')
        << VK_VERSION_MINOR(static_cast<u32>(version))
        << STRING_LITERAL(_Char, '_')
        << VK_VERSION_PATCH(static_cast<u32>(version));
}
//----------------------------------------------------------------------------
template <typename _Char>
static TBasicTextWriter<_Char>& InstanceExtensionsToTextWriter_(TBasicTextWriter<_Char>& oss, const RHI::FVulkanInstanceExtensionSet& exts) {
    using namespace RHI;
    auto sep = Fmt::NotFirstTime(STRING_LITERAL(_Char, ", "));

    FVulkanInstanceExtensionSet it{ exts };
    for (;;) {
        const auto bit = it.PopFront();
        if (not bit) break;

        const auto ext = static_cast<EVulkanInstanceExtension>(bit - 1);
        const FConstChar ansi{ vk::instance_extension_name(ext) };
        Assert(ansi);

        oss << sep;

        IF_CONSTEXPR(std::is_same_v<_Char, char>)
            oss << ansi.MakeView();
        else
            oss << UTF_8_TO_WCHAR(ansi.MakeView());
    }

    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char>
static TBasicTextWriter<_Char>& DeviceExtensionsToTextWriter_(TBasicTextWriter<_Char>& oss, const RHI::FVulkanDeviceExtensionSet& exts) {
    using namespace RHI;
    auto sep = Fmt::NotFirstTime(STRING_LITERAL(_Char, ", "));

    FVulkanDeviceExtensionSet it{ exts };
    for (;;) {
        const auto bit = it.PopFront();
        if (not bit) break;

        const auto ext = static_cast<EVulkanDeviceExtension>(bit - 1);
        const FConstChar ansi{ vk::device_extension_name(ext) };
        Assert(ansi);

        oss << sep;

        IF_CONSTEXPR(std::is_same_v<_Char, char>)
            oss << ansi.MakeView();
        else
            oss << UTF_8_TO_WCHAR(ansi.MakeView());
    }

    return oss;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, RHI::EVulkanVersion version) {
    return VersionToTextWriter_(oss, version);
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, RHI::EVulkanVersion version) {
    return VersionToTextWriter_(oss, version);
}
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, RHI::EVulkanInstanceExtension ext) {
    const FConstChar name{ vk::instance_extension_name(ext) };
    return oss << name.MakeView();
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, RHI::EVulkanInstanceExtension ext) {
    const FConstChar name{ vk::instance_extension_name(ext) };
    return oss << UTF_8_TO_WCHAR(name.MakeView());
}
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, const RHI::FVulkanInstanceExtensionSet& exts) {
    return InstanceExtensionsToTextWriter_(oss, exts);
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, const RHI::FVulkanInstanceExtensionSet& exts) {
    return InstanceExtensionsToTextWriter_(oss, exts);
}
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, RHI::EVulkanDeviceExtension ext) {
    const FConstChar name{ vk::device_extension_name(ext) };
    return oss << name.MakeView();
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, RHI::EVulkanDeviceExtension ext) {
    const FConstChar name{ vk::device_extension_name(ext) };
    return oss << UTF_8_TO_WCHAR(name.MakeView());
}
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, const RHI::FVulkanDeviceExtensionSet& exts) {
    return DeviceExtensionsToTextWriter_(oss, exts);
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, const RHI::FVulkanDeviceExtensionSet& exts) {
    return DeviceExtensionsToTextWriter_(oss, exts);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
