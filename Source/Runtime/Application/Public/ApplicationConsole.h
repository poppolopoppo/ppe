#pragma once

#include "Application.h"

#include "ApplicationBase.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_APPLICATION_API FApplicationConsole : public FApplicationBase {
public:
    FApplicationConsole(const FModularDomain& domain, FWString&& name);
    virtual ~FApplicationConsole();

    virtual void Start() override;
    virtual void Shutdown() override;

    void Daemonize();
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
