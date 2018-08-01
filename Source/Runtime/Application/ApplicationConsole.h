#pragma once

#include "Core.Application/Application.h"

#include "Core.Application/ApplicationBase.h"

namespace Core {
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
} //!namespace Core
