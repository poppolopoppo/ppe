#pragma once

#include "Application/ApplicationConsole.h"

namespace PPE {
namespace Test {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FTestApp : public Application::FApplicationConsole {
    typedef Application::FApplicationConsole parent_type;
public:
    explicit FTestApp(FModularDomain& domain);
    ~FTestApp() override;

    virtual void Start() override;
    virtual void Run() override;
    virtual void Shutdown() override;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Test
} //!namespace PPE
