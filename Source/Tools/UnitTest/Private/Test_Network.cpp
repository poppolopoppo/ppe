#include "stdafx.h"

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

#if USE_PPE_LOGGER
    LOG(Test_Network, Info, L"Args[{0}]:", args.size());
    for (const auto& it : args)
        LOG(Test_Network, Info, L"    - {0} = '{1}'", it.first, it.second);
#endif

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
static void LogRequest_(const FHttpRequest& req) {
#if USE_PPE_LOGGER
    FWStringBuilder oss;
    oss << L"#HTTP_REQUEST" << Eol
        << L"> URI      : " << req.Uri() << Eol
        << L"> Method   : " << req.Method() << Eol
        << L"> Headers  : " << Eol;

    for (const auto& it : req.Headers()) {
        Format(oss, L" - {{0}} = \"{1}\"", it.first, it.second);
        oss << Eol;
    }

    FHttpRequest::FCookieMap cookies;
    if (FHttpRequest::UnpackCookie(&cookies, req)) {
        oss << L"> Cookies  :" << Eol;
        for (const auto& it : cookies) {
            Format(oss, L" - {{0}} = \"{1}\"", it.first, it.second);
            oss << Eol;
        }
    }

    FLogger::Log(
        LOG_CATEGORY_GET(Test_Network),
        FLogger::EVerbosity::Debug,
        FLogger::FSiteInfo::Make(WIDESTRING(__FILE__), __LINE__),
        oss.Written() );

#else
    UNUSED(req);
#endif
}
//----------------------------------------------------------------------------
static void LogResponse_(const FHttpResponse& resp) {
#if USE_PPE_LOGGER
    FWStringBuilder oss;
    oss << L"#HTTP_RESPONSE" << Eol
        << L"> Status   : " << resp.Status() << Eol
        << L"> Reason   : " << resp.Reason() << Eol
        << L"> Headers  : " << Eol;

    for (const auto& it : resp.Headers()) {
        Format(oss, L" - {{0}} = \"{1}\"", it.first, it.second);
        oss << Eol;
    }

    oss << L"> Body     : " << Fmt::FSizeInBytes{ checked_cast<size_t>(resp.Body().SizeInBytes()) } << Eol
        << resp.MakeView();

    FLogger::Log(
        LOG_CATEGORY_GET(Test_Network),
        FLogger::EVerbosity::Debug,
        FLogger::FSiteInfo::Make(WIDESTRING(__FILE__), __LINE__),
        oss.Written());

#else
    UNUSED(resp);
#endif
}
//----------------------------------------------------------------------------
#if WITH_PPE_NETWORK_Test_SocketAccept_
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

            PPE_TRY
            {
                FHttpRequest request;
                if (not FHttpRequest::Read(&request, socket, maxContentLength))
                    continue;

                LogRequest_(request);

                FHttpResponse response;

                if (EndsWithI(request.Uri().Path(), "favicon.ico")) {
                    response.SetStatus(EHttpStatus::NotFound);
                }
                else {
                    response.SetStatus(EHttpStatus::OK);
                    response.Body().WriteView("<marquee width='100%'>");
                    response.Body().WriteView(ToString(FDateTime::Now()));
                    response.Body().WriteView("</marquee>");
                    response.UpdateContentHeaders("text/html; charset=UTF-8");
                }

                LogResponse_(response);

                succeed &= FHttpResponse::Write(&socket, response);
            }
            PPE_CATCH(FHttpException e)
            PPE_CATCH_BLOCK({
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
#endif //!WITH_PPE_NETWORK_Test_SocketAccept_
//----------------------------------------------------------------------------
static void Test_HttpGet_() {
    FUri uri;
    if (not FUri::Parse(uri, MakeStringView("http://frostiebek.free.fr/datas/sample.json?fake=arg")))
        AssertNotReached();

    forrange(i, 0, 3) {
        FHttpResponse response;
        if (EHttpStatus::OK != HttpGet(&response, uri))
            AssertNotReached();

        LogResponse_(response);

        if (response.Status() == Network::EHttpStatus::OK) {
            Serialize::FJson json;
            IBufferedStreamReader* reader = &response.Body();
            if (not Serialize::FJson::Load(&json, MakeStringView(L"network.tmp"), reader))
                LOG(Test_Network, Error, L"request failed for {0}", uri);
            else
                LOG(Test_Network, Info, L"request result:\n{0}", json);
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
    srv.Stop();
}
#endif //!WITH_PPE_NETWORK_Test_HttpServer_
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void Test_Network() {
    PPE_DEBUG_NAMEDSCOPE("Test_Network");

    LOG(Test_Network, Emphasis, L"starting network tests ...");

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
