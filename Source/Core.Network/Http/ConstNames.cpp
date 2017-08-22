#include "stdafx.h"

#include "ConstNames.h"

#include "Core/Meta/AlignedStorage.h"

#define FOREACH_HTTPCONSTNAMES(_Macro) \
    _Macro(FName,           Accept,             "Accept") \
    _Macro(FName,           AcceptCharset,      "Accept-Charset") \
    _Macro(FName,           AcceptEncoding,     "Accept-Encoding") \
    _Macro(FName,           AcceptLanguage,     "Accept-Language") \
    _Macro(FName,           CacheControl,       "Cache-Control") \
    _Macro(FName,           Connection,         "Connection") \
    _Macro(FName,           Cookie,             "Cookie") \
    _Macro(FName,           ContentLanguage,    "Content-Language") \
    _Macro(FName,           ContentLength,      "Content-Length") \
    _Macro(FName,           ContentType,        "Content-Type") \
    _Macro(FName,           Date,               "Date") \
    _Macro(FName,           Host,               "Host") \
    _Macro(FName,           KeepAlive,          "Keep-Alive") \
    _Macro(FName,           Location,           "Location") \
    _Macro(FName,           Referer,            "Referer") \
    _Macro(FName,           RetryAfter,         "RetryAfter") \
    _Macro(FName,           Server,             "Server") \
    _Macro(FName,           Status,             "Status") \
    _Macro(FName,           UserAgent,          "User-Agent")

namespace Core {
namespace Network {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
#define DEF_HTTPCONSTNAMES_STORAGE(_Type, _Name, _Content) \
    static POD_STORAGE(_Type) CONCAT(gPod_##_Type##_, _Name);
FOREACH_HTTPCONSTNAMES(DEF_HTTPCONSTNAMES_STORAGE)
#undef DEF_HTTPCONSTNAMES_STORAGE
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define DEF_HTTPCONSTNAMES_ACCESSOR(_Type, _Name, _Content) \
    const _Type& FHttpConstNames::_Name() { return *reinterpret_cast<const _Type *>(&CONCAT(gPod_##_Type##_, _Name)); }
FOREACH_HTTPCONSTNAMES(DEF_HTTPCONSTNAMES_ACCESSOR)
#undef DEF_HTTPCONSTNAMES_ACCESSOR
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FHttpConstNames::Start() {
#define DEF_HTTPCONSTNAMES_STARTUP(_Type, _Name, _Content) \
    new ((void *)&CONCAT(gPod_##_Type##_, _Name)) _Type(MakeStringView(_Content));
    FOREACH_HTTPCONSTNAMES(DEF_HTTPCONSTNAMES_STARTUP)
#undef DEF_HTTPCONSTNAMES_STARTUP
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FHttpConstNames::Shutdown() {
#define DEF_HTTPCONSTNAMES_SHUTDOWN(_Type, _Name, _Content) \
    reinterpret_cast<const _Type *>(&CONCAT(gPod_##_Type##_, _Name))->~_Type();
    FOREACH_HTTPCONSTNAMES(DEF_HTTPCONSTNAMES_SHUTDOWN)
#undef DEF_HTTPCONSTNAMES_SHUTDOWN
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace Core

#undef FOREACH_HTTPCONSTNAMES
