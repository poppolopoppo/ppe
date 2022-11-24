// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Socket/HandShaker.h"

#include "Diagnostic/Logger.h"
#include "Thread/ThreadContext.h"

namespace PPE {
namespace Network {
EXTERN_LOG_CATEGORY(PPE_NETWORK_API, Network)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FHandShaker::FHandShaker(
    const FStringView& name,
    size_t workerCount,
    FListener&& rlistener,
    FServicingTask&& rdispatch,
    FMilliseconds acceptTimeout,
    FMilliseconds keepAliveTimeout )
:   _running(false)
,   _lastAlive(0)
,   _listener(std::move(rlistener))
,   _acceptTimeout(acceptTimeout)
,   _keepAliveTimeout(keepAliveTimeout)
,   _userData(nullptr)
,   _dispatch(std::move(rdispatch))
,   _workers(name, PPE_THREADTAG_SERVICING, workerCount, EThreadPriority::Lowest) {
    Assert_NoAssume(_dispatch.Valid());
}
//----------------------------------------------------------------------------
void FHandShaker::Start() {
    Assert(false == _running);
    Assert_NoAssume(not _listener.IsConnected());

    // kick-in servicing workers
    _running = true;
    _lastAlive = FTimepoint::Now().Value();
    _workers.Start();

    // initialize first polling through the task manager
    _workers.Run(FTaskFunc::Bind<&FHandShaker::StartupTask_>(this), ETaskPriority::High);
}
//----------------------------------------------------------------------------
void FHandShaker::Shutdown() {
    Assert(_running);

    // signal exit and join workers
    _running = false;
    _workers.Shutdown();
    _lastAlive = 0;
}
//----------------------------------------------------------------------------
bool FHandShaker::Dispatch(FServicingPort& port) const {
    Assert_NoAssume(port.Socket().IsConnected());

    if (_dispatch(port) && _running) {
        _lastAlive = FTimepoint::Now().Value(); // don't need an exact value, just a recent one
        return true;
    }
    else {
        return false;
    }
}
//----------------------------------------------------------------------------
void FHandShaker::StartupTask_(ITaskContext& ctx) {
    Assert_NoAssume(not _listener.IsConnected());

    if (_listener.Connect()) {
        LOG(Network, Verbose, L"HandShaker: <{0}> started polling from {1}", _workers.Name(), _listener.Listening());

        PollingTask_(ctx);
    }
    else {
        LOG(Network, Error, L"HandShaker: <{0}> failed to connect listener {1}, stopping hand shaker", _workers.Name(), _listener.Listening());
    }
}
//----------------------------------------------------------------------------
void FHandShaker::PollingTask_(ITaskContext& ctx) {
    Assert_NoAssume(_workers.IsRunning());

    if (_running && _listener.IsConnected()) {
        // check for incoming connections
        FSocketBuffered socket;
        if (_listener.Accept(socket, _acceptTimeout)) {
            Assert(socket.IsConnected());

            LOG(Network, Verbose, L"HandShaker: <{0}> accepted a new connection {1}", _workers.Name(), socket.Remote());

            socket.SetTimeout(_acceptTimeout);

            FServicingPort::Queue(ctx, NEW_REF(Socket, FServicingPort, *this, std::move(socket)));
        }

        // queue next polling through the task context
        ctx.FireAndForget(FTaskFunc::Bind<&FHandShaker::PollingTask_>(this), ETaskPriority::Low);
    }
    else {
        if (_listener.IsConnected())
            _listener.Disconnect();

        LOG(Network, Verbose, L"HandShaker: <{0}> stopped polling from {1}", _workers.Name(), _listener.Listening());
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace PPE
