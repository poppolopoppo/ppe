﻿#pragma once

#include "Core_fwd.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class ETargetPlatform;
//----------------------------------------------------------------------------
enum class EPlatformFeature;
//----------------------------------------------------------------------------
class PPE_CORE_API ITargetPlaftorm;
//----------------------------------------------------------------------------
PPE_CORE_API TMemoryView<const ITargetPlaftorm* const> AllTargetPlatforms();
PPE_CORE_API const ITargetPlaftorm& TargetPlatform(ETargetPlatform platform);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE