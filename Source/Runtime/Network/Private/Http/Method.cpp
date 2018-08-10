#include "stdafx.h"

#include "Http/Method.h"

#include "IO/TextWriter.h"

namespace PPE {
namespace Network {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FStringView HttpMethodToCStr(EHttpMethod method) {
    switch (method) {
    case PPE::Network::EHttpMethod::Get:
        return "GET";
    case PPE::Network::EHttpMethod::Head:
        return "HEAD";
    case PPE::Network::EHttpMethod::Post:
        return "POST";
    case PPE::Network::EHttpMethod::Put:
        return "PUT";
    case PPE::Network::EHttpMethod::Delete:
        return "DELETE";
    case PPE::Network::EHttpMethod::Trace:
        return "TRACE";
    case PPE::Network::EHttpMethod::Options:
        return "OPTIONS";
    case PPE::Network::EHttpMethod::Connect:
        return "CONNECT";
    case PPE::Network::EHttpMethod::Patch:
        return "PATCH";
    }
    AssertNotImplemented();
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
} //!namespace PPE

namespace PPE {
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
} //!namespace PPE
