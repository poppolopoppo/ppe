// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Remoting/BaseEndpoint.h"

#include "RemotingModule.h"
#include "RemotingServer.h" // LOG_CATEGORY(Remoting)
#include "RemotingService.h"
#include "Remoting/OpenAPI.h"

#include "Network_Http.h"
#include "Uri.h"

#include "Json/Json.h"
#include "Json/JsonSerializer.h"
#include "TransactionLinker.h"

#include "RTTI/UserFacetHelpers.h"
#include "RTTI/Macros-impl.h"

#include "Diagnostic/Exception.h"
#include "Diagnostic/Logger.h"
#include "IO/Format.h"
#include "IO/FormatHelpers.h"
#include "IO/StringBuilder.h"
#include "Misc/Function.h"
#include "Thread/ThreadContext.h"

#if USE_PPE_EXCEPTION_CALLSTACK
#include "Diagnostic/DecodedCallstack.h"
#endif

namespace PPE {
namespace Remoting {
EXTERN_LOG_CATEGORY(PPE_REMOTING_API, Remoting)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
constexpr bool MinifyJsonForDebug_ = (!USE_PPE_DEBUG);
//----------------------------------------------------------------------------
struct FMetaEndpointCall_ {
    const FRemotingContext& Context;
    FBaseEndpoint& Endpoint;
    const IRemotingEndpoint::FOperation& Operation;
    const IRemotingEndpoint::FArguments& Args;

    using FInputFunc = TFunction<bool(RTTI::FAtom* dst, Serialize::FTransactionLinker& linker)>;

    Serialize::FJson::FAllocator Alloc;
    Serialize::FJson Response{ Alloc };
    VECTORINSITU(Remoting, FInputFunc, 4) Inputs;
    VECTORINSITU(Remoting, RTTI::FAny, 4) Values;

    FMetaEndpointCall_(
        const FRemotingContext& context,
        FBaseEndpoint& endpoint,
        const IRemotingEndpoint::FOperation& operation,
        const IRemotingEndpoint::FArguments& args) NOEXCEPT
        :   Context(context)
        ,   Endpoint(endpoint)
        ,   Operation(operation)
        ,   Args(args)
    {}

    ~FMetaEndpointCall_() {
        Response.Clear_ForgetMemory();
    }

    const RTTI::FMetaFunction& Function() const {
        return *static_cast<const RTTI::FMetaFunction*>(Operation.UserData);
    }

    void OnException(const FException& e) const NOEXCEPT {
        FTextWriter outp{ &Context.pResponse->Body() };
        outp << Fmt::Repeat("-!"_view, 40);
        Format(outp, "caught exception during invocation of operation {0}#{1}/{2}: {3}",
            Operation.Method,
            Operation.Prefix,
            Fmt::Join(Operation.Path.MakeView(), Network::FUri::PathSeparator),
            MakeCStringView(e.What()) );
#if USE_PPE_EXCEPTION_DESCRIPTION
        outp << Fmt::Repeat("-="_view, 40);
        FWStringBuilder desc;
        e.Description(desc);
        outp << desc.ToString();
#endif
#if USE_PPE_EXCEPTION_CALLSTACK
        outp << Fmt::Repeat("_."_view, 40);
        outp << Eol << e.Callstack();
#endif

        Context.Failed(Network::EHttpStatus::InternalServerError, ToString(e.What()));
    }

    void SyncedCall(const FRemotingServer&) NOEXCEPT {
        AssertIsMainThread();

        const RTTI::FMetaFunction& func = Function();

        RTTI::FAtom result;
        if (func.HasReturnValue())
            result = Values.back();

        Serialize::FTransactionLinker linker(ForceInit);
        STACKLOCAL_POD_ARRAY(RTTI::FAtom, parsedAtoms, Inputs.size());

        PPE_TRY {
            Collect(parsedAtoms, [this, &linker](size_t i, RTTI::FAtom* dst) {
                *dst = Values[i].InnerAtom();

                if (Inputs[i] && not Inputs[i](dst, linker))
                    PPE_THROW_IT(Serialize::FJsonException{ "failed to parse argument" });
            });

            func.Invoke(Endpoint, result, parsedAtoms);

            if (func.HasReturnValue()) {
                Assert(result);
                RTTI_to_Json(result, &Response);
            }
        }
        PPE_CATCH(const FException& e)
        PPE_CATCH_BLOCK({
            Response.Clear_ReleaseMemory();
            OnException(e);
        })
    }

    bool Process() NOEXCEPT {
        Assert(Operation.Parameters.size() == Function().Parameters().size());

        const RTTI::FMetaFunction& func = Function();

        Inputs.reserve(func.Parameters().size());
        Values.reserve(func.Parameters().size() +
            (func.HasReturnValue() ? 1 : 0));

        const auto jsonParser = [this](Serialize::FJson* dst, const FStringView& src, const IRemotingEndpoint::FParameter& prm) NOEXCEPT -> bool {
            Unused(prm);
            PPE_TRY {
                if (Serialize::FJson::Append(dst, L"remoting:/arguments", src))
                    return true;

                LOG(Remoting, Error, L"failed to parse Json argument for '{0}':\n{1}",
                    prm.Name, Fmt::HexDump(src));
                return false;
            }
            PPE_CATCH(const Serialize::FJsonException & e)
            PPE_CATCH_BLOCK({
                OnException(e);
                return false;
            })
        };

        for (const IRemotingEndpoint::FParameter& prm : Operation.Parameters) {
            // construct actual data for parsed parameter
            Assert(prm.UserData);
            const auto& meta = *static_cast<const RTTI::FMetaParameter*>(prm.UserData);
            Values.emplace_back_AssumeNoGrow(meta.Traits());

            // handle missing param, will fail if required
            const auto it = Args.find(prm);
            if (Args.end() == it) {
                if (not prm.Required) {
                    Inputs.emplace_back_AssumeNoGrow(NoFunction);
                    continue;
                }
                return false;
            }

            // handle structured (Json) or non-structured (scanf) parsing
            if (prm.Structured) {
                Serialize::FJson json{ Alloc };

                if (prm.In == IRemotingEndpoint::Query && meta.Traits()->AsList()) {
                    FStringView slice;
                    FStringView arr{ it->second };
                    while (Split(arr, '|', slice)) {
                        if (not jsonParser(&json, slice, prm))
                            return false;
                    }
                }
                else if (not jsonParser(&json, it->second, prm)) {
                    return false;
                }

                Inputs.emplace_back_AssumeNoGrow(
                    [json{ std::move(json) }](RTTI::FAtom* dst, Serialize::FTransactionLinker& linker) -> bool {
                        return Json_to_RTTI(*dst, json, &linker);
                    });
            }
            else {
                RTTI::FAtom dst = Values.back().InnerAtom();
                if (not dst.FromString(it->second))
                    return false;

                Inputs.emplace_back_AssumeNoGrow(NoFunction);
            }
        }

        if (func.HasReturnValue())
            Values.emplace_back_AssumeNoGrow(func.Result());

        Context.WaitForSync(FRemotingCallback::Bind<&FMetaEndpointCall_::SyncedCall>(this));

        if (Likely(not Context.pResponse->Failed())) {
            Context.pResponse->SetStatus(Network::EHttpStatus::OK);

            if (func.HasReturnValue()) {
                Assert(Response.Root().Valid());

                FTextWriter outp{ &Context.pResponse->Body() };
                Response.ToStream(outp, MinifyJsonForDebug_);

                Context.pResponse->HTTP_SetContentType(Network::FMimeTypes::Application_json());
                return true;
            }
        }

        Assert_NoAssume(Context.pResponse->Failed()/* should already been set */);
        return false;
    }

    static bool EntryPoint(
        const FRemotingContext& context,
        IRemotingEndpoint& endpoint,
        const IRemotingEndpoint::FOperation& operation,
        const IRemotingEndpoint::FArguments& args ) NOEXCEPT {
        return FMetaEndpointCall_{
            context,
            *checked_cast<FBaseEndpoint*>(&endpoint),
            operation,
            args
        }.Process();
    }

};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
RTTI_ENUM_FLAGS_BEGIN(Remoting, EEndpointFlags)
    RTTI_ENUM_VALUE(Unknown)
    RTTI_ENUM_VALUE(AutomaticBinding)
    RTTI_ENUM_VALUE(AutomaticRegister)
RTTI_ENUM_END()
//----------------------------------------------------------------------------
RTTI_CLASS_BEGIN(Remoting, FBaseEndpoint, Abstract)
    RTTI_PROPERTY_PRIVATE_FIELD(_endpointFlags)
    RTTI_PROPERTY_PRIVATE_FIELD(_endpointPrefix)
RTTI_CLASS_END()
//----------------------------------------------------------------------------
FBaseEndpoint::FBaseEndpoint(RTTI::FConstructorTag) NOEXCEPT
:   IRemotingEndpoint()
,   _endpointFlags(EEndpointFlags::Automated)
{}
//----------------------------------------------------------------------------
FBaseEndpoint::FBaseEndpoint(FString&& prefix) NOEXCEPT
:   IRemotingEndpoint()
,   _endpointPrefix(std::move(prefix)) {
    Assert_NoAssume(not _endpointPrefix.empty());
    Assert_NoAssume(_endpointPrefix.MakeView().StartsWith(Network::FUri::PathSeparator));
}
//----------------------------------------------------------------------------
void FBaseEndpoint::RTTI_Load(RTTI::ILoadContext& context) {
    RTTI_parent_type::RTTI_Load(context);

    if (_endpointFlags & EEndpointFlags::AutomaticBinding) {
        RTTI_EndpointAutomaticBinding();
    }

    if (_endpointFlags & EEndpointFlags::AutomaticRegister) {
        if (IRemotingService* const remoting = FModularDomain::Get().Services().GetIFP<IRemotingService>())
            remoting->RegisterEndpoint(PBaseEndpoint{this});
    }
}
//----------------------------------------------------------------------------
void FBaseEndpoint::RTTI_Unload(RTTI::IUnloadContext& context) {

    if (_endpointFlags & EEndpointFlags::AutomaticRegister) {
        if (IRemotingService* const remoting = FModularDomain::Get().Services().GetIFP<IRemotingService>())
            remoting->UnregisterEndpoint(PBaseEndpoint{this});
    }

    RTTI_parent_type::RTTI_Unload(context);
}
//----------------------------------------------------------------------------
#ifdef WITH_RTTI_VERIFY_PREDICATES
void FBaseEndpoint::RTTI_VerifyPredicates() const PPE_THROW() {
    RTTI_parent_type::RTTI_VerifyPredicates();
    RTTI_VerifyPredicate(not _endpointPrefix.empty());
    RTTI_VerifyPredicate(_endpointPrefix.MakeView().StartsWith(Network::FUri::PathSeparator));
}
#endif
//----------------------------------------------------------------------------
void FBaseEndpoint::ExportAPI(
    FOpenAPI& api,
    const FStringView& root,
    const FOperationKey& key,
    const FOperation& operation,
    std::initializer_list<FStringView> tags) const {
    const auto& func = (*static_cast<RTTI::FMetaFunction*>(operation.UserData));

    FStringBuilder path;
    path << Fmt::Join({ root, std::get<0>(key).MakeView() }, Network::FUri::PathSeparator);
    if (not operation.Path.empty())
        path << Network::FUri::PathSeparator << Fmt::Join(operation.Path.MakeView(), Network::FUri::PathSeparator);

    FString httpMethod = ToString(operation.Method);
    httpMethod.to_lower();

    api.Operation(
        func.Name(),
        path.Written(),
        httpMethod,
        Default,
        RTTI::FDescriptionFacet::GetIFP(func),
        tags,
        [&](FOpenAPI::FOperation* span) {
            for (const FParameter& prm : operation.Parameters) {
                const auto& prop = (*static_cast<const RTTI::FMetaProperty*>(prm.UserData));

                FOpenAPI::FSchema schema;
                if (const FParameterSchemaFacet* const overrideFacet = prop.Facets().GetIFP<FParameterSchemaFacet>())
                    schema = api.Ref(Serialize::FJson::LiteralText(overrideFacet->Schema));
                else
                    api.Traits(&schema, prop.Traits());

                switch (prm.In) {
                case Body: {
                    api.Operation_Body(span,
                        Network::FMimeTypes::Application_json().MakeView(),
                        prm.Name.MakeView(),
                        std::move(schema),
                        prm.Required);
                    break;
                }
                case FormData: {
                    api.Operation_Body(span,
                        Network::FMimeTypes::Application_x_www_form_urlencoded().MakeView(),
                        prm.Name.MakeView(),
                        std::move(schema),
                        prm.Required);
                    break;
                }
                case Cookie:
                case Headers:
                case Path:
                case Query: {
                    FString location = ToString(prm.In);
                    location.to_lower();
                    api.Operation_Parameter(span,
                        std::move(location),
                        prm.Name,
                        RTTI::FDescriptionFacet::GetIFP(prop),
                        std::move(schema),
                        prm.Required);
                    break;
                }}
            }

            if (func.HasReturnValue()) {
                FOpenAPI::FSchema schema;
                api.Traits(&schema, func.Result());

                api.Operation_Response(span,
                    "200", // Http OK
                    Network::FMimeTypes::Application_json().MakeView(),
                    "success",
                    std::move(schema) );
            }
            else {
                api.Operation_Response(span,
                    "200", // Http OK
                    "success" );
            }

            if (not func.IsNoExcept())
                api.Operation_Response(span, "500", "text/plain", "exception thrown",
                    api.Scalar("error message", "string", Default));
        });
}
//----------------------------------------------------------------------------
void FBaseEndpoint::RegisterOperation(
    Network::EHttpMethod method,
    const Network::FName& prefix,
    const TMemoryView<const FStringView>& path,
    const RTTI::FMetaFunction& func) {
    Assert(not MakeIterable(path).Any([](const FStringView& x) { return x.empty(); }));

    FOperation op;
    op.Method = method;
    op.Prefix = prefix;
    op.Path.assign(path);
    op.Process = FProcessFunc::Bind<&FMetaEndpointCall_::EntryPoint>();
    op.Parameters.reserve(func.Parameters().size());
    op.UserData = const_cast<void*>(static_cast<const void*>(std::addressof(func)));

    for (const RTTI::FMetaParameter& prm : func.Parameters()) {
        const RTTI::FTypeInfos typeInfos{ prm.Traits()->TypeInfos() };

        FString name{ prm.Name().MakeView() };
        name.to_lower(); // force lower case since RTTI::FName is case insensitive, but not OpenAPI

        FParameter wrapper;
        wrapper.Name = Network::FName{ name };
        wrapper.UserData = const_cast<void*>(static_cast<const void*>(std::addressof(prm)));
        wrapper.Required = (prm.IsOptional() ? 1 : 0);
        wrapper.Structured = typeInfos.IsStructured();

        wrapper.In = Query; // look in query by default
        if (const FParameterLocationFacet* pFacet = RTTI::UserFacetIFP<FParameterLocationFacet>(prm))
            wrapper.In = pFacet->In;
        else if (path.Any([&wrapper](FStringView p) {
            if (p.StartsWith('{') && p.EndsWith('}')) {
                p = p.ShiftFront().ShiftBack();
                if (p.EndsWith('*'))
                    p = p.ShiftBack();
                return wrapper.Name.Equals(p);
            }
            return false;
        })) {
            wrapper.In = EParameterLocation::Path;
            wrapper.Required = true; // path parameters are always required
        }

        AssertReleaseMessage(L"body parameter are not supported for GET/HEAD request",
            Body != wrapper.In || (
                Network::EHttpMethod::Get != method &&
                Network::EHttpMethod::Head != method ));

        op.Parameters.emplace_back_AssumeNoGrow(std::move(wrapper));
    }

    _operations.insert_AssertUnique({
        MakeTuple(prefix, method),
        std::move(op)
    });
}
//----------------------------------------------------------------------------
void FBaseEndpoint::RTTI_EndpointAutomaticBinding() {
    const RTTI::FMetaClass* const class_ = RTTI_Class();
    Assert(class_);
    for (const auto it : class_->FunctionsByTag<FOperationFacet>()) {
        RegisterOperation(
            it.second->Method,
            it.second->Prefix,
            it.second->Path.MakeView(),
            *it.first);
    }
}
//----------------------------------------------------------------------------
bool FBaseEndpoint::DispatchOperations(const FRemotingContext& ctx, FStringView relativePath) {
    Assert(not relativePath.empty());

    auto dispatch = [this, &ctx, relativePath](FStringView::iterator sep) NOEXCEPT -> bool {
        FStringView path{ relativePath.begin(), sep };
        while (not path.empty() && path.back() == Network::FUri::PathSeparator)
            path = path.ShiftBack();

        if (path.empty() || not Network::FLazyName::IsValidToken(path))
            return false;

        const auto key = MakeTuple(
            Network::FLazyName{ path },
            ctx.Request.Method()
        );

        const auto op = _operations.find_like(key, hash_value(key));
        if (Likely(_operations.end() != op)) {
            FStringView subPath = relativePath.CutStartingAt(sep);
            while (subPath.StartsWith(Network::FUri::PathSeparator))
                subPath = subPath.ShiftFront();

            return EndpointDispatchIFP(ctx, subPath, op->second);
        }

        return false;
    };

    auto it = relativePath.rbegin();
    while (relativePath.rend() != it) {
        if (dispatch(it.base()))
            return true;

        it = relativePath.FindAfterR(Network::FUri::PathSeparator, it);
    }

    return dispatch(relativePath.end());
}
//----------------------------------------------------------------------------
void FBaseEndpoint::PrivateEndpointOpenAPI(FOpenAPI& api) const {
    const RTTI::FMetaClass& class_ = *RTTI_Class();

    api.Tag(class_.Name().MakeView(),
            RTTI::FDescriptionFacet::GetIFP(class_),
            NoFunction);

    for (const auto& it : _operations) {
        ExportAPI(api, _endpointPrefix, it.first, it.second, {
            class_.Name().MakeView()
        });
    }
}
//----------------------------------------------------------------------------
void FBaseEndpoint::PrivateEndpointProcess(const FRemotingContext& ctx, const FStringView& relativePath) {
    DispatchOperations(ctx, relativePath);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Remoting
} //!namespace PPE
