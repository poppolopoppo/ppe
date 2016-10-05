#pragma once

#include "Core.Application/Application.h"
#include "Core.Application/ApplicationConsole.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FApplicationTest : public Application::FApplicationConsole {
public:
    FApplicationTest();
    virtual ~FApplicationTest();

protected:
    virtual void Start() override;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
