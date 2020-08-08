#include "stdafx.h"

#include "Http/ConstNames.h"

#include "Meta/AlignedStorage.h"

#define FOREACH_HTTPHEADERS(_Macro) \
    _Macro(FHttpHeaders,    Accept,                     "Accept") \
    _Macro(FHttpHeaders,    AcceptCharset,              "Accept-Charset") \
    _Macro(FHttpHeaders,    AcceptEncoding,             "Accept-Encoding") \
    _Macro(FHttpHeaders,    AcceptLanguage,             "Accept-Language") \
    _Macro(FHttpHeaders,    CacheControl,               "Cache-Control") \
    _Macro(FHttpHeaders,    Connection,                 "Connection") \
    _Macro(FHttpHeaders,    Cookie,                     "Cookie") \
    _Macro(FHttpHeaders,    ContentLanguage,            "Content-Language") \
    _Macro(FHttpHeaders,    ContentLength,              "Content-Length") \
    _Macro(FHttpHeaders,    ContentType,                "Content-Type") \
    _Macro(FHttpHeaders,    Date,                       "Date") \
    _Macro(FHttpHeaders,    Host,                       "Host") \
    _Macro(FHttpHeaders,    KeepAlive,                  "Keep-Alive") \
    _Macro(FHttpHeaders,    Location,                   "Location") \
    _Macro(FHttpHeaders,    Referer,                    "Referer") \
    _Macro(FHttpHeaders,    RetryAfter,                 "RetryAfter") \
    _Macro(FHttpHeaders,    Server,                     "Server") \
    _Macro(FHttpHeaders,    Status,                     "Status") \
    _Macro(FHttpHeaders,    UserAgent,                  "User-Agent")

#define FOREACH_MIMETYPES(_Macro) \
    _Macro(FMimeTypes,      Application_javascript,     "application/javascript") \
    _Macro(FMimeTypes,      Application_json,           "application/json") \
    _Macro(FMimeTypes,      Application_octet_stream,   "application/octet-stream") \
    _Macro(FMimeTypes,      Application_ogg,            "application/ogg") \
    _Macro(FMimeTypes,      Application_pdf,            "application/pdf") \
    _Macro(FMimeTypes,      Application_xhtml_xml,      "application/xhtml+xml") \
    _Macro(FMimeTypes,      Application_xml,            "application/xml") \
    _Macro(FMimeTypes,      Application_zip,            "application/zip") \
    _Macro(FMimeTypes,      Audio_mpeg,                 "audio/mpeg") \
    _Macro(FMimeTypes,      Audio_wav,                  "audio/wav") \
    _Macro(FMimeTypes,      Image_gif,                  "image/gif") \
    _Macro(FMimeTypes,      Image_jpeg,                 "image/jpeg") \
    _Macro(FMimeTypes,      Image_png,                  "image/png") \
    _Macro(FMimeTypes,      Image_svg_xml,              "image/svg+xml") \
    _Macro(FMimeTypes,      Image_tiff,                 "image/tiff") \
    _Macro(FMimeTypes,      Text_css,                   "text/css") \
    _Macro(FMimeTypes,      Text_csv,                   "text/csv") \
    _Macro(FMimeTypes,      Text_html,                  "text/html") \
    _Macro(FMimeTypes,      Text_javascript,            "text/javascript")

namespace PPE {
namespace Network {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
STATIC_ASSERT(Meta::is_pod_v<POD_STORAGE(FName)>);
#define DEF_HTTPCONSTNAMES_STORAGE(_Type, _Name, _Content) \
    static POD_STORAGE(FName) CONCAT(G, CONCAT(_Type, _Name));
FOREACH_HTTPHEADERS(DEF_HTTPCONSTNAMES_STORAGE)
FOREACH_MIMETYPES(DEF_HTTPCONSTNAMES_STORAGE)
#undef DEF_HTTPCONSTNAMES_STORAGE
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define DEF_HTTPCONSTNAMES_ACCESSOR(_Type, _Name, _Content) \
    const FName& _Type::_Name() { return *reinterpret_cast<const FName*>(&CONCAT(G, CONCAT(_Type, _Name))); }
FOREACH_HTTPHEADERS(DEF_HTTPCONSTNAMES_ACCESSOR)
FOREACH_MIMETYPES(DEF_HTTPCONSTNAMES_ACCESSOR)
#undef DEF_HTTPCONSTNAMES_ACCESSOR
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FHttpConstNames::Start() {
#define DEF_HTTPCONSTNAMES_STARTUP(_Type, _Name, _Content) \
    INPLACE_NEW(&CONCAT(G, CONCAT(_Type, _Name)), FName){ MakeStringView(_Content) };
FOREACH_HTTPHEADERS(DEF_HTTPCONSTNAMES_STARTUP)
FOREACH_MIMETYPES(DEF_HTTPCONSTNAMES_STARTUP)
#undef DEF_HTTPCONSTNAMES_STARTUP
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FHttpConstNames::Shutdown() {
#define DEF_HTTPCONSTNAMES_SHUTDOWN(_Type, _Name, _Content) \
    Meta::Destroy(reinterpret_cast<const FName*>(&CONCAT(G, CONCAT(_Type, _Name))));
FOREACH_HTTPHEADERS(DEF_HTTPCONSTNAMES_SHUTDOWN)
FOREACH_MIMETYPES(DEF_HTTPCONSTNAMES_SHUTDOWN)
#undef DEF_HTTPCONSTNAMES_SHUTDOWN
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace PPE

#undef FOREACH_MIMETYPES
#undef FOREACH_HTTPHEADERS
