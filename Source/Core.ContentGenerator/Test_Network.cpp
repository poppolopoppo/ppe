#include "stdafx.h"

#include "Core.Network/Socket/Address.h"
#include "Core.Network/Socket/Listener.h"
#include "Core.Network/Socket/SocketBuffered.h"
#include "Core.Network/Uri.h"

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

    bool succeed = false;
    forrange(i, 0, 20) {
        FSocket socket;

        std::cout << "Listening on '" << listener.Listening() << "' ..." << std::endl;

        if (listener.Accept(socket, Seconds(5))) {
            succeed = true;
            std::cout << "Accepted from '" << socket.Local() << "' to '" << socket.Remote() << "' :)" << std::endl;

            u8 buffer[2048];
            const size_t read = socket.Read(buffer, true);

            std:: cout << "Read[" << read << "] = '" << FStringView((const char*)buffer, read) << "'" << std::endl;

            socket.Disconnect(true);
            break;
        }
    }

    if (not succeed)
        std::cout << "No incomming connection :'(" << std::endl;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void Test_Network() {
    Test_ParseUri_();
    Test_SocketAccept_();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentGenerator
} //!namespace Core
