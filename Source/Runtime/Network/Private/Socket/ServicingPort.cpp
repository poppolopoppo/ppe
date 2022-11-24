// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Socket/ServicingPort.h"

#include "Socket/HandShaker.h"

#include "Diagnostic/Logger.h"
#include "Time/Timepoint.h"
#include "Thread/ThreadContext.h"

namespace PPE {
namespace Network {
EXTERN_LOG_CATEGORY(PPE_NETWORK_API, Network)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FServicingPort::FServicingPort(const FHandShaker& owner, FSocketBuffered&& rsocket) NOEXCEPT
:   _owner(&owner)
,   _uid(FGuid::Generate())
,   _socket(std::move(rsocket)) {
    Assert_NoAssume(_socket.IsConnected());
}
//----------------------------------------------------------------------------
FServicingPort::~FServicingPort() {
    Assert_NoAssume(not _socket.IsConnected());
}
//----------------------------------------------------------------------------
void FServicingPort::KeepAlive() {
    _lastAlive = FTimepoint::Now();
}
//----------------------------------------------------------------------------
bool FServicingPort::Timeout() const {
    return (FTimepoint::ElapsedSince(_lastAlive) > _owner->KeepAliveTimeout());
}
//----------------------------------------------------------------------------
void FServicingPort::Servicing(ITaskContext& ctx) {
    // defer socket control to our dispatch callback, and check if we need rescheduling
    if (_owner->Dispatch(*this)) {
        if (_socket.IsConnected()) {
            // check port lifetime to see if it timeout
            if (Timeout()) {
                LOG(Network, Info, L"HandShaker: servicing port {0} -> {1} timeouted ({2} > {3})",
                    _socket.Local(), _socket.Remote(),
                    FTimepoint::ElapsedSince(_lastAlive),
                    _owner->KeepAliveTimeout() );
            }
            // else re-schedule port for execution
            else {
                Queue(ctx, PServicingPort{ this }); // duplicate refptr to keep the port alive with the task
                return;
            }
        }
    }

    // disconnect the socket is not re-scheduled
    if (_socket.IsConnected()) {
        _socket.Disconnect(_owner->IsRunning()); // only graceful when not exiting
    }
}
//----------------------------------------------------------------------------
void FServicingPort::Queue(ITaskContext& ctx, PServicingPort&& rport) {
    Assert(rport);
    // immediately dispatch without waiting and default priority
    ctx.FireAndForget(FTaskFunc::Bind<&FServicingPort::Servicing>(std::move(rport)));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace PPE
