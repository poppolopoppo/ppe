// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Http/Method.h"

#include "IO/TextWriter.h"

namespace PPE {
namespace Network {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FStringLiteral HttpMethodToCStr(EHttpMethod method) {
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

    if      (EqualsI(str, "GET"_view)) {
        *method = EHttpMethod::Get;
        return true;
    }
    else if (EqualsI(str, "HEAD"_view)) {
        *method = EHttpMethod::Head;
        return true;
    }
    else if (EqualsI(str, "POST"_view)) {
        *method = EHttpMethod::Post;
        return true;
    }
    else if (EqualsI(str, "PUT"_view)) {
        *method = EHttpMethod::Put;
        return true;
    }
    else if (EqualsI(str, "DELETE"_view)) {
        *method = EHttpMethod::Delete;
        return true;
    }
    else if (EqualsI(str, "TRACE"_view)) {
        *method = EHttpMethod::Trace;
        return true;
    }
    else if (EqualsI(str, "OPTIONS"_view)) {
        *method = EHttpMethod::Options;
        return true;
    }
    else if (EqualsI(str, "CONNECT"_view)) {
        *method = EHttpMethod::Connect;
        return true;
    }
    else if (EqualsI(str, "PATCH"_view)) {
        *method = EHttpMethod::Patch;
        return true;
    }

    return false;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, EHttpMethod httpMethod) {
    return oss << HttpMethodToCStr(httpMethod);
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, EHttpMethod httpMethod) {
    return oss << HttpMethodToCStr(httpMethod);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace PPE
