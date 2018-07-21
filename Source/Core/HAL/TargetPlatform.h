#pragma once

#include "Core/Core.h"

#include "Core/Diagnostic/Logger_fwd.h"
#include "Core/IO/String_fwd.h"
#include "Core/Memory/MemoryView.h"

#define CORE_HAL_MAKEINCLUDE(_BASENAME) \
    STRINGIZE(Core/HAL/TARGET_PLATFORM/CONCAT(TARGET_PLATFORM, _BASENAME).h)

#define CORE_HAL_MAKEALIAS(_BASENAME) \
    namespace Core { \
        using CONCAT(F, _BASENAME) = CONCAT3(F, TARGET_PLATFORM, _BASENAME); \
    } //!namespace Core

namespace Core {
EXTERN_LOG_CATEGORY(CORE_API, HAL)
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
class CORE_API ITargetPlaftorm {
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
CORE_API TMemoryView<const ITargetPlaftorm* const> AllTargetPlatforms();
CORE_API const ITargetPlaftorm& TargetPlatform(ETargetPlatform platform);
inline const ITargetPlaftorm& CurrentPlatform() { TargetPlatform(ETargetPlatform::Current); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
