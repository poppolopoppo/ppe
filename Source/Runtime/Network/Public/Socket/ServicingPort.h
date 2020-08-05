#pragma once

#include "Network_fwd.h"

#include "Socket/SocketBuffered.h"

#include "Memory/RefPtr.h"
#include "Misc/Function.h"
#include "Misc/Guid.h"
#include "Thread/Task_fwd.h"
#include "Time/Timepoint.h"

namespace PPE {
namespace Network {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(ServicingPort);
class FServicingPort : public FRefCountable {
public:
    FServicingPort(const FHandShaker& owner, FSocketBuffered&& rsocket) NOEXCEPT;
    ~FServicingPort();

    const FHandShaker& Owner() const { return (*_owner); }
    const FGuid& UID() const { return _uid; }
    const FTimepoint& LastAlive() const { return _lastAlive; }

    FSocketBuffered& Socket() { return _socket; }
    const FSocketBuffered& Socket() const { return _socket; }

    void KeepAlive(); // should be called to avoid timeout
    bool Timeout() const; // see HandShaker keep alive timeout

    void Servicing(ITaskContext& ctx);

    static void Queue(ITaskContext& ctx, PServicingPort&& rport);

private:
    const SCHandShaker _owner;
    const FGuid _uid;

    FTimepoint _lastAlive;
    FSocketBuffered _socket;
};
//----------------------------------------------------------------------------
using FServicingTask = TFunction<bool/* keep-alive */(FServicingPort& port)>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace PPE
