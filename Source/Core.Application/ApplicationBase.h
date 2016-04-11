#pragma once

#include "Core.Application/Application.h"

#include "Core/IO/String.h"
#include "Core/Misc/ServiceContainer.h"

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

    ServiceContainer& Services() { return _services; }
    const ServiceContainer& Services() const { return _services; }

    virtual void Start();
    virtual void Shutdown();

private:
    WString _appname;
    ServiceContainer _services;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace Core
