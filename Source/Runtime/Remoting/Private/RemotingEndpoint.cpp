// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "RemotingEndpoint.h"

#include "RemotingServer.h"

#include "Container/AssociativeVector.h"
#include "Diagnostic/Logger.h"
#include "Http/Request.h"
#include "IO/FormatHelpers.h"
#include "Uri.h"
#include "Http/Response.h"
#include "Meta/Utility.h"

namespace PPE {
namespace Remoting {
EXTERN_LOG_CATEGORY(PPE_REMOTING_API, Remoting)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void IRemotingEndpoint::EndpointOpenAPI(FOpenAPI* api) const {
    Assert(api);
    PrivateEndpointOpenAPI(*api);
}
//----------------------------------------------------------------------------
void IRemotingEndpoint::EndpointProcess(const FRemotingContext& ctx) {
    Assert(ctx.pResponse);

    const FString& prefix = this->EndpointPrefix();
    PPE_LOG(Remoting, Debug, "Call endpoint <{0}> for '{1}'", prefix, ctx.Request.Uri());

    Assert_NoAssume(StartsWithI(ctx.Request.Uri().Path(), prefix));
    FStringView relativePath = ctx.Request.Uri().Path().CutStartingAt(prefix.length());
    while (relativePath.StartsWith(Network::FUri::PathSeparator))
        relativePath = relativePath.ShiftFront();

    // #TODO : benchmarking histogram for performance
    PrivateEndpointProcess(ctx, relativePath);
}
//----------------------------------------------------------------------------
bool IRemotingEndpoint::EndpointDispatchIFP(
    const FRemotingContext& ctx,
    FStringView relativePath,
    const FOperation& operation ) NOEXCEPT {
    if (ctx.Request.Method() != operation.Method)
        return false;

    ASSOCIATIVE_VECTORINSITU(Remoting, FStringView, FString, 8) pathArgs;

    for (const FStringView& part : operation.Path) {
        while (relativePath.StartsWith(Network::FUri::PathSeparator))
            relativePath = relativePath.ShiftFront();

        const auto sep = relativePath.Find(Network::FUri::PathSeparator);
        const FStringView input = relativePath.CutBefore(sep);

        Assert(not part.empty());
        if (Unlikely(part.StartsWith('{'))) {
            AssertMessage("unbalanced path parameter", part.EndsWith('}'));

            FStringView prmName = part.ShiftFront().ShiftBack();
            if (prmName.EndsWith('*')) {
                AssertMessage("globbing path parameter must be at the end of the path", &operation.Path.back() == &part);

                FString decoded;
                if (Network::FUri::Decode(decoded, relativePath)) {
                    pathArgs.Emplace_AssertUnique(prmName.ShiftBack(), std::move(decoded));
                    break;
                }

                return false;
            }

            FString decoded;
            if (Network::FUri::Decode(decoded, input))
                pathArgs.Emplace_AssertUnique(prmName, std::move(decoded));
            else
                return false;
        }
        else if (not EqualsI(input, part)) {
            return false;
        }

        relativePath = relativePath.CutStartingAt(sep);
    }

    FArguments parsedArgs;
    parsedArgs.reserve(operation.Parameters.size());

    forrange(i, 0, operation.Parameters.size()) {
        const FParameter& prm = operation.Parameters[i];

        bool present = false;
        FStringView parsed;

        switch (prm.In) {
        case Body: {
            parsed = ctx.pResponse->Body().MakeView().Cast<const char>();
            present = (not parsed.empty());
            break;
        }
        case Cookie: {
            const auto it = ctx.Cookie.FindLike(prm.Name.MakeView());
            if (ctx.Cookie.end() != it) {
                parsed = it->second.MakeView();
                present = true;
            }
            break;
        }
        case FormData: {
            const auto it = ctx.Post.FindLike(prm.Name.MakeView());
            if (ctx.Post.end() != it) {
                parsed = it->second.MakeView();
                present = true;
            }
            break;
        }
        case Headers: {
            const Network::FLazyName header{ FString("X-") + prm.Name };
            const auto it = ctx.Request.Headers().FindLike(header);
            if (ctx.Request.Headers().end() != it) {
                parsed = it->second.MakeView();
                present = true;
            }
            break;
        }
        case Path: {
            if (const FString* src = pathArgs.GetIFP(prm.Name.MakeView())) {
                parsed = src->MakeView();
                present = true;
            }
            break;
        }
        case Query: {
            if (const FString* src = ctx.Query.GetIFP(prm.Name)) {
                parsed = src->MakeView();
                present = true;
            }
            break;
        }}

        if (Likely(present)) {
            parsedArgs.Insert_AssertUnique(prm, parsed);
            continue;
        }

        if (prm.Required) {
            PPE_LOG(Remoting, Error, "operation <{1}>: missing required param '{0}'",
                prm.Name, Fmt::Join(operation.Path.MakeView(), Network::FUri::PathSeparator) );
            ctx.BadRequest("missing argument"_view);
            return false;
        }

        PPE_LOG(Remoting, Warning, "operation <{1}>: missing optional param '{0}'",
            prm.Name, Fmt::Join(operation.Path.MakeView(), Network::FUri::PathSeparator) );
    }

    if (Likely(operation.Process(ctx, *this, operation, parsedArgs)))
        return true;


    PPE_LOG(Remoting, Error, "operation <{0}>: execution failed",
        Fmt::Join(operation.Path.MakeView(), Network::FUri::PathSeparator) );
    return false;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& WriteParameterLocation_(
    TBasicTextWriter<_Char>& oss,
    IRemotingEndpoint::EParameterLocation in ) {
    switch (in) {
    case IRemotingEndpoint::Body: return oss << STRING_LITERAL(_Char, "Body");
    case IRemotingEndpoint::Cookie: return oss << STRING_LITERAL(_Char, "Cookie");
    case IRemotingEndpoint::FormData: return oss << STRING_LITERAL(_Char, "FormData");
    case IRemotingEndpoint::Headers: return oss << STRING_LITERAL(_Char, "Headers");
    case IRemotingEndpoint::Path: return oss << STRING_LITERAL(_Char, "Path");
    case IRemotingEndpoint::Query: return oss << STRING_LITERAL(_Char, "Query");
    }
    AssertNotImplemented();
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, IRemotingEndpoint::EParameterLocation in) {
    return WriteParameterLocation_(oss, in);
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, IRemotingEndpoint::EParameterLocation in) {
    return WriteParameterLocation_(oss, in);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Remoting
} //!namespace PPE
