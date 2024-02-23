// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Http/Client.h"
#include "Http/Exceptions.h"
#include "Http/Method.h"
#include "Http/Server.h"
#include "Http/Status.h"
#include "Http/Request.h"
#include "Http/Response.h"
#include "Socket/Address.h"
#include "Socket/Listener.h"
#include "Socket/SocketBuffered.h"
#include "Uri.h"

#include "Json/Json.h"
#include "Markup/Xml.h"

#include "Diagnostic/Logger.h"
#include "IO/Filename.h"
#include "IO/FileStream.h"
#include "IO/FormatHelpers.h"
#include "IO/String.h"
#include "IO/StringBuilder.h"
#include "IO/StringView.h"
#include "IO/TextWriter.h"
#include "Time/DateTime.h"

// #TODO : wrap RTTI meta classes with OpenAPI/Swagger (https://editor.swagger.io/)

#define WITH_PPE_NETWORK_INTERACTIVE_TESTS (not USE_PPE_FINAL_RELEASE && not USE_PPE_PROFILING)

#define WITH_PPE_NETWORK_Test_SocketAccept_ (WITH_PPE_NETWORK_INTERACTIVE_TESTS && 1) //%_NOCOMMIT%
#define WITH_PPE_NETWORK_Test_HttpServer_   (WITH_PPE_NETWORK_INTERACTIVE_TESTS && 1) //%_NOCOMMIT%

namespace PPE {
namespace Test {
LOG_CATEGORY(, Test_Network)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
using namespace PPE::Network;
//----------------------------------------------------------------------------
static bool ParseUri_(const FStringView& str) {
    FUri uri;
    if (not FUri::Parse(uri, str))
        return false;

    PPE_LOG(Test_Network, Info,
        "{0}\n"
        "   Scheme: {0}\n"
        "   Username: {1}\n"
        "   Hostname: {2}\n"
        "   Post: {3}\n"
        "   Path: {4}\n"
        "   Query: {5}\n"
        "   Anchor: {6}",
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

#if USE_PPE_LOGGER
    PPE_LOG(Test_Network, Info, "Args[{0}]:", args.size());
    for (const auto& it : args)
        PPE_LOG(Test_Network, Info, "    - {0} = '{1}'", it.first, it.second);
#endif

    FUri test;
    if (not FUri::Pack(test, uri.Scheme(), uri.Username(), uri.Port(), uri.Hostname(), uri.Path(), args, uri.Anchor()))
        return false;

    PPE_LOG(Test_Network, Info, "{0}\n -> {1}", uri, test);

    if (test.Str() != uri.Str())
        return false;

    return true;
}
//----------------------------------------------------------------------------
static void Test_ParseUri_() {
    if (not ParseUri_("http://jquery.com/?s=toto+slip+123"_view))
        AssertNotReached();
    if (not ParseUri_("https://bek@www.google.fr/search?num=20&safe=off&site=&source=hp&q=mangeur+de+pingouin&oq=mangeur+de+pingouin&gs_l=hp.3..0i22i30k1.1022.5665.0.5988.22.19.1.2.3.0.112.1214.16j3.19.0....0...1c.1.64.hp..0.19.1065.0..0j35i39k1j0i67k1j0i131k1j0i13i30k1.lOSTdUm6eCQ#anchor"_view))
        AssertNotReached();
}
//----------------------------------------------------------------------------
static void LogRequest_(const FHttpRequest& req) {
    Unused(req);
    PPE_LOG_DIRECT(Test_Network, Debug, [&](FTextWriter& oss) {
        oss << "#HTTP_REQUEST" << Eol
            << "> URI      : " << req.Uri() << Eol
            << "> Method   : " << req.Method() << Eol
            << "> Headers  : " << Eol;

        for (const auto& it : req.Headers()) {
            Format(oss, " - {0} = \"{1}\"", it.first, it.second);
            oss << Eol;
        }

        FHttpRequest::FCookieMap cookies;
        if (FHttpRequest::UnpackCookie(&cookies, req)) {
            oss << "> Cookies  :" << Eol;
            for (const auto& it : cookies) {
                Format(oss, " - {0} = \"{1}\"", it.first, it.second);
                oss << Eol;
            }
        }
    });
}
//----------------------------------------------------------------------------
static void LogResponse_(const FHttpResponse& resp) {
    Unused(resp);
    PPE_LOG_DIRECT(Test_Network, Debug, [&](FTextWriter& oss) {
        oss << "#HTTP_RESPONSE" << Eol
            << "> Status   : " << resp.Status() << Eol
            << "> Reason   : " << resp.Reason() << Eol
            << "> Headers  : " << Eol;

        for (const auto& it : resp.Headers()) {
            Format(oss, " - {0} = \"{1}\"", it.first, it.second);
            oss << Eol;
        }

        oss << "> Body     : " << Fmt::FSizeInBytes{ checked_cast<size_t>(resp.Body().SizeInBytes()) } << Eol
            << resp.MakeView();
    });
}
//----------------------------------------------------------------------------
#if WITH_PPE_NETWORK_Test_SocketAccept_
static void Test_SocketAccept_() {
    FListener listener = FListener::Localhost(8123);

    const FListener::FConnectionScope connection(listener);
    if (not connection.Succeed) {
        PPE_LOG(Test_Network, Error, "failed to open listener on '{0}'", listener.Listening());
        return;
    }

    const size_t maxContentLength = size_t(FMegabytes(10).Value());

    bool succeed = false;
    forrange(i, 0, 20) {
        FSocketBuffered socket;
        socket.SetTimeout(FSeconds(0.3));

        PPE_LOG(Test_Network, Info, "Listening on '{0}'...", listener.Listening());

        if (FSocketBuffered::Accept(socket, listener, FSeconds(5))) {
            succeed = true;
            PPE_LOG(Test_Network, Info, "Accepted from '{0}' to '{1}' :)", socket.Local(), socket.Remote());

            PPE_TRY
            {
                FHttpRequest request;
                if (not FHttpRequest::Read(&request, socket, maxContentLength)) {
                    socket.Disconnect();
                    continue;
                }

                LogRequest_(request);

                FHttpResponse response;

                if (EndsWithI(request.Uri().Path(), "favicon.ico"_view)) {
                    response.SetStatus(EHttpStatus::NotFound);
                }
                else {
                    response.SetStatus(EHttpStatus::OK);
                    response.Body().WriteView("<marquee width='100%'>");
                    response.Body().WriteView(ToString(FDateTime::Now()));
                    response.Body().WriteView("</marquee>");
                    response.UpdateContentHeaders("text/html; charset=UTF-8"_view);
                }

                LogResponse_(response);

                succeed &= FHttpResponse::Write(&socket, response);
            }
            PPE_CATCH(FHttpException e)
            PPE_CATCH_BLOCK({
                Unused(e);
                PPE_LOG(Test_Network, Error, "HTTP error {0} : {1}", e.Status(), MakeCStringView(e.What()));
            });

            socket.Disconnect(true);
            break;
        }
    }

    if (not succeed)
        PPE_LOG(Test_Network, Warning, "no incomming HTTP connexion :'(");
}
#endif //!WITH_PPE_NETWORK_Test_SocketAccept_
//----------------------------------------------------------------------------
static void Test_HttpGet_() {
    FUri uri;
    if (not FUri::Parse(uri, MakeStringView("http://frostiebek.free.fr/datas/sample.json?fake=arg")))
        AssertNotReached();

    Serialize::FJson::FAllocator alloc;

    forrange(i, 0, 3) {
        FHttpResponse response;
        const EHttpStatus status = HttpGet(&response, uri);

        if (EHttpStatus::ServiceUnavailable == status) {
            PPE_LOG(Test_Network, Warning, "service unavailable for {0}", uri);
            return;
        }

        if (EHttpStatus::OK != status)
            AssertNotReached();

        LogResponse_(response);

        if (response.Status() == EHttpStatus::OK) {
            Serialize::FJson json{ alloc };
            if (not Serialize::FJson::Load(&json, MakeStringView(L"network.tmp"), &response.Body()))
                PPE_LOG(Test_Network, Error, "request failed for {0}", uri);
            else
                PPE_LOG(Test_Network, Info, "request result:\n{0}", json);
        }
        else {
            PPE_LOG(Test_Network, Error, "HTTP request to '{0}' failed : {1}", uri, response.Status());
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
    const EHttpStatus status = HttpPost(&response, uri, post);

    if (EHttpStatus::ServiceUnavailable == status) {
        PPE_LOG(Test_Network, Warning, "service unavailable for {0}", uri);
        return;
    }

    if (EHttpStatus::OK != status)
        AssertNotReached();

    LogResponse_(response);
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
    FTestHttpServer() : FHttpServer("localhost", FAddress::Localhost(8776)) {}

private:
    virtual bool OnRequest(FServicingPort& port, const FHttpRequest& request) const override {
        LogRequest_(request);

        FHttpResponse response;
        response.SetStatus(EHttpStatus::OK);
        response.SetReason("OK");

        FTextWriter oss(&response.Body());

        oss << "<html>" << Eol
            << "    <body>" << Eol;

        oss << "<h1>" << port.UID() << "</h1>" << Eol;

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

        return FHttpResponse::Write(&port.Socket(), response);
    }
};
//----------------------------------------------------------------------------
#if WITH_PPE_NETWORK_Test_HttpServer_
static void Test_HttpServer_() {
    FTestHttpServer srv;
    srv.Start(2);
    std::this_thread::sleep_for(std::chrono::seconds(30));
    srv.Shutdown();
}
#endif //!WITH_PPE_NETWORK_Test_HttpServer_
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void Test_Network() {
    PPE_DEBUG_NAMEDSCOPE("Test_Network");

    PPE_LOG(Test_Network, Emphasis, "starting network tests ...");

    Test_ParseUri_();
    Test_HttpGet_();
    Test_HttpPost_();
#if WITH_PPE_NETWORK_Test_SocketAccept_
    Test_SocketAccept_();
#endif
#if WITH_PPE_NETWORK_Test_HttpServer_
    Test_HttpServer_();
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Test
} //!namespace PPE
