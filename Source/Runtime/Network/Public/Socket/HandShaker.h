#pragma once

#include "Network_fwd.h"

#include "Socket/Listener.h"
#include "Socket/ServicingPort.h"

#include "Thread/Task/TaskManager.h"

#include "IO/StringView.h"
#include "Memory/RefPtr.h"
#include "Misc/Function.h"

#include <atomic>

namespace PPE {
namespace Network {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(HandShaker);
class PPE_NETWORK_API FHandShaker : public FRefCountable {
public:
    FHandShaker(
        FStringLiteral name,
        size_t workerCount,
        FListener&& rlistener,
        FServicingTask&& rdispatch,
        FMilliseconds acceptTimeout = 1.0_s,
        FMilliseconds keepAliveTimeout = 10.0_s );

    bool IsRunning() const NOEXCEPT { return _running; }
    FTimepoint LastAlive() const NOEXCEPT { return FTimepoint{ _lastAlive }; }

    const FMilliseconds& AcceptTimeout() const { return _acceptTimeout; }
    void SetAcceptTimeout(FMilliseconds value) { _acceptTimeout = value; }

    const FMilliseconds& KeepAliveTimeout() const { return _keepAliveTimeout; }
    void SetKeepAliveTimeout(FMilliseconds value) { _keepAliveTimeout = value; }

    void* UserData() const NOEXCEPT { return _userData; }
    void SetUserData(void* data) { _userData = data; }

    void Start();
    void Shutdown();

    bool Dispatch(FServicingPort& port) const;

private:
    void StartupTask_(ITaskContext& ctx);
    void PollingTask_(ITaskContext& ctx);

    std::atomic_bool _running;
    mutable/* Dispatch() */std::atomic<FTimepoint::value_type> _lastAlive;

    FListener _listener;

    FMilliseconds _acceptTimeout;
    FMilliseconds _keepAliveTimeout;
    void* _userData;

    const FServicingTask _dispatch;

    FTaskManager _workers;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace PPE
