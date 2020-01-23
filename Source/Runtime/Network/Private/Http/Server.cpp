#include "stdafx.h"

#include "Http/Server.h"

#include "Http/Exceptions.h"
#include "Http/Method.h"
#include "Http/Request.h"
#include "Http/Response.h"
#include "Http/Status.h"

#include "Network_fwd.h"
#include "Socket/Address.h"
#include "Socket/Listener.h"
#include "Socket/SocketBuffered.h"

#include "Diagnostic/Logger.h"
#include "IO/StringBuilder.h"
#include "Thread/ThreadContext.h"
#include "Thread/ThreadPool.h"

namespace PPE {
namespace Network {
EXTERN_LOG_CATEGORY(PPE_NETWORK_API, Network)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FHttpServerImpl;
//----------------------------------------------------------------------------
namespace {
static void HttpServicingHandleConnection_(const FHttpServerImpl* server, FSocketBuffered&& socket);
} //!namespace
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
    void OnRequest(FSocketBuffered& socket, const FHttpRequest& request) const PPE_THROW() { _owner->OnRequest(socket, request); }
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
void HttpServicingTask_(ITaskContext& , const FHttpServerImpl* server, FSocketBuffered& socket) {
    Assert(server);
    Assert(socket.IsConnected());

    server->OnAccept(socket);

    PPE_TRY {
        FHttpRequest request;
        FHttpRequest::Read(&request, socket, server->MaxContentLength());

        LOG(Network, Info, L"HTTP server request : method={0}, uri={1} from {2}:{3}",
            request.Method(), request.Uri(),
            socket.Remote().Host(), socket.Remote().Port() );

        server->OnRequest(socket, request);
    }
    PPE_CATCH(FHttpException e)
    PPE_CATCH_BLOCK({
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
    const FThreadContextStartup threadStartup("HttpServer", PPE_THREADTAG_OTHER);
    threadStartup.Context().SetPriority(EThreadPriority::Lowest);

    const FMilliseconds acceptTimeout = 1.0_s;

    FHttpServerImpl server(owner);
    for (; not server.Serve_ReturnIfQuit(acceptTimeout); );

    FIOThreadPool::Get().WaitForAll(); // wait for eventual pending tasks before leaving
}
//----------------------------------------------------------------------------
static void HttpServicingHandleConnection_(const FHttpServerImpl* server, FSocketBuffered&& socket) {
    TUniquePtr<FSocketBuffered> psocket{ new FSocketBuffered(std::move(socket)) };
    FIOThreadPool::Get().Run([server, psocket{ std::move(psocket) }](ITaskContext & ctx) {
        HttpServicingTask_(ctx, server, *psocket);
        psocket->Disconnect(true);
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
:   _name(name)
,   _localhost(std::move(localhost))
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
} //!namespace PPE
