#pragma once

#include "ApplicationConsole.h"

namespace PPE {
namespace Test {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FTestApp : public Application::FApplicationConsole {
    typedef Application::FApplicationConsole parent_type;
public:
    FTestApp();
    ~FTestApp();

    virtual void Start() override;
    virtual void Shutdown() override;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Test
} //!namespace PPE
