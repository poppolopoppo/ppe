#pragma once

#include "Core.Application/Application.h"

#include "Core.Application/ApplicationService.h"

#include "Core/IO/String.h"

namespace Core {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ApplicationBase
{
public:
    explicit ApplicationBase(const wchar_t *appname);
    virtual ~ApplicationBase();

    ApplicationBase(ApplicationBase&&) = delete;
    ApplicationBase& operator =(ApplicationBase&&) = delete;

    ApplicationBase(const ApplicationBase&) = delete;
    ApplicationBase& operator =(const ApplicationBase&) = delete;

    const WString& AppName() const { return _appname; }

    ApplicationServiceContainer& Services() { return _services; }
    const ApplicationServiceContainer& Services() const { return _services; }

    virtual void Start();
    virtual void Shutdown();

private:
    WString _appname;
    ApplicationServiceContainer _services;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace Core
