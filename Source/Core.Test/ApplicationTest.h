#pragma once

#include "Core.Application/Application.h"
#include "Core.Application/ApplicationConsole.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ApplicationTest : public Application::ApplicationConsole {
public:
    ApplicationTest();
    virtual ~ApplicationTest();

protected:
    virtual void Start() override;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
