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
#include "Container/MinMaxHeap.h"
#include "Container/PerfectHashing.h"
#include "Container/Vector.h"
#include "HAL/PlatformMemory.h"
#include "IO/Format.h"
#include "IO/String.h"
#include "IO/StringBuilder.h"
#include "Json/JsonSerializer.h"
#include "RTTI/AtomHelpers.h"
#include "Thread/ThreadContext.h"

namespace PPE {
RTTI_STRUCT_DECL(, FPlatformMemory::FConstants);
RTTI_STRUCT_DEF(, FPlatformMemory::FConstants);
RTTI_STRUCT_DECL(, FPlatformMemory::FStats);
RTTI_STRUCT_DEF(, FPlatformMemory::FStats);
namespace Remoting {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
RTTI_MODULE_DECL(, Endpoint);
RTTI_MODULE_DEF(, Endpoint, Remoting);
//----------------------------------------------------------------------------
class FRemotingObject : public RTTI::FMetaObject {
    RTTI_CLASS_HEADER(, FRemotingObject, RTTI::FMetaObject);
public:
    explicit FRemotingObject(FRTTIEndpoint& rtti) NOEXCEPT
    :    _rtti(rtti) {
        UNUSED(_rtti);
    }

    FPlatformMemory::FConstants MemoryConstants() const { return FPlatformMemory::Constants(); }
    FPlatformMemory::FStats MemoryStats() const { return FPlatformMemory::Stats(); }

private:
    FRTTIEndpoint& _rtti;
};
RTTI_CLASS_BEGIN(Endpoint, FRemotingObject, Concrete)
RTTI_FUNCTION(MemoryConstants)
RTTI_FUNCTION(MemoryStats)
RTTI_CLASS_END()
//----------------------------------------------------------------------------
template <typename _Query>
static void Dispatch_JsonQuery_(const FRemotingContext& ctx, _Query&& query) {
    Serialize::FJson json;

    ctx.WaitForSync([&query, &json](const FRemotingServer&) {
        AssertIsMainThread();
        query(json);
    });

    if (json.Root().Valid()) {
        FTextWriter oss(&ctx.pResponse->Body());
        json.ToStream(oss, !USE_PPE_DEBUG);
    }
}
//----------------------------------------------------------------------------
static void Dispatch_Namespace_(const FRemotingContext& ctx, const RTTI::FLazyName& id) {
    switch (ctx.Request.Method()) {
    case Network::EHttpMethod::Get: break;
    default:
        ctx.BadRequest("Only GET http query is supported");
        return;
    }

    Dispatch_JsonQuery_(ctx, [&](Serialize::FJson& json) {
        const RTTI::FMetaDatabaseReadable db;
        RTTI_to_Json(db->TransactionIFP(id), &json);
    });
}
//----------------------------------------------------------------------------
static void Dispatch_Object_(const FRemotingContext& ctx, const RTTI::FLazyPathName& id) {
    switch (ctx.Request.Method()) {
    case Network::EHttpMethod::Get: break;
    case Network::EHttpMethod::Put: AssertNotImplemented(); // #TODO: call to property set, while reading the body
    default:
        ctx.BadRequest("Only GET or PUT http query is supported");
        return;
    }

    Dispatch_JsonQuery_(ctx, [&](Serialize::FJson& json) {
        const RTTI::FMetaDatabaseReadable db;

        if (const RTTI::PMetaObject pObj{ db->ObjectIFP(id) })
            RTTI_to_Json(pObj, &json);
        else
            ctx.NotFound(ToString(id));
    });
}
//----------------------------------------------------------------------------
static void Dispatch_PropertyGet_(const FRemotingContext& ctx, const RTTI::FLazyPathName& id, const RTTI::FLazyName& propName) {
    switch (ctx.Request.Method()) {
    case Network::EHttpMethod::Get: break;
    case Network::EHttpMethod::Put: AssertNotImplemented(); // #TODO: call to property set, while reading the body
    default:
        ctx.BadRequest("Only GET or PUT http query is supported");
        return;
    }

    Dispatch_JsonQuery_(ctx, [&](Serialize::FJson& json) {
        const RTTI::FMetaDatabaseReadable db;

        if (const RTTI::PMetaObject pObj{ db->ObjectIFP(id) }) {
            RTTI::FAtom value;

            if (pObj->RTTI_Property(propName, &value))
                RTTI_to_Json(value, &json);
            else
                ctx.NotFound(StringFormat("{0}::{1}",
                    pObj->RTTI_Class()->Name(), propName ));
        }
        else
            ctx.NotFound(ToString(id));
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
        if (get.end() != it) {
            if (not Serialize::FJson::Load(&args, L"HttpGet", it->second.MakeView())) {
                ctx.ExpectationFailed("invalid <json> data");
                return;
            }
        }
        else {
            args.Root().MakeDefault_AssumeNotValid<Serialize::FJson::FObject>();
        }
    }
        break;
    case Network::EHttpMethod::Post:
    {
        const FStringView bodyStr{ ctx.Request.Body().MakeView().Cast<const char>() };

        if (not Serialize::FJson::Load(&args, L"HttpPost", bodyStr)) {
            ctx.ExpectationFailed("invalid <json> data");
        }
    }
        break;
    default:
        ctx.BadRequest("Only GET or POST http query are supported");
        return;
    }

    Dispatch_JsonQuery_(ctx, [&](Serialize::FJson& json) {
        const RTTI::FMetaDatabaseReadable db;

        const RTTI::PMetaObject pObj{ db->ObjectIFP(id) };
        if (Unlikely(not pObj)) {
            ctx.NotFound(ToString(id));
            return;
        }

        const RTTI::FMetaFunction* pFunc;
        if (Unlikely(not pObj->RTTI_Function(funcName, &pFunc))) {
            ctx.NotFound(StringFormat("{0}::{1}()",
                pObj->RTTI_Class()->Name(), funcName ));
            return;
        }

        STACKLOCAL_POD_ARRAY(RTTI::FAtom, call, pFunc->Parameters().size());
        RTTI::FAtom* pArg = call.data();

        const Serialize::FJson::FObject& input = args.Root().ToObject();
        for (const RTTI::FMetaParameter& prm : pFunc->Parameters()) {
            const auto it = input.FindLike(prm.Name());

            if (Unlikely(it == input.end())) {
                if (not prm.IsOptional()) {
                    ctx.Failed(Network::EHttpStatus::NotAcceptable,
                        StringFormat("Missing mandatory parameter <{0}> for {1}::{2}()",
                        prm.Name(), pObj->RTTI_Class()->Name(), pFunc->Name() ));
                    return;
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
        if (pFunc->InvokeIFP(*pObj, result, call))
            RTTI_to_Json(result, &json);
        else
            ctx.Failed(
                Network::EHttpStatus::NotAcceptable,
                StringFormat("Call to function {0}::{1}() on <{2}> failed",
                pObj->RTTI_Class()->Name(), pFunc->Name(), pObj->RTTI_PathName() ));
    });
}
//----------------------------------------------------------------------------
static void RTTIEndpoint_Class_(const FRemotingContext& ctx, const FStringView& args) {
    switch (ctx.Request.Method()) {
    case Network::EHttpMethod::Get: break;
    default:
        ctx.BadRequest("only HTTP Get is supported");
        return;
    }

    Dispatch_JsonQuery_(ctx, [&ctx, args](Serialize::FJson& json) {
        const RTTI::FMetaDatabaseReadable db;
        if (const RTTI::FMetaClass* const mclass = db->ClassIFP(args))
            RTTI_to_Json(*mclass, &json);
        else
            ctx.NotFound(args);
    });
}
//----------------------------------------------------------------------------
static void RTTIEndpoint_Enum_(const FRemotingContext& ctx, const FStringView& args) {
    switch (ctx.Request.Method()) {
    case Network::EHttpMethod::Get: break;
    default:
        ctx.BadRequest("only HTTP Get is supported");
        return;
    }

    Dispatch_JsonQuery_(ctx, [&ctx, args](Serialize::FJson& json) {
        const RTTI::FMetaDatabaseReadable db;
        if (const RTTI::FMetaEnum* const menum = db->EnumIFP(args))
            RTTI_to_Json(*menum, &json);
        else
            ctx.NotFound(args);
    });
}
//----------------------------------------------------------------------------
static void RTTIEndpoint_Module_(const FRemotingContext& ctx, const FStringView& args) {
    switch (ctx.Request.Method()) {
    case Network::EHttpMethod::Get: break;
    default:
        ctx.BadRequest("only HTTP Get is supported");
        return;
    }

    Dispatch_JsonQuery_(ctx, [&ctx, args](Serialize::FJson& json) {
        const RTTI::FMetaDatabaseReadable db;
        if (const RTTI::FMetaModule* const mmodule = db->ModuleIFP(args))
            RTTI_to_Json(*mmodule, &json);
        else
            ctx.NotFound(args);
    });
}
//----------------------------------------------------------------------------
static void RTTIEndpoint_Traits_(const FRemotingContext& ctx, const FStringView& args) {
    switch (ctx.Request.Method()) {
    case Network::EHttpMethod::Get: break;
    default:
        ctx.BadRequest("only HTTP Get is supported");
        return;
    }

    Dispatch_JsonQuery_(ctx, [&ctx, args](Serialize::FJson& json) {
        const RTTI::FMetaDatabaseReadable db;
        if (const RTTI::PTypeTraits mtraits = db->TraitsIFP(args))
            RTTI_to_Json(mtraits, &json);
        else
            ctx.NotFound(args);
    });
}
//----------------------------------------------------------------------------
static void RTTIEndpoint_Export_(const FRemotingContext& ctx, const FStringView& args) {
    FStringView path, subpart;
    const auto sep = args.FindR('.');
    if (args.rend() == sep) {
        path = args;
    }
    else {
        path = args.CutBefore(sep);
        subpart = args.CutStartingAt(sep - 1);
    }

    RTTI::FLazyPathName id;
    if (subpart.empty()) {
        if (RTTI::FLazyPathName::Parse(&id, path, false))
            Dispatch_Object_(ctx, id);
        else
            Dispatch_Namespace_(ctx, RTTI::FLazyName{ args });
    }
    else {
        if (not RTTI::FLazyPathName::Parse(&id, path, false)) {
            ctx.ExpectationFailed("malformed RTTI pathname");
            return;
        }

        if (subpart.EndsWith("()"))
            Dispatch_FunctionCall_(ctx, id, RTTI::FLazyName{ subpart.ShiftBack(2) });
        else
            Dispatch_PropertyGet_(ctx, id, RTTI::FLazyName{ subpart });
    }
}
//----------------------------------------------------------------------------
struct FAutoCompleteResult {
    FString Text;
    size_t Distance;

    bool operator <(const FAutoCompleteResult& other) const NOEXCEPT {
        return ((Distance < other.Distance) | (Distance == other.Distance &&
            Text < other.Text ));
    }

    bool operator ==(const FAutoCompleteResult& other) const NOEXCEPT {
        return (Distance == other.Distance && Text == other.Text);
    }
};
//----------------------------------------------------------------------------
template <typename _Source>
static void Dispatch_AutoComplete_(const FRemotingContext& ctx, _Source source) {
    Serialize::FJson json; // don't execute on game-thread for auto-complete (read-only and performance critical)

    STACKLOCAL_HEAP(FAutoCompleteResult, Meta::TLess<FAutoCompleteResult>{}, results, 10);
    source(results);

    results.Sort(); // largest to smallest distance

    Serialize::FJson::FArray& arr = json.Root().MakeDefault_AssumeNotValid<Serialize::FJson::FArray>();
    arr.reserve(results.size());

    reverseforeachitem(it, results) {
        arr.emplace_back(std::move(it->Text));
    }

    if (json.Root().Valid()) {
        FTextWriter oss(&ctx.pResponse->Body());
        json.ToStream(oss, !USE_PPE_DEBUG);
    }
}
//----------------------------------------------------------------------------
static void RTTIEndpoint_Complete_Export_(const FRemotingContext& ctx) {
    FStringView term;
    const RTTI::FMetaClass* mclass = nullptr;

    Network::FUri::FQueryMap prms;
    if (Network::FUri::Unpack(prms, ctx.Request.Uri())) {
        auto it = prms.FindLike(MakeStringView("class"));
        if (it != prms.end()) {
            {
                RTTI::FMetaDatabaseReadable db;
                mclass = db->ClassIFP(it->second);
            }
            if (nullptr == mclass) {
                ctx.ExpectationFailed(StringFormat("invalid parent metaclass: {0}", it->second));
                return;
            }
        }
        it = prms.FindLike(MakeStringView("term"));
        if (it != prms.end()) {
            term = it->second;
        }
    }

    Dispatch_AutoComplete_(ctx, [term, mclass](TStackHeapAdapter<FAutoCompleteResult>& results) {
        FStringBuilder sb;

        RTTI::FMetaDatabaseReadable db;
        for (const RTTI::SMetaObject& obj : db->Objects().Values()) {
            Assert(obj);
            if ((nullptr == mclass) || (obj->RTTI_InheritsFrom(*mclass))) {
                sb << obj->RTTI_PathName().Namespace << '/' << obj->RTTI_PathName().Identifier;
                const size_t dist = LevenshteinDistanceI(sb.Written(), term);
                if (results.empty() || dist < results.Max().Distance)
                    results.Roll(sb.ToString(), dist);
                else
                    sb.clear();
            }
        }
    });
}
//----------------------------------------------------------------------------
static void RTTIEndpoint_Complete_Traits_(const FRemotingContext& ctx) {
    FStringView term;
    RTTI::ETypeFlags typeFlags{ 0 };

    Network::FUri::FQueryMap prms;
    if (Network::FUri::Unpack(prms, ctx.Request.Uri())) {
        auto it = prms.FindLike(MakeStringView("TypeFlags"));
        if (it != prms.end()) {
            if (not MetaEnumParse(it->second, &typeFlags)) {
                ctx.ExpectationFailed(StringFormat("invalid type flags: {0}", it->second));
                return;
            }
        }
        it = prms.FindLike(MakeStringView("term"));
        if (it != prms.end()) {
            term = it->second;
        }
    }

    Dispatch_AutoComplete_(ctx,  [term, typeFlags](TStackHeapAdapter<FAutoCompleteResult>& results) {
        RTTI::FMetaDatabaseReadable db;
        for (const RTTI::PTypeTraits& traits : db->Traits().Values()) {
            if ((typeFlags == RTTI::ETypeFlags{ 0 }) || (typeFlags ^ traits->TypeFlags())) {
                const size_t dist = LevenshteinDistanceI(traits->TypeName(), term);
                if (results.empty() || dist < results.Max().Distance)
                    results.Roll(ToString(traits->TypeName()), dist);
            }
        }
    });
}
//----------------------------------------------------------------------------
static void RTTIEndpoint_Complete_(const FRemotingContext& ctx, const FStringView& args) {
    switch (ctx.Request.Method()) {
    case Network::EHttpMethod::Get: break;
    default:
        ctx.BadRequest("only HTTP Get is supported");
        return;
    }

    if (EqualsI("export", args))
        RTTIEndpoint_Complete_Export_(ctx);
    else if (EqualsI("traits", args))
        RTTIEndpoint_Complete_Traits_(ctx);
    else
        ctx.NotFound(args);
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FRTTIEndpoint::FRTTIEndpoint() NOEXCEPT
:   IRemotingEndpoint("/RTTI") {

    _dispatch.reserve(6);
    _dispatch.emplace_AssertUnique("class", &RTTIEndpoint_Class_);
    _dispatch.emplace_AssertUnique("enum", &RTTIEndpoint_Enum_);
    _dispatch.emplace_AssertUnique("module", &RTTIEndpoint_Module_);
    _dispatch.emplace_AssertUnique("export", &RTTIEndpoint_Export_);
    _dispatch.emplace_AssertUnique("traits", &RTTIEndpoint_Traits_);
    _dispatch.emplace_AssertUnique("complete", &RTTIEndpoint_Complete_);

    RTTI_MODULE(Endpoint).Start();

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

    RTTI_MODULE(Endpoint).Shutdown();
}
//----------------------------------------------------------------------------
void FRTTIEndpoint::ProcessImpl(const FRemotingContext& ctx, const FStringView& relativePath) {
    const auto sep = relativePath.Find(Network::FUri::PathSeparator);
    if (relativePath.end() == sep) {
        ctx.BadRequest(relativePath);
        return;
    }

    const FStringView method = relativePath.CutBefore(sep);
    const FStringView args = relativePath.CutStartingAt(sep + 1);

    const auto it = _dispatch.find(method);
    if (Likely(it != _dispatch.end())) {
        // set JSON mime-type when succeeded
        ctx.pResponse->HTTP_SetContentType(Network::FMimeTypes::Application_json());
        it->second(ctx, args);
    }
    else {
        ctx.NotFound(method);
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Remoting
} //!namespace PPE
