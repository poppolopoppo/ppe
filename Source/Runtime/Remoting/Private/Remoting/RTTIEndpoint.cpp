#include "stdafx.h"

#include "Remoting/RTTIEndpoint.h"

#include "RemotingServer.h"

#include "Network_Http.h"

#include "MetaDatabase.h"
#include "MetaFunction.h"
#include "MetaObject.h"
#include "MetaTransaction.h"
#include "RTTI/Atom.h"
#include "RTTI/Macros.h"
#include "RTTI/Macros-impl.h"
#include "RTTI/Module.h"
#include "RTTI/Module-impl.h"
#include "RTTI/OpaqueData.h"
#include "RTTI/Typedefs.h"

#include "Json/Json.h"

#include "Allocator/Alloca.h"
#include "Container/Vector.h"
#include "IO/Format.h"
#include "IO/String.h"
#include "Json/JsonSerializer.h"
#include "RTTI/AtomHelpers.h"
#include "Thread/ThreadContext.h"

namespace PPE {
namespace Remoting {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
RTTI_MODULE_DECL(, RTTI_Endpoint);
RTTI_MODULE_DEF(, RTTI_Endpoint, Remoting);
//----------------------------------------------------------------------------
class FRemotingObject : public RTTI::FMetaObject {
    RTTI_CLASS_HEADER(, FRemotingObject, RTTI::FMetaObject);
public:
    FString Address;
    u32 Port;

    explicit FRemotingObject(FRTTIEndpoint& rtti) NOEXCEPT
    :    _rtti(rtti) {

    }

private:
    FRTTIEndpoint& _rtti;
};
RTTI_CLASS_BEGIN(RTTI_Endpoint, FRemotingObject, Concrete)
RTTI_PROPERTY_PUBLIC_FIELD(Address)
RTTI_PROPERTY_PUBLIC_FIELD(Port)
RTTI_CLASS_END()
//----------------------------------------------------------------------------
template <typename _Query>
static void Dispatch_JsonQuery_(const FRemotingContext& ctx, _Query&& query) {
    Serialize::FJson json;
    Network::EHttpStatus status = Network::EHttpStatus::OK;

    ctx.WaitForSync([&query, &json, &status](const FRemotingServer&) {
        AssertIsMainThread();
        status = query(json);
    });

    ctx.pResponse->SetStatus(status);

    FTextWriter oss(&ctx.pResponse->Body());
    json.ToStream(oss);
}
//----------------------------------------------------------------------------
static void Dispatch_Object_(const FRemotingContext& ctx, const RTTI::FLazyPathName& id) {
    switch (ctx.Request.Method()) {
    case Network::EHttpMethod::Get: break;
    case Network::EHttpMethod::Put: AssertNotImplemented(); // #TODO: call to property set, while reading the body
    default:
        ctx.pResponse->SetReason("Only GET or PUT http query is supported");
        ctx.pResponse->SetStatus(Network::EHttpStatus::BadRequest);
        return;
    }

    Dispatch_JsonQuery_(ctx, [&](Serialize::FJson& json) {
        const RTTI::FMetaDatabaseReadable db;

        if (const RTTI::PMetaObject pObj{ db->ObjectIFP(id) }) {
            Serialize::RTTI_to_Json(pObj, &json);
            return Network::EHttpStatus::OK;
        }
        else {
            ctx.pResponse->SetReason(StringFormat("Object not found '{0}'", id));
            return Network::EHttpStatus::NotFound;
        }
    });
}
//----------------------------------------------------------------------------
static void Dispatch_PropertyGet_(const FRemotingContext& ctx, const RTTI::FLazyPathName& id, const RTTI::FLazyName& propName) {
    switch (ctx.Request.Method()) {
    case Network::EHttpMethod::Get: break;
    case Network::EHttpMethod::Put: AssertNotImplemented(); // #TODO: call to property set, while reading the body
    default:
        ctx.pResponse->SetReason("Only GET or PUT http query is supported");
        ctx.pResponse->SetStatus(Network::EHttpStatus::BadRequest);
        return;
    }

    Dispatch_JsonQuery_(ctx, [&](Serialize::FJson& json) {
        const RTTI::FMetaDatabaseReadable db;

        if (const RTTI::PMetaObject pObj{ db->ObjectIFP(id) }) {
            RTTI::FAtom value;

            if (pObj->RTTI_Property(propName, &value)) {
                Serialize::RTTI_to_Json(value, &json);
                return Network::EHttpStatus::OK;
            }
            else {
                ctx.pResponse->SetReason(StringFormat("Property not found <{0}::{1}>",
                    pObj->RTTI_Class()->Name(), propName ));
                return Network::EHttpStatus::NotFound;
            }

        }
        else {
            ctx.pResponse->SetReason(StringFormat("Object not found '{0}'", id));
            return Network::EHttpStatus::NotFound;
        }
    });
}
//----------------------------------------------------------------------------
static void Dispatch_FunctionCall_(const FRemotingContext& ctx, const RTTI::FLazyPathName& id, const RTTI::FLazyName& funcName) {
    // Parse function arguments asap, in the worker thread (less contention compared to sync)
    Serialize::FJson args;

    switch (ctx.Request.Method()) {
    case Network::EHttpMethod::Get:
    {
        Network::FUriQueryMap get;
        Network::FUri::Unpack(get, ctx.Request.Uri());

        const auto it = get.FindLike(MakeStringView("json"));
        if (get.end() == it) {
            ctx.pResponse->SetReason("Missing <json> parameter");
            ctx.pResponse->SetStatus(Network::EHttpStatus::ExpectationFailed);
            return;
        }

        if (not Serialize::FJson::Load(&args, L"HttpGet", it->second.MakeView())) {
            ctx.pResponse->SetReason("Invalid <json> data");
            ctx.pResponse->SetStatus(Network::EHttpStatus::ExpectationFailed);
            return;
        }
    }
        break;
    case Network::EHttpMethod::Post:
    {
        const FStringView bodyStr{ ctx.Request.Body().MakeView().Cast<const char>() };

        if (not Serialize::FJson::Load(&args, L"HttpPost", bodyStr)) {
            ctx.pResponse->SetReason("Invalid <json> data");
            ctx.pResponse->SetStatus(Network::EHttpStatus::ExpectationFailed);
        }
    }
        break;
    default:
        ctx.pResponse->SetReason("Only GET or POST http query are supported");
        ctx.pResponse->SetStatus(Network::EHttpStatus::BadRequest);
        return;
    }

    Dispatch_JsonQuery_(ctx, [&](Serialize::FJson& json) {
        const RTTI::FMetaDatabaseReadable db;

        const RTTI::PMetaObject pObj{ db->ObjectIFP(id) };
        if (Unlikely(not pObj)) {
            ctx.pResponse->SetReason(StringFormat("Object not found '{0}'", id));
            return Network::EHttpStatus::NotFound;
        }

        const RTTI::FMetaFunction* pFunc;
        if (Unlikely(not pObj->RTTI_Function(funcName, &pFunc))) {
            ctx.pResponse->SetReason(StringFormat("Function not found <{0}::{1}()>",
                pObj->RTTI_Class()->Name(), funcName ));
            return Network::EHttpStatus::NotFound;
        }

        STACKLOCAL_POD_ARRAY(RTTI::FAtom, call, pFunc->Parameters().size());
        RTTI::FAtom* pArg = call.data();

        const Serialize::FJson::FObject& input = args.Root().ToObject();
        for (const RTTI::FMetaParameter& prm : pFunc->Parameters()) {
            const auto it = input.FindLike(prm.Name());

            if (Unlikely(it == input.end())) {
                if (not prm.IsOptional()) {
                    ctx.pResponse->SetReason(StringFormat("Missing mandatory parameter <{0}> for {1}::{2}()",
                        prm.Name(), pObj->RTTI_Class()->Name(), pFunc->Name() ));
                    return Network::EHttpStatus::NotAcceptable;
                }

                AssertNotImplemented(); // #TODO: should fill <call> array with the default value ?
                // but the default value isn't necessarily the optional value :'(
            }
            else {
                *pArg = MakeAtom(it->second);
            }

            pArg++;
        }

        STACKLOCAL_ATOM(result, pFunc->Result());
        if (pFunc->InvokeIFP(*pObj, result, call)) {
            RTTI_to_Json(result, &json);
            return Network::EHttpStatus::OK;
        }
        else {
            ctx.pResponse->SetReason(StringFormat("Call to function {0}::{1}() on <{2}> failed",
                pObj->RTTI_Class()->Name(), pFunc->Name(), pObj->RTTI_PathName() ));
            return Network::EHttpStatus::NotAcceptable;
        }
    });
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FRTTIEndpoint::FRTTIEndpoint() NOEXCEPT
:   IRemotingEndpoint("/RTTI") {
    RTTI_MODULE(RTTI_Endpoint).Start();

    TRefPtr<FRemotingObject> o;
    o = NEW_REF(Remoting, FRemotingObject, *this);
    o->RTTI_Export(RTTI::FName{ "obj" });

    _transaction = NEW_REF(Remoting, RTTI::FMetaTransaction, RTTI::FName{ "Remoting" });
    _transaction->Add(std::move(o));
    _transaction->LoadAndMount();
}
//----------------------------------------------------------------------------
FRTTIEndpoint::~FRTTIEndpoint() NOEXCEPT {
    _transaction->UnmountAndUnload();
    _transaction.reset();

    RTTI_MODULE(RTTI_Endpoint).Shutdown();
}
//----------------------------------------------------------------------------
void FRTTIEndpoint::ProcessImpl(const FRemotingContext& ctx, const FStringView& relativePath) {
    FStringView namespace_(relativePath), identifier;
    if (not SplitR(namespace_, Network::FUri::PathSeparator, identifier) || namespace_.empty()) {
        ctx.pResponse->SetStatus(Network::EHttpStatus::BadRequest);
        ctx.pResponse->SetReason(StringFormat("invalid path name: \"{0}\"", namespace_));
        return;
    }

    ctx.pResponse->HTTP_SetContentType(Network::FMimeTypes::Application_json());

    const auto sep = identifier.FindIf([](char c) {
        return (c == '-' || c == '.');
    });

    void (*dispatch_subpart_f)(const FRemotingContext&, const RTTI::FLazyPathName&, const RTTI::FLazyName&) = nullptr;
    FStringView subpart;
    if (identifier.end() != sep) {
        dispatch_subpart_f = (*sep == '-'
            ? &Dispatch_FunctionCall_
            : &Dispatch_PropertyGet_ );

        subpart = identifier.CutStartingAt(sep + 1);
        identifier = identifier.CutBefore(sep);
    }

    RTTI::FLazyPathName id{
        RTTI::FLazyName(namespace_),
        RTTI::FLazyName(identifier)
    };

    if (dispatch_subpart_f) { // function or property
        dispatch_subpart_f(ctx, id, RTTI::FLazyName{ subpart });
    }
    else { // whole object
        Dispatch_Object_(ctx, id);
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Remoting
} //!namespace PPE
