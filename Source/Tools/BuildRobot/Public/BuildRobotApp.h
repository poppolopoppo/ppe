#pragma once

#include "ApplicationConsole.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FBuildRobotApp : public Application::FApplicationConsole {
    typedef Application::FApplicationConsole parent_type;
public:
    FBuildRobotApp();
    ~FBuildRobotApp();

    virtual void Start() override;
    virtual void Shutdown() override;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE