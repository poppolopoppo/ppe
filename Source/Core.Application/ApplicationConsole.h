#pragma once

#include "Core.Application/Application.h"

#include "Core.Application/ApplicationBase.h"

namespace Core {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ApplicationConsole : public ApplicationBase
{
public:
    ApplicationConsole(const wchar_t *appname);
    virtual ~ApplicationConsole();

    virtual void Start() override;
    virtual void Shutdown() override;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace Core
