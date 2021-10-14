#include "stdafx.h"

#include "RemotingService.h"

#include "RemotingEndpoint.h"
#include "RemotingServer.h"
#include "Remoting/BaseEndpoint.h"
#include "Remoting/ProcessEndpoint.h"
#include "Remoting/RTTIEndpoint.h"
#include "Remoting/SwaggerEndpoint.h"

#include "ApplicationModule.h"
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
        _srv->Add(MakeUnique<FSwaggerEndpoint>());
        _srv->Add(NEW_REF(Remoting, FProcessEndpoint));
        _srv->Add(NEW_REF(Remoting, FRTTIEndpoint));

        _srv->Start();

        _tickHandle = FApplicationModule::Get(_domain).OnApplicationTick().Add(
            FApplicationModule::FApplicationTick::Bind<&FDefaultRemotingService_::TickRemoting>(this) );
    }

    virtual ~FDefaultRemotingService_() override {
        Assert(_srv);

        FApplicationModule::Get(_domain).OnApplicationTick().Remove(_tickHandle);

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
