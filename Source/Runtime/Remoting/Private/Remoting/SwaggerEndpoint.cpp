﻿// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Remoting/SwaggerEndpoint.h"

#include "RemotingServer.h"
#include "Http/ConstNames.h"
#include "Remoting/OpenAPI.h"

#include "Http/Request.h"
#include "Http/Response.h"
#include "Http/Status.h"

#include "IO/Format.h"
#include "IO/String.h"
#include "IO/StringBuilder.h"
#include "IO/TextFormat.h"

namespace PPE {
namespace Remoting {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FSwaggerEndpoint::FSwaggerEndpoint() NOEXCEPT
:   _prefix("/swagger") {
}
//----------------------------------------------------------------------------
FSwaggerEndpoint::~FSwaggerEndpoint() {
}
//----------------------------------------------------------------------------
FString FSwaggerEndpoint::SwaggerApi(const FRemotingServer& server) const {
    return StringFormat(
        "http://{0}:{1}{2}/api",
        server.Localhost().Host(),
        server.Localhost().Port(),
        _prefix);
}
//----------------------------------------------------------------------------
void FSwaggerEndpoint::PrivateEndpointOpenAPI(FOpenAPI& ) const {
    NOOP(); // this is handled bellow
}
//----------------------------------------------------------------------------
void FSwaggerEndpoint::PrivateEndpointProcess(const FRemotingContext& ctx, const FStringView& relativePath) {
    if (not EqualsI(relativePath, "api")) {
        ctx.NotFound(relativePath);
        return;
    }

    FOpenAPI api;
    api.Header(
        "PPE remoting API",
        "1.0",
        "Pilot the engine through REST",
        TextFormat(api.Content.Allocator(), "http://{}:{}", ctx.Localhost.Host(), ctx.Localhost.Port()) );
    api.DefineRTTISchemas();

    for (const IRemotingEndpoint& endpoint : ctx.Endpoints)
        endpoint.EndpointOpenAPI(&api);

    Meta::BubbleSort(api.Paths->begin(), api.Paths->end(),
                     [](const auto& a, const auto& b) NOEXCEPT -> bool {
                         return Meta::TLess<>{}( a.key, b.key );
                     });

    ctx.pResponse->SetStatus(Network::EHttpStatus::OK);
    ctx.pResponse->HTTP_SetContentType(Network::FMimeTypes::Application_json());

    FTextWriter oss(&ctx.pResponse->Body());
    api.Content.ToStream(oss);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Remoting
} //!namespace PPE
