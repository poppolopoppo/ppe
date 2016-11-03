#pragma once

#include "Core.Network/Network.h"

#include "Core/IO/StringView.h"

#define FOREACH_HTTP_STATUSCODE(_MACRO) \
    \
    /* 1xx: Information */ \
    \
    _MACRO(Continue                        , 100 ,  "The server has received the request headers, and the client should proceed to send the request body") \
    _MACRO(Switching                       , 101 ,  "Protocols The requester has asked the server to switch protocols") \
    _MACRO(Checkpoint                      , 103 ,  "Used in the resumable requests proposal to resume aborted PUT or POST requests") \
    \
    /* 2xx: Successful */ \
    \
    _MACRO(OK                              , 200 ,  "The request is OK (this is the standard response for successful HTTP requests)") \
    _MACRO(Created                         , 201 ,  "The request has been fulfilled, and a new resource is created") \
    _MACRO(Accepted                        , 202 ,  "The request has been accepted for processing, but the processing has not been completed") \
    _MACRO(NonAuthoritativeInformation     , 203 ,  "The request has been successfully processed, but is returning information that may be from another source") \
    _MACRO(NoContent                       , 204 ,  "The request has been successfully processed, but is not returning any content") \
    _MACRO(ResetContent                    , 205 ,  "The request has been successfully processed, but is not returning any content, and requires that the requester reset the document view") \
    _MACRO(PartialContent                  , 206 ,  "The server is delivering only part of the resource due to a range header sent by the client") \
    \
    /* 3xx: Redirection */ \
    \
    _MACRO(MultipleChoices                 , 300 ,  "A link list. The user can select a link and go to that location. Maximum five addresses") \
    _MACRO(MovedPermanently                , 301 ,  "The requested page has moved to a new URL") \
    _MACRO(Found                           , 302 ,  "The requested page has moved temporarily to a new URL") \
    _MACRO(SeeOther                        , 303 ,  "The requested page can be found under a different URL") \
    _MACRO(NotModified                     , 304 ,  "Indicates the requested page has not been modified since last requested") \
    _MACRO(SwitchProxy                     , 306 ,  "No longer used") \
    _MACRO(TemporaryRedirect               , 307 ,  "The requested page has moved temporarily to a new URL") \
    _MACRO(ResumeIncomplete                , 308 ,  "Used in the resumable requests proposal to resume aborted PUT or POST requests") \
    \
    /* 4xx: Client Error */ \
    \
    _MACRO(BadRequest                      , 400 ,  "The request cannot be fulfilled due to bad syntax") \
    _MACRO(Unauthorized                    , 401 ,  "The request was a legal request, but the server is refusing to respond to it. For use when authentication is possible but has failed or not yet been provided") \
    _MACRO(PaymentRequired                 , 402 ,  "Reserved for future use") \
    _MACRO(Forbidden                       , 403 ,  "The request was a legal request, but the server is refusing to respond to it") \
    _MACRO(NotFound                        , 404 ,  "The requested page could not be found but may be available again in the future") \
    _MACRO(MethodNotAllowed                , 405 ,  "A request was made of a page using a request method not supported by that page") \
    _MACRO(NotAcceptable                   , 406 ,  "The server can only generate a response that is not accepted by the client") \
    _MACRO(ProxyAuthenticationRequired     , 407 ,  "The client must first authenticate itself with the proxy") \
    _MACRO(Request                         , 408 ,  "Timeout The server timed out waiting for the request") \
    _MACRO(Conflict                        , 409 ,  "The request could not be completed because of a conflict in the request") \
    _MACRO(Gone                            , 410 ,  "The requested page is no longer available") \
    _MACRO(LengthRequired                  , 411 ,  "The 'Content-Length' is not defined. The server will not accept the request without it") \
    _MACRO(PreconditionFailed              , 412 ,  "The precondition given in the request evaluated to false by the server") \
    _MACRO(RequestEntityTooLarge           , 413 ,  "The server will not accept the request, because the request entity is too large") \
    _MACRO(RequestURITooLong               , 414 ,  "The server will not accept the request, because the URL is too long. Occurs when you convert a POST request to a GET request with a long query information") \
    _MACRO(UnsupportedMediaType            , 415 ,  "The server will not accept the request, because the media type is not supported") \
    _MACRO(RequestedRangeNotSatisfiable    , 416 ,  "The client has asked for a portion of the file, but the server cannot supply that portion") \
    _MACRO(ExpectationFailed               , 417 ,  "The server cannot meet the requirements of the Expect request-header field") \
    \
    /* 5xx: Server Error */ \
    \
    _MACRO(InternalServerError             , 500 ,  "A generic error message, given when no more specific message is suitable") \
    _MACRO(NotImplemented                  , 501 ,  "The server either does not recognize the request method, or it lacks the ability to fulfill the request") \
    _MACRO(BadGateway                      , 502 ,  "The server was acting as a gateway or proxy and received an invalid response from the upstream server") \
    _MACRO(ServiceUnavailable              , 503 ,  "The server is currently unavailable (overloaded or down)") \
    _MACRO(GatewayTimeout                  , 504 ,  "The server was acting as a gateway or proxy and did not receive a timely response from the upstream server") \
    _MACRO(HTTPVersionNotSupported         , 505 ,  "The server does not support the HTTP protocol version used in the request") \
    _MACRO(NetworkAuthentication           , 511 ,  "Required The client needs to authenticate to gain network access")

namespace Core {
namespace Network {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EHttpStatus {
#define HTTP_ERRORCODE_ENUMDEF(_Name, _Value, _Desc) \
    _Name = _Value,
FOREACH_HTTP_STATUSCODE(HTTP_ERRORCODE_ENUMDEF)
#undef HTTP_ERRORCODE_ENUMDEF
};
//----------------------------------------------------------------------------
FStringView HttpStatusName(EHttpStatus status);
FStringView HttpStatusCode(EHttpStatus status);
FStringView HttpStatusDescription(EHttpStatus status);
//----------------------------------------------------------------------------
inline bool HttpIsInformation(EHttpStatus status) { return (size_t(status) >= 100 && size_t(status) < 200); }
inline bool HttpIsSuccessful (EHttpStatus status) { return (size_t(status) >= 200 && size_t(status) < 300); }
inline bool HttpIsRedirection(EHttpStatus status) { return (size_t(status) >= 300 && size_t(status) < 400); }
inline bool HttpIsClientError(EHttpStatus status) { return (size_t(status) >= 400 && size_t(status) < 500); }
inline bool HttpIsServerError(EHttpStatus status) { return (size_t(status) >= 500 && size_t(status) < 600); }
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
    Network::EHttpStatus httpStatus ) {
    return oss
        << Network::HttpStatusCode(httpStatus)
        << " - " << Network::HttpStatusName(httpStatus)
        << " : " << Network::HttpStatusDescription(httpStatus);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
