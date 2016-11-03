#pragma once

#include "Core.Network/Network.h"

#include "Core/IO/StringView.h"

namespace Core {
namespace Network {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EHttpMethod {
    Get = 0,
    Head,
    Post,
    Put,
    Delete,
    Trace,
    Options,
    Connect,
    Patch,
};
//----------------------------------------------------------------------------
FStringView HttpMethodToCStr(EHttpMethod method);
bool HttpMethodFromCStr(EHttpMethod* method, const FStringView& str);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace Core

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(
    std::basic_ostream<_Char, _Traits>& oss,
    Network::EHttpMethod httpMethod ) {
    return oss << Network::HttpMethodToCStr(httpMethod);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
