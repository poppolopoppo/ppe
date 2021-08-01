#pragma once

#include "Core_fwd.h"

#include "HAL/TargetPlatform_fwd.h"

#include "Diagnostic/Logger_fwd.h"
#include "IO/String_fwd.h"
#include "Memory/MemoryView.h"

namespace PPE {
EXTERN_LOG_CATEGORY(PPE_CORE_API, HAL)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class ETargetPlatform {
    Windows         = 0,
    Linux,
    MacOS,

    _Count,

    Current         = TARGET_PLATFORM,
};
//----------------------------------------------------------------------------
enum class EPlatformFeature {
    Client          = 1<<0,
    Server          = 1<<1,
    Editor          = 1<<2,
    DataGeneration  = 1<<3,
    HighQuality     = 1<<4,
    CookedData      = 1<<5,
};
//----------------------------------------------------------------------------
class PPE_CORE_API ITargetPlaftorm {
public:
    ITargetPlaftorm() = default;
    virtual ~ITargetPlaftorm() = default;

    ITargetPlaftorm(const ITargetPlaftorm&) = delete;
    ITargetPlaftorm& operator =(const ITargetPlaftorm&) = delete;

    virtual ETargetPlatform Platform() const = 0;

    virtual FString DisplayName() const = 0;
    virtual FString FullName() const = 0;
    virtual FString ShortName() const = 0;

    virtual bool RequiresFeature(EPlatformFeature feature) const = 0;
    virtual bool SupportsFeature(EPlatformFeature feature) const = 0;
};
//----------------------------------------------------------------------------
inline const ITargetPlaftorm& CurrentPlatform() { return TargetPlatform(ETargetPlatform::Current); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
