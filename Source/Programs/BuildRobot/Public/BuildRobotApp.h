#pragma once

#include "Application/ApplicationConsole.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FBuildRobotApp : public Application::FApplicationConsole {
    typedef Application::FApplicationConsole parent_type;
public:
    explicit FBuildRobotApp(FModularDomain& domain);
    ~FBuildRobotApp() override;

    virtual void Start() override;
    virtual bool PumpMessages() NOEXCEPT override;
    virtual void Shutdown() override;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
