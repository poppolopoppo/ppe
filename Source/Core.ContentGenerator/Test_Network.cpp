#include "stdafx.h"

#include "Core.Network/Http/Client.h"
#include "Core.Network/Http/Exceptions.h"
#include "Core.Network/Http/Method.h"
#include "Core.Network/Http/Server.h"
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
        << str << eol
        << "    Scheme: " << uri.Scheme() << eol
        << "    Username: " << uri.Username() << eol
        << "    Hostname: " << uri.Hostname() << eol
        << "    Port: " << uri.Port() << eol
        << "    Path: " << uri.Path() << eol
        << "    Query: " << uri.Query() << eol
        << "    Anchor: " << uri.Anchor() << eol
        << eol;

    FUri::FQueryMap args;
    if (not FUri::Unpack(args, uri))
        return false;

    std::cout << "Args[" << args.size() << "]:" << eol;
    for (const auto& it : args)
        std::cout << "    " << it.first << " = " << it.second << eol;

    FUri test;
    if (not FUri::Pack(test, uri.Scheme(), uri.Username(), uri.Port(), uri.Hostname(), uri.Path(), args, uri.Anchor()) )
        return false;

    std::cout << uri << eol;
    std::cout << test << eol;

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

        std::cout << "Listening on '" << listener.Listening() << "' ..." << eol;

        if (FSocketBuffered::Accept(socket, listener, FSeconds(5))) {
            succeed = true;
            std::cout << "Accepted from '" << socket.Local() << "' to '" << socket.Remote() << "' :)" << eol;

            CORE_TRY
            {
                FHttpRequest request;
                FHttpRequest::Read(&request, socket, maxContentLength);

                std::cout << "Method: " << request.Method() << eol;
                std::cout << "Uri: " << request.Uri() << eol;

                std::cout << "Headers:" << eol;
                for (const auto& it : request.Headers())
                    std::cout << " - '" << it.first << "' : '" << it.second << "'" << eol;

                FHttpRequest::FCookieMap cookies;
                if (FHttpRequest::UnpackCookie(&cookies, request)) {
                    std::cout << "Cookies:" << eol;
                    for (const auto& it : cookies)
                        std::cout << " - '" << it.first << "' : '" << it.second << "'" << eol;
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
                CORE_CATCH_BLOCK(std::cerr << e.Status() << eol << e.what() << eol;)
            }

            socket.Disconnect(true);
            break;
        }
    }

    if (not succeed)
        std::cout << "No incomming connection :'(" << eol;
}
//----------------------------------------------------------------------------
static void Test_HttpGet_() {
    FUri uri;
    if (not FUri::Parse(uri, "http://freegeoip.net/xml/poppolopoppo.ddns.net"))
        AssertNotReached();

    FHttpResponse response;
    if (EHttpStatus::OK != HttpGet(&response, uri))
        AssertNotReached();

    std::cout << "Status: " << response.Status() << eol;
    std::cout << "Reason: " << response.Reason() << eol;

    std::cout << "Headers:" << eol;
    for (const auto& it : response.Headers())
        std::cout << " - '" << it.first << "' : '" << it.second << "'" << eol;

    std::cout << "Body:" << eol;
    std::cout << response.Body().MakeView() << eol;

	if (response.Status() == Network::EHttpStatus::OK) {
		XML::FDocument xml;
		if (not XML::FDocument::Load(&xml, L"network.tmp", &response.Body()))
			AssertNotReached();

		std::cout << "XML:" << eol;
		std::cout << xml << eol;
	}
	else {
		LOG(Error, L"[Http] Request to '{0}' failed : {1}", uri, response.Status());
	}
}
//----------------------------------------------------------------------------
static void Test_HttpPost_() {
    FUri uri;
    if (not FUri::Parse(uri, "http://www.w3schools.com/php/demo_form_validation_complete.php"))
        AssertNotReached();

    FHttpClient::FPostMap post = {
        { "name",       "Tom Cum von Bernardo $"},
        { "email",      "tom.cum@gmail.com"     },
        { "website",    "www.tom-cum.com"       },
        { "gender",     "don't be so tacky"     },
    };

    FHttpResponse response;
    if (EHttpStatus::OK != HttpPost(&response, uri, post))
        AssertNotReached();

    std::cout << "Status: " << response.Status() << eol;
    std::cout << "Reason: " << response.Reason() << eol;

    std::cout << "Headers:" << eol;
    for (const auto& it : response.Headers())
        std::cout << " - '" << it.first << "' : '" << it.second << "'" << eol;

    std::cout << "Body:" << eol;
    std::cout << response.Body().MakeView() << eol;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
class FTestHttpServer : public FHttpServer {
public:
    FTestHttpServer() : FHttpServer("localhost", FAddress::Localhost(8126)) {}

private:
    virtual void OnAccept(FSocketBuffered& socket) const override {
        LOG(Info, L"[{0}] Accept connection on test server : {1}", socket.Local(), socket.Remote());
    }

    virtual void OnRequest(FSocketBuffered& socket, const FHttpRequest& request) const CORE_THROW() override {
        LOG(Info, L"[{0}] {1} requested : {2}", socket.Local(), socket.Remote(), request.Uri());

        FHttpResponse response;
        response.SetStatus(EHttpStatus::OK);
        response.SetReason("OK");

        FStreamWriterOStream oss(&response.Body());

        oss << "<html>" << eol
            << "    <body>" << eol;

        oss << "Method: " << request.Method() << "<br/>" << eol;
        oss << "Uri   : " << request.Uri() << "<br/>" << eol;

        oss << "Headers:" << "<br/>" << eol;
        for (const auto& it : request.Headers())
            oss << " - '" << it.first << "' : '" << it.second << "'" << "<br/>" << eol;

        oss << "<br/>" << eol;

        oss << "Body:" << "<br/>" << eol;
        oss << request.Body().MakeView() << "<br/>" << eol;

        oss << "    </body>" << eol
            << "</html>" << eol;

        FHttpResponse::Write(&socket, response);
    }

    virtual void OnDisconnect(FSocketBuffered& socket) const override{
        LOG(Info, L"[{0}] Disconnected from test server : {1}", socket.Local(), socket.Remote());
    }
};
//----------------------------------------------------------------------------
static void Test_HttpServer_() {
    FTestHttpServer srv;
    srv.Start();
    std::this_thread::sleep_for(std::chrono::seconds(30));
    srv.Stop();
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
    Test_HttpPost_();
    Test_HttpServer_();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentGenerator
} //!namespace Core
