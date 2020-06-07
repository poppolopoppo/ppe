#pragma once

#include "RHI_fwd.h"

#include "Meta/StronglyTyped.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EGenericColorSpace;
enum class EGenericPixelFormat;
struct FGenericPixelInfo;
struct FGenericSurfaceFormat;
//----------------------------------------------------------------------------
struct FGenericInstance;
enum class EGenericPhysicalDeviceFlags;
PPE_STRONGLYTYPED_NUMERIC_DEF(void*, FGenericWindowHandle);
PPE_STRONGLYTYPED_NUMERIC_DEF(void*, FGenericWindowSurface);
//----------------------------------------------------------------------------
class FGenericSwapChain;
//----------------------------------------------------------------------------
class FGenericDevice;
enum class EGenericPresentMode;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
