#include "stdafx.h"

#include "Server.h"

#include "Exceptions.h"
#include "Method.h"
#include "Request.h"
#include "Response.h"
#include "Status.h"

#include "../Network_fwd.h"
#include "../Socket/Address.h"
#include "../Socket/Listener.h"
#include "../Socket/SocketBuffered.h"

#include "Core/Diagnostic/Logger.h"
#include "Core/IO/StringBuilder.h"
#include "Core/Thread/ThreadContext.h"
#include "Core/Thread/ThreadPool.h"

namespace Core {
namespace Network {
EXTERN_LOG_CATEGORY(CORE_NETWORK_API, Network)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace { static void HttpServicingHandleConnection_(const FHttpServerImpl* server, FSocketBuffered&& socket); }
//----------------------------------------------------------------------------
class FHttpServerImpl {
public:
    FHttpServerImpl(const FHttpServer* owner);
    ~FHttpServerImpl();

    FHttpServerImpl(const FHttpServerImpl& ) = delete;
    FHttpServerImpl& operator =(const FHttpServerImpl& ) = delete;

    size_t MaxContentLength() const { return _maxContentLength; }

    bool Serve_ReturnIfQuit(const FMilliseconds& timeout);

    void OnAccept(FSocketBuffered& socket) const { _owner->OnAccept(socket); }
    void OnRequest(FSocketBuffered& socket, const FHttpRequest& request) const CORE_THROW() { _owner->OnRequest(socket, request); }
    void OnDisconnect(FSocketBuffered& socket) const { _owner->OnDisconnect(socket); }

private:
    const FHttpServer* _owner;
    FListener _listener;
    const size_t _maxContentLength;
};
//----------------------------------------------------------------------------
FHttpServerImpl::FHttpServerImpl(const FHttpServer* owner)
:   _owner(owner)
,   _listener(owner->Localhost())
,   _maxContentLength(owner->MaxContentLength()) {
    _listener.Connect();

    LOG(Network, Info, L"starting HTTP server on {0}",  _listener.Listening());
}
//----------------------------------------------------------------------------
FHttpServerImpl::~FHttpServerImpl() {
    LOG(Network, Info, L"stopping HTTP server on {0}",  _listener.Listening());

    if (_listener.IsConnected())
        _listener.Disconnect();
}
//----------------------------------------------------------------------------
bool FHttpServerImpl::Serve_ReturnIfQuit(const FMilliseconds& timeout) {
    if (_owner->_quit)
        return true;

    if (_listener.IsConnected() == false) {
        LOG(Network, Error, L"got disconnected from HTTP listener on {0}", _listener.Listening());
        return true;
    }

    FSocketBuffered socket;
    if (_listener.Accept(socket, timeout)) {
        Assert(socket.IsConnected());

        socket.SetTimeout(_owner->Timeout());

        HttpServicingHandleConnection_(this, std::move(socket));
    }

    return (_owner->_quit);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
void HttpServicingTask_(ITaskContext& ctx, const FHttpServerImpl* server, FSocketBuffered& socket) {
    Assert(server);
    Assert(socket.IsConnected());

    server->OnAccept(socket);

    CORE_TRY {
        FHttpRequest request;
        FHttpRequest::Read(&request, socket, server->MaxContentLength());

        LOG(Network, Info, L"HTTP server request : method={0}, uri={1} from {2}:{3}",
            request.Method(), request.Uri(),
            socket.Remote().Host(), socket.Remote().Port() );

        server->OnRequest(socket, request);
    }
    CORE_CATCH(FHttpException e)
    CORE_CATCH_BLOCK({
        LOG(Network, Error, L"HTTP server error : status={0}, reason={1} on {2}:{3}",
            e.Status(), e.What(),
            socket.Local().Host(), socket.Local().Port() );

        FHttpResponse response;
        response.Clear();
        response.SetStatus(e.Status());
        response.SetReason(FString(MakeCStringView(e.What())));

        FHttpResponse::Write(&socket, response);
    })

    server->OnDisconnect(socket);
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static void HttpServicingThreadLaunchPad_(FHttpServer* owner) {
    const FThreadContextStartup threadStartup("HttpServer", CORE_THREADTAG_OTHER);
    threadStartup.Context().SetPriority(EThreadPriority::Lowest);

    const FMilliseconds acceptTimeout = FSeconds(1);

    FHttpServerImpl server(owner);
    for (; not server.Serve_ReturnIfQuit(acceptTimeout); );

    FIOThreadPool::Get().WaitForAll(); // wait for eventual pending tasks before leaving
}
//----------------------------------------------------------------------------
static void HttpServicingHandleConnection_(const FHttpServerImpl* server, FSocketBuffered&& socket) {
    FSocketBuffered* psocket = new FSocketBuffered(std::move(socket));
    FIOThreadPool::Get().Run([server, psocket](ITaskContext& ctx) {
        HttpServicingTask_(ctx, server, *psocket);
        psocket->Disconnect(true);
        checked_delete(psocket);
    });
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FHttpServer::FHttpServer(
    const FStringView& name,
    FAddress&& localhost,
    const FMilliseconds& timeout/* = FSeconds(3) */,
    size_t maxContentLength/* = DefaultMaxContentLength */)
:   _localhost(std::move(localhost))
,   _timeout(timeout)
,   _maxContentLength(maxContentLength)
,   _userData(nullptr)
,   _quit(true) {}
//----------------------------------------------------------------------------
FHttpServer::~FHttpServer() {
    Assert(not IsRunning());
}
//----------------------------------------------------------------------------
bool FHttpServer::IsRunning() const {
    return (not _quit && _servicing.joinable());
}
//----------------------------------------------------------------------------
void FHttpServer::Start() {
    Assert(not IsRunning());

    _quit = false;
    _servicing = std::thread(&HttpServicingThreadLaunchPad_, this);

    Assert(IsRunning());
}
//----------------------------------------------------------------------------
void FHttpServer::Stop() {
    Assert(IsRunning());

    _quit = true;
    _servicing.join();

    Assert(not IsRunning());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace Core
