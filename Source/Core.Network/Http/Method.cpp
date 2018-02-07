#include "stdafx.h"

#include "Method.h"

#include "Core/IO/TextWriter.h"

namespace Core {
namespace Network {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FStringView HttpMethodToCStr(EHttpMethod method) {
    switch (method)
    {
    case Core::Network::EHttpMethod::Get:
        return "GET";
    case Core::Network::EHttpMethod::Head:
        return "HEAD";
    case Core::Network::EHttpMethod::Post:
        return "POST";
    case Core::Network::EHttpMethod::Put:
        return "PUT";
    case Core::Network::EHttpMethod::Delete:
        return "DELETE";
    case Core::Network::EHttpMethod::Trace:
        return "TRACE";
    case Core::Network::EHttpMethod::Options:
        return "OPTIONS";
    case Core::Network::EHttpMethod::Connect:
        return "CONNECT";
    case Core::Network::EHttpMethod::Patch:
        return "PATCH";
    }
    return FStringView();
}
//----------------------------------------------------------------------------
bool HttpMethodFromCStr(EHttpMethod* method, const FStringView& str) {
    Assert(method);

    if      (EqualsI(str, "GET")) {
        *method = EHttpMethod::Get;
        return true;
    }
    else if (EqualsI(str, "HEAD")) {
        *method = EHttpMethod::Head;
        return true;
    }
    else if (EqualsI(str, "POST")) {
        *method = EHttpMethod::Post;
        return true;
    }
    else if (EqualsI(str, "PUT")) {
        *method = EHttpMethod::Put;
        return true;
    }
    else if (EqualsI(str, "DELETE")) {
        *method = EHttpMethod::Delete;
        return true;
    }
    else if (EqualsI(str, "TRACE")) {
        *method = EHttpMethod::Trace;
        return true;
    }
    else if (EqualsI(str, "OPTIONS")) {
        *method = EHttpMethod::Options;
        return true;
    }
    else if (EqualsI(str, "CONNECT")) {
        *method = EHttpMethod::Connect;
        return true;
    }
    else if (EqualsI(str, "PATCH")) {
        *method = EHttpMethod::Patch;
        return true;
    }

    return false;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace Core

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, Network::EHttpMethod httpMethod) {
    return oss << Network::HttpMethodToCStr(httpMethod);
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, Network::EHttpMethod httpMethod) {
    return oss << Network::HttpMethodToCStr(httpMethod);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
