#pragma once

#include "Core.h"

#include "Diagnostic/Logger_fwd.h"
#include "IO/String_fwd.h"
#include "Memory/MemoryView.h"

#define PPE_HAL_MAKEINCLUDE(_BASENAME) \
    STRINGIZE(HAL/TARGET_PLATFORM/CONCAT(TARGET_PLATFORM, _BASENAME).h)

#define PPE_HAL_MAKEALIAS(_BASENAME) \
    namespace PPE { \
        using CONCAT(F, _BASENAME) = CONCAT3(F, TARGET_PLATFORM, _BASENAME); \
    } //!namespace PPE

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
    virtual ~ITargetPlaftorm() {}

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
PPE_CORE_API TMemoryView<const ITargetPlaftorm* const> AllTargetPlatforms();
PPE_CORE_API const ITargetPlaftorm& TargetPlatform(ETargetPlatform platform);
inline const ITargetPlaftorm& CurrentPlatform() { TargetPlatform(ETargetPlatform::Current); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
