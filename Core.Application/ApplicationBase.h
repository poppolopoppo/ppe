#pragma once

#include "Application.h"

#include "Core.Engine/ServiceContainer.h"

#include "Core/String.h"

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

    Engine::IServiceProvider *Services() { return &_services; }
    const Engine::IServiceProvider *Services() const { return &_services; }

    virtual void Start();
    virtual void Shutdown();

private:
    WString _appname;
    Engine::ServiceContainer _services;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace Core
