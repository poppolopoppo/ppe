#pragma once

#include "Core.Application/Application.h"

#include "Core/IO/String.h"
#include "Core/Misc/ServiceContainer.h"

namespace Core {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FApplicationBase
{
public:
    explicit FApplicationBase(const wchar_t *appname);
    virtual ~FApplicationBase();

    FApplicationBase(FApplicationBase&&) = delete;
    FApplicationBase& operator =(FApplicationBase&&) = delete;

    FApplicationBase(const FApplicationBase&) = delete;
    FApplicationBase& operator =(const FApplicationBase&) = delete;

    const FWString& AppName() const { return _appname; }

    FServiceContainer& Services() { return _services; }
    const FServiceContainer& Services() const { return _services; }

    virtual void Start();
    virtual void Shutdown();

private:
    FWString _appname;
    FServiceContainer _services;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace Core
