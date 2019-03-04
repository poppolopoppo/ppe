#pragma once

#include "Application_fwd.h"

#include "Allocator/PoolAllocatorTag.h"

namespace PPE {
namespace Application {
POOL_TAG_DECL(PPE_APPLICATION_API, Application);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_APPLICATION_API FApplicationContext {
public:
    FApplicationContext();
    ~FApplicationContext();
};
//----------------------------------------------------------------------------
PPE_APPLICATION_API int LaunchApplication(const FApplicationContext& context, class FApplicationBase* app);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
