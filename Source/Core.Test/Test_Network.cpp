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

/*
#include "Core.Serialize/XML/Document.h"
#include "Core.Serialize/XML/Element.h"
*/

#include "Core/Diagnostic/Logger.h"
#include "Core/IO/FS/Filename.h"
#include "Core/IO/FileStream.h"
#include "Core/IO/FormatHelpers.h"
#include "Core/IO/String.h"
#include "Core/IO/StringView.h"
#include "Core/IO/TextWriter.h"

namespace Core {
namespace Test {
LOG_CATEGORY(, Test_Network)
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

    LOG(Test_Network, Info,
        L"{0}\n"
        L"   Scheme: {0}\n"
        L"   Username: {1}\n"
        L"   Hostname: {2}\n"
        L"   Post: {3}\n"
        L"   Path: {4}\n"
        L"   Query: {5}\n"
        L"   Anchor: {6}",
        uri.Scheme(),
        uri.Username(),
        uri.Hostname(),
        uri.Port(),
        uri.Path(),
        uri.Query(),
        uri.Anchor());

    FUri::FQueryMap args;
    if (not FUri::Unpack(args, uri))
        return false;

    LOG(Test_Network, Info, L"Args[{0}]:", args.size());
    for (const auto& it : args)
        LOG(Test_Network, Info, L"    - {0} = '{1}'", it.first, it.second);

    FUri test;
    if (not FUri::Pack(test, uri.Scheme(), uri.Username(), uri.Port(), uri.Hostname(), uri.Path(), args, uri.Anchor()))
        return false;

    LOG(Test_Network, Info, L"{0}\n -> {1}", uri, test);

    if (test.Str() != uri.Str())
        return false;

    return true;
}
//----------------------------------------------------------------------------
static void Test_ParseUri_() {
    if (not ParseUri_("http://jquery.com/?s=toto+slip+123"))
        AssertNotReached();
    if (not ParseUri_("https://bek@www.google.fr/search?num=20&safe=off&site=&source=hp&q=mangeur+de+pingouin&oq=mangeur+de+pingouin&gs_l=hp.3..0i22i30k1.1022.5665.0.5988.22.19.1.2.3.0.112.1214.16j3.19.0....0...1c.1.64.hp..0.19.1065.0..0j35i39k1j0i67k1j0i131k1j0i13i30k1.lOSTdUm6eCQ#anchor"))
        AssertNotReached();
}
//----------------------------------------------------------------------------
static void Test_SocketAccept_() {
    FListener listener = FListener::Localhost(8123);

    const FListener::FConnectionScope connection(listener);
    if (not connection.Succeed) {
        LOG(Test_Network, Error, L"failed to open listener on '{0}'", listener.Listening());
        return;
    }

    const size_t maxContentLength = size_t(FMegabytes(10).Value());

    bool succeed = false;
    forrange(i, 0, 20) {
        FSocketBuffered socket;
        socket.SetTimeout(FSeconds(0.3));

        LOG(Test_Network, Info, L"Listening on '{0}'...", listener.Listening());

        if (FSocketBuffered::Accept(socket, listener, FSeconds(5))) {
            succeed = true;
            LOG(Test_Network, Info, L"Accepted from '{0}' to '{1}' :)", socket.Local(), socket.Remote());

            CORE_TRY
            {
                FHttpRequest request;
                FHttpRequest::Read(&request, socket, maxContentLength);

                LOG(Test_Network, Info, L"Method :   {0}", request.Method());
                LOG(Test_Network, Info, L"Uri    :   {0}", request.Uri());
                
                LOG(Test_Network, Info, L"Header :");
                for (const auto& it : request.Headers())
                    LOG(Test_Network, Info, L" - '{0}' : '{1}'", it.first, it.second);

                FHttpRequest::FCookieMap cookies;
                if (FHttpRequest::UnpackCookie(&cookies, request)) {
                    LOG(Test_Network, Info, L"Cookies :");
                    for (const auto& it : cookies)
                        LOG(Test_Network, Info, L" - '{0}' : '{1}'", it.first, it.second);
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
            CORE_CATCH_BLOCK({
                UNUSED(e);
                LOG(Test_Network, Error, L"HTTP error {0} : {1}", e.Status(), MakeCStringView(e.What()));
            });

            socket.Disconnect(true);
            break;
        }
    }

    if (not succeed)
        LOG(Test_Network, Warning, L"no incomming HTTP connexion :'(");
}
//----------------------------------------------------------------------------
static void Test_HttpGet_() {
    FUri uri;
    if (not FUri::Parse(uri, MakeStringView("http://freegeoip.net/xml/poppolopoppo.ddns.net")))
        AssertNotReached();

    forrange(i, 0, 3) {
        FHttpResponse response;
        if (EHttpStatus::OK != HttpGet(&response, uri))
            AssertNotReached();

        LOG(Test_Network, Info, L"Status: {0}", response.Status());
        LOG(Test_Network, Info, L"Reason: {0}", response.Reason());

        LOG(Test_Network, Info, L"Headers   :");
        for (const auto& it : response.Headers())
            LOG(Test_Network, Info, L" - '{0}' : '{1}'", it.first, it.second);

        LOG(Test_Network, Info, L"Body : {0} =\n{1}",
            Fmt::FSizeInBytes{ checked_cast<size_t>(response.Body().SizeInBytes()) },
            response.MakeView());

        if (response.Status() == Network::EHttpStatus::OK) {
            /* %_NOCOMMIT% TODO
            XML::FDocument xml;
            IBufferedStreamReader* reader = &response.Body();
            if (not XML::FDocument::Load(&xml, L"network.tmp", reader))
                AssertNotReached();

            GStdout << "XML:" << Eol;
            GStdout << xml << Eol;
            */
        }
        else {
            LOG(Test_Network, Error, L"HTTP request to '{0}' failed : {1}", uri, response.Status());
        }
    }
}
//----------------------------------------------------------------------------
static void Test_HttpPost_() {
    FUri uri;
    if (not FUri::Parse(uri, MakeStringView("http://www.w3schools.com/php/demo_form_validation_complete.php")))
        AssertNotReached();

    FHttpClient::FPostMap post = {
        {"name",       "Tom Cum von Bernardo $" },
        {"email",      "tom.cum@gmail.com"      },
        {"website",    "www.tom-cum.com"        },
        {"gender",     "don't be so tacky"      },
    };

    FHttpResponse response;
    if (EHttpStatus::OK != HttpPost(&response, uri, post))
        AssertNotReached();

    LOG(Test_Network, Info, L"Status: {0}", response.Status());
    LOG(Test_Network, Info, L"Reason: {0}", response.Reason());

    LOG(Test_Network, Info, L"Headers   :");
    for (const auto& it : response.Headers())
        LOG(Test_Network, Info, L" - '{0}' : '{1}'", it.first, it.second);

    LOG(Test_Network, Info, L"Body : {0} =\n{1}", 
        Fmt::FSizeInBytes{ checked_cast<size_t>(response.Body().SizeInBytes()) }, 
        response.MakeView() );
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
        LOG(Test_Network, Info, L"[{0}] accept connection on test server : {1}", socket.Local(), socket.Remote());
    }

    virtual void OnRequest(FSocketBuffered& socket, const FHttpRequest& request) const CORE_THROW() override {
        LOG(Test_Network, Info, L"[{0}] {1} requested : {2}", socket.Local(), socket.Remote(), request.Uri());

        FHttpResponse response;
        response.SetStatus(EHttpStatus::OK);
        response.SetReason("OK");

        FTextWriter oss(&response.Body());

        oss << "<html>" << Eol
            << "    <body>" << Eol;

        oss << "Method: " << request.Method() << "<br/>" << Eol;
        oss << "Uri   : " << request.Uri() << "<br/>" << Eol;

        oss << "Headers:" << "<br/>" << Eol;
        for (const auto& it : request.Headers())
            oss << " - '" << it.first << "' : '" << it.second << "'" << "<br/>" << Eol;

        oss << "<br/>" << Eol;

        oss << "Body:" << "<br/>" << Eol;
        oss << request.MakeView() << "<br/>" << Eol;

        oss << "    </body>" << Eol
            << "</html>" << Eol;

        oss.Flush();

        FHttpResponse::Write(&socket, response);
    }

    virtual void OnDisconnect(FSocketBuffered& socket) const override{
        LOG(Test_Network, Info, L"[{0}] disconnected from test server : {1}", socket.Local(), socket.Remote());
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
    LOG(Test_Network, Emphasis, L"starting network tests ...");

    Test_ParseUri_();
    Test_HttpGet_();
    Test_HttpPost_();
#if not defined(PROFILING_ENABLED) && not defined(FINAL_RELEASE)
    Test_SocketAccept_();
    Test_HttpServer_();
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Test
} //!namespace Core
