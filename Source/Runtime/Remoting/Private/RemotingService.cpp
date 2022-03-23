#include "stdafx.h"

#include "RemotingService.h"

#include "RemotingEndpoint.h"
#include "RemotingServer.h"
#include "Remoting/BaseEndpoint.h"
#include "Remoting/ProcessEndpoint.h"
#include "Remoting/RTTIEndpoint.h"
#include "Remoting/SwaggerEndpoint.h"

#include "Application.h"
#include "ApplicationModule.h"
#include "HAL/PlatformNotification.h"
#include "IO/String.h"
#include "Modular/ModularDomain.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Remoting {
namespace {
//----------------------------------------------------------------------------
class FDefaultRemotingService_ final : public IRemotingService {
public:
    explicit FDefaultRemotingService_(const FModularDomain& domain)
    :   _domain(domain) {
        _srv.reset<FRemotingServer>();

        // add default endpoints here
        auto swagger = MakeUnique<FSwaggerEndpoint>();

        _swapperCmdIndex = Application::FPlatformNotification::AddSystrayCommand(
            L"Remoting",
            L"Swagger/API",
            [pSwagger{ swagger.get() }, pSrv{ _srv.get() }]() {
                const FWString swaggerUrl = ToWString(pSwagger->SwaggerApi(*pSrv));
                FPlatformProcess::OpenURL(swaggerUrl.c_str());
            });

        _srv->Add(std::move(swagger));
        _srv->Add(NEW_REF(Remoting, FProcessEndpoint));
        _srv->Add(NEW_REF(Remoting, FRTTIEndpoint));

        _srv->Start();

        _tickHandle = FApplicationModule::Get(_domain).OnApplicationTick().Add(
            FApplicationModule::FApplicationTick::Bind<&FDefaultRemotingService_::TickRemoting>(this) );
    }

    virtual ~FDefaultRemotingService_() override {
        Assert(_srv);

        FApplicationModule::Get(_domain).OnApplicationTick().Remove(_tickHandle);

        Application::FPlatformNotification::RemoveSystrayCommand(_swapperCmdIndex);
        _swapperCmdIndex = INDEX_NONE;

        _srv->Shutdown();

        _srv.reset();
    }

    virtual const FRemotingServer& Server() const NOEXCEPT override {
        return (*_srv);
    }

    virtual void RegisterEndpoint(PBaseEndpoint&& endpoint) override {
        _srv->Add(std::move(endpoint));
    }

    virtual void UnregisterEndpoint(const PBaseEndpoint& endpoint) override {
        _srv->Remove(endpoint);
    }

    void TickRemoting(Application::FApplicationBase& , FTimespan dt) NOEXCEPT {
        _srv->Tick(dt);
    }

private:
    const FModularDomain& _domain;
    TUniquePtr<FRemotingServer> _srv;
    FEventHandle _tickHandle;
    size_t _swapperCmdIndex{INDEX_NONE};

};
//----------------------------------------------------------------------------
} //!namespace
} //!namespace Remoting
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void IRemotingService::MakeDefault(
    URemotingService* remoting,
    const FModularDomain& domain ) {
    Assert(remoting);
    remoting->reset<Remoting::FDefaultRemotingService_>(domain);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
