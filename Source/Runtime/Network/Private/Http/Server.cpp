﻿// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Http/Server.h"

#include "Http/Exceptions.h"
#include "Http/Method.h"
#include "Http/Request.h"
#include "Http/Response.h"
#include "Http/Status.h"

#include "Network_fwd.h"
#include "Socket/Address.h"
#include "Socket/HandShaker.h"
#include "Socket/Listener.h"
#include "Socket/ServicingPort.h"
#include "Socket/SocketBuffered.h"

#include "Diagnostic/Logger.h"
#include "IO/StringBuilder.h"

namespace PPE {
namespace Network {
EXTERN_LOG_CATEGORY(PPE_NETWORK_API, Network)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FHttpServer::FHttpServer(
    FStringLiteral name,
    FAddress&& localhost,
    FMilliseconds timeout,
    FBytes maxContentLength )
:   _name(name)
,   _localhost(std::move(localhost))
,   _timeout(timeout)
,   _maxContentLength(static_cast<size_t>(maxContentLength.Value()))
{}
//----------------------------------------------------------------------------
FHttpServer::~FHttpServer() {
    Assert(not IsRunning());
}
//----------------------------------------------------------------------------
bool FHttpServer::IsRunning() const {
    return (!!_service);
}
//----------------------------------------------------------------------------
void FHttpServer::Start(size_t workerCount) {
    Assert(not IsRunning());

    PPE_LOG(Network, Info, "HTTP: starting <{0}> server on {1}", _name, Localhost());

    _service = NEW_REF(Socket, FHandShaker, _name, workerCount,
        FListener{ Localhost() },
        FServicingTask::Bind<&FHttpServer::Servicing_ReturnKeepAlive_>(this) );

    _service->Start();

    Assert_NoAssume(IsRunning());
}
//----------------------------------------------------------------------------
void FHttpServer::OnConnect(FServicingPort& port) const {
    Unused(port);

    PPE_LOG(Network, Verbose, "HTTP: <{0}> server has new connection {1} -> {2} ({3})",
        _name, port.Socket().Local(), port.Socket().Remote(), port.UID() );
}
//----------------------------------------------------------------------------
bool FHttpServer::OnRequest(FServicingPort& port, const FHttpRequest& request) const {
    Unused(port);
    Unused(request);

    PPE_LOG(Network, Error, "HTTP: <{0}> server unhandled request: {1} {2} ({3})",
        _name, request.Method(), request.Uri(), port.UID() );

    PPE_THROW_IT(FHttpException(EHttpStatus::NotFound, "unhandled request"));
}
//----------------------------------------------------------------------------
void FHttpServer::OnDisconnect(FServicingPort& port) const {
    Unused(port);

    PPE_LOG(Network, Verbose, "HTTP: <{0}> server closed connection {1} ({2})",
        _name, port.Socket().Remote(), port.UID());
}
//----------------------------------------------------------------------------
void FHttpServer::Shutdown() {
    Assert(IsRunning());

    _service->Shutdown();
    _service.reset();

    Assert(not IsRunning());
}
//----------------------------------------------------------------------------
bool FHttpServer::Servicing_ReturnKeepAlive_(FServicingPort& port) const {

    Assert(port.Socket().IsConnected());

    if (port.LastAlive().Value() == 0)
        OnConnect(port);

    FSocketBuffered& socket = port.Socket();

    PPE_TRY{
        FHttpRequest request;
        if (not FHttpRequest::Read(&request, socket, _maxContentLength))
            return false;

        const bool keepAlive = (EqualsI("keep-alive"_view, request.HTTP_Connection().MakeView()));

        PPE_LOG(Network, Info, "HTTP: <{0}> server request method={1}, uri={2} from {3}:{4} (keep-alive={5})",
            _name,
            request.Method(), request.Uri(),
            socket.Remote().Host(), socket.Remote().Port(),
            keepAlive );

        if (OnRequest(port, request) && keepAlive) {
            port.KeepAlive(); // avoid timeout
            return true;
        }
    }
    PPE_CATCH(FHttpException e)
    PPE_CATCH_BLOCK({
        PPE_LOG(Network, Error, "HTTP: <{0}> server error status={1}, reason={2} on {3}:{4}",
            _name,
            e.Status(), e.What(),
            socket.Local().Host(), socket.Local().Port());

        FHttpResponse response;
        response.Clear();
        response.SetStatus(e.Status());
        response.SetReason(FString(MakeCStringView(e.What())));

        if (not FHttpResponse::Write(&socket, response)) {
            PPE_LOG(Network, Error, "HTTP: <{0}> server failed to respond to {1}:{2}",
                _name, socket.Local().Host(), socket.Local().Port());
            // still try to disconnect gracefully
        }
    })

    OnDisconnect(port);
    return false;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace PPE
