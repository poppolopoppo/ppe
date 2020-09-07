#pragma once

#include "ApplicationConsole.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FBuildRobotApp : public Application::FApplicationConsole {
    typedef Application::FApplicationConsole parent_type;
public:
    explicit FBuildRobotApp(const FModularDomain& domain);
    ~FBuildRobotApp();

    virtual void Start() override;
    virtual bool PumpMessages() NOEXCEPT override;
    virtual void Shutdown() override;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
