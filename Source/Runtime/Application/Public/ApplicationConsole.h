#pragma once

#include "Application.h"

#include "ApplicationBase.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FApplicationConsole : public FApplicationBase {
public:
    FApplicationConsole(const wchar_t *appname);
    virtual ~FApplicationConsole();

    virtual void Start() override;
    virtual void Shutdown() override;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
