#pragma once

#include "Network.h"

#include "NetworkName.h"

namespace PPE {
namespace Network {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FHttpHeaders {
    static PPE_NETWORK_API const FName& Accept();
    static PPE_NETWORK_API const FName& AcceptCharset();
    static PPE_NETWORK_API const FName& AcceptEncoding();
    static PPE_NETWORK_API const FName& AcceptLanguage();
    static PPE_NETWORK_API const FName& AccessControlAllowOrigin();
    static PPE_NETWORK_API const FName& CacheControl();
    static PPE_NETWORK_API const FName& Connection();
    static PPE_NETWORK_API const FName& Cookie();
    static PPE_NETWORK_API const FName& ContentLanguage();
    static PPE_NETWORK_API const FName& ContentLength();
    static PPE_NETWORK_API const FName& ContentType();
    static PPE_NETWORK_API const FName& Date();
    static PPE_NETWORK_API const FName& Host();
    static PPE_NETWORK_API const FName& KeepAlive();
    static PPE_NETWORK_API const FName& Location();
    static PPE_NETWORK_API const FName& Referer();
    static PPE_NETWORK_API const FName& RetryAfter();
    static PPE_NETWORK_API const FName& Server();
    static PPE_NETWORK_API const FName& Status();
    static PPE_NETWORK_API const FName& UserAgent();
};
//----------------------------------------------------------------------------
struct FMimeTypes {
    static PPE_NETWORK_API const FName& Application_javascript();
    static PPE_NETWORK_API const FName& Application_json();
    static PPE_NETWORK_API const FName& Application_octet_stream();
    static PPE_NETWORK_API const FName& Application_ogg();
    static PPE_NETWORK_API const FName& Application_pdf();
    static PPE_NETWORK_API const FName& Application_xhtml_xml();
    static PPE_NETWORK_API const FName& Application_xml();
    static PPE_NETWORK_API const FName& Application_x_www_form_urlencoded();
    static PPE_NETWORK_API const FName& Application_zip();
    static PPE_NETWORK_API const FName& Audio_mpeg();
    static PPE_NETWORK_API const FName& Audio_wav();
    static PPE_NETWORK_API const FName& Image_gif();
    static PPE_NETWORK_API const FName& Image_jpeg();
    static PPE_NETWORK_API const FName& Image_png();
    static PPE_NETWORK_API const FName& Image_svg_xml();
    static PPE_NETWORK_API const FName& Image_tiff();
    static PPE_NETWORK_API const FName& Text_css();
    static PPE_NETWORK_API const FName& Text_csv();
    static PPE_NETWORK_API const FName& Text_html();
    static PPE_NETWORK_API const FName& Text_javascript();
};
//----------------------------------------------------------------------------
struct FHttpConstNames {
    static PPE_NETWORK_API void Start();
    static PPE_NETWORK_API void Shutdown();
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace PPE
