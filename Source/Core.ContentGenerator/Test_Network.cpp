#include "stdafx.h"

#include "Core.Network/Http/Client.h"
#include "Core.Network/Http/Exceptions.h"
#include "Core.Network/Http/Method.h"
#include "Core.Network/Http/Status.h"
#include "Core.Network/Http/Request.h"
#include "Core.Network/Http/Response.h"
#include "Core.Network/Socket/Address.h"
#include "Core.Network/Socket/Listener.h"
#include "Core.Network/Socket/SocketBuffered.h"
#include "Core.Network/Uri.h"

#include "Core.Serialize/XML/Document.h"
#include "Core.Serialize/XML/Element.h"

namespace Core {
namespace ContentGenerator {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
using namespace Core::Network;
//----------------------------------------------------------------------------
static bool ParseUri_(const FStringView& str) {
    FUri uri;
    if (not FUri::Parse(uri, str))
        return false;

    std::cout
        << str << std::endl
        << "    Scheme: " << uri.Scheme() << std::endl
        << "    Username: " << uri.Username() << std::endl
        << "    Hostname: " << uri.Hostname() << std::endl
        << "    Port: " << uri.Port() << std::endl
        << "    Path: " << uri.Path() << std::endl
        << "    Query: " << uri.Query() << std::endl
        << "    Anchor: " << uri.Anchor() << std::endl
        << std::endl;

    FUri::FQueryMap args;
    if (not FUri::Unpack(args, uri))
        return false;

    std::cout << "Args[" << args.size() << "]:" << std::endl;
    for (const auto& it : args)
        std::cout << "    " << it.first << " = " << it.second << std::endl;

    FUri test;
    if (not FUri::Pack(test, uri.Scheme(), uri.Username(), uri.Port(), uri.Hostname(), uri.Path(), args, uri.Anchor()) )
        return false;

    std::cout << uri << std::endl;
    std::cout << test << std::endl;

    if (test.Str() != uri.Str())
        return false;

    return true;
}
//----------------------------------------------------------------------------
static void Test_ParseUri_() {
    if (not ParseUri_("http://jquery.com/?s=toto+slip+123"))
        AssertNotReached();
    if (not ParseUri_("https://bek@www.google.fr/search?num=20&safe=off&site=&source=hp&q=mangeur+de+p%C3%AEngouin&oq=mangeur+de+p%C3%AEngouin&gs_l=hp.3..0i22i30k1.1022.5665.0.5988.22.19.1.2.3.0.112.1214.16j3.19.0....0...1c.1.64.hp..0.19.1065.0..0j35i39k1j0i67k1j0i131k1j0i13i30k1.lOSTdUm6eCQ#anchor"))
        AssertNotReached();
}
//----------------------------------------------------------------------------
static void Test_SocketAccept_() {
    FListener listener = FListener::Localhost(8123);

    const FListener::FConnectionScope connection(listener);

    const size_t maxContentLength = size_t(FMegabytes(10).Value());

    bool succeed = false;
    forrange(i, 0, 20) {
        FSocketBuffered socket;
        socket.SetTimeout(FSeconds(0.3));

        std::cout << "Listening on '" << listener.Listening() << "' ..." << std::endl;

        if (FSocketBuffered::Accept(socket, listener, FSeconds(5))) {
            succeed = true;
            std::cout << "Accepted from '" << socket.Local() << "' to '" << socket.Remote() << "' :)" << std::endl;

            CORE_TRY
            {
                FHttpRequest request;
                FHttpRequest::Read(&request, socket, maxContentLength);

                std::cout << "Method: " << request.Method() << std::endl;
                std::cout << "Uri: " << request.Uri() << std::endl;

                std::cout << "Headers:" << std::endl;
                for (const auto& it : request.Headers())
                    std::cout << " - '" << it.first << "' : '" << it.second << "'" << std::endl;

                FHttpRequest::FCookieMap cookies;
                if (FHttpRequest::UnpackCookie(&cookies, request)) {
                    std::cout << "Cookies:" << std::endl;
                    for (const auto& it : cookies)
                        std::cout << " - '" << it.first << "' : '" << it.second << "'" << std::endl;
                }

                FHttpResponse response;

                if (EndsWithI(request.Uri().Path(), "favicon.ico")) {
                    response.SetStatus(EHttpStatus::NotFound);
                }
                else {
                    response.SetStatus(EHttpStatus::OK);
                    response.Body().WriteView(MakeStringView("<marquee width='100%'>Coming soon <b>bitches</b> !</marquee>"));
                    response.UpdateContentHeaders("text/html; charset=UTF-8");
                }

                FHttpResponse::Write(&socket, response);
            }
            CORE_CATCH(FHttpException e)
            {
                CORE_CATCH_BLOCK(std::cerr << e.Status() << std::endl << e.what() << std::endl;)
            }

            socket.Disconnect(true);
            break;
        }
    }

    if (not succeed)
        std::cout << "No incomming connection :'(" << std::endl;
}
//----------------------------------------------------------------------------
static void Test_HttpGet_() {
    FUri uri;
    if (not FUri::Parse(uri, "http://freegeoip.net/xml/poppolopoppo.ddns.net"))
        AssertNotReached();

    FHttpResponse response;
    if (not HttpGet(&response, uri))
        AssertNotReached();

    std::cout << "Status: " << response.Status() << std::endl;
    std::cout << "Reason: " << response.Reason() << std::endl;

    std::cout << "Headers:" << std::endl;
    for (const auto& it : response.Headers())
        std::cout << " - '" << it.first << "' : '" << it.second << "'" << std::endl;

    std::cout << "Body:" << std::endl;
    std::cout << response.Body().MakeView() << std::endl;

    XML::FDocument xml;
    if (not XML::FDocument::Load(&xml, L"network.tmp", &response.Body()))
        AssertNotReached();

    std::cout << "XML:" << std::endl;
    std::cout << xml << std::endl;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void Test_Network() {
    Test_ParseUri_();
    Test_SocketAccept_();
    Test_HttpGet_();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentGenerator
} //!namespace Core
