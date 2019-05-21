#pragma once

#include "Application_fwd.h"

namespace PPE {
namespace Application {
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
