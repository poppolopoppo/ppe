#pragma once

#include "Core.Engine/Engine.h"

#include "Core/Memory/RefPtr.h"

namespace Core {
struct Guid;

namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class IService;
class IServiceProvider;
//----------------------------------------------------------------------------
#define FWD_ENGINESERVICE(_NameWithoutI) \
    class CONCAT(I, _NameWithoutI); \
    typedef Core::RefPtr<CONCAT(I, _NameWithoutI)> CONCAT(P, _NameWithoutI); \
    typedef Core::RefPtr<const CONCAT(I, _NameWithoutI)> CONCAT(PC, _NameWithoutI); \
    typedef Core::SafePtr<CONCAT(I, _NameWithoutI)> CONCAT(S, _NameWithoutI); \
    typedef Core::SafePtr<const CONCAT(I, _NameWithoutI)> CONCAT(SC, _NameWithoutI); \
    namespace ServicePriority { enum { CONCAT(I, _NameWithoutI) = __LINE__ }; }
//----------------------------------------------------------------------------
// Services from greatest to lowest priority :
//----------------------------------------------------------------------------
FWD_ENGINESERVICE(DeviceEncapsulatorService);
FWD_ENGINESERVICE(RenderSurfaceService);
FWD_ENGINESERVICE(TextureCacheService);
FWD_ENGINESERVICE(EffectCompilerService);
//----------------------------------------------------------------------------
#undef FWD_ENGINESERVICE
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
