#pragma once

#include "Remoting_fwd.h"

#include "RemotingEndpoint.h"
#include "RemotingModule.h" // RTTI module declaration

#include "RTTI_fwd.h"
#include "RTTI/Macros.h"
#include "RTTI/Typedefs.h"

#include "Container/HashMap.h"
#include "Container/Tuple.h"
#include "Memory/RefPtr.h"
#include "Misc/Function.h"
#include "NetworkName.h"
#include "Http/Method.h"

namespace PPE {
namespace Remoting {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Annotations used by automatic RTTI function binding
//----------------------------------------------------------------------------
struct PPE_REMOTING_API FOperationFacet : RTTI::FGenericUserFacet {
    VECTORINSITU(Remoting, FStringView, 3) Path;
    Network::FName Prefix;
    Network::EHttpMethod Method;

    FOperationFacet(
        Network::EHttpMethod method,
        const FStringView& prefix,
        std::initializer_list<FStringView> path )
    :   Path(path)
    ,   Prefix(prefix)
    ,   Method(method) {
        Assert(not Prefix.empty());
    }

    static FOperationFacet Get(FStringView prefix, std::initializer_list<FStringView> path) {
        return FOperationFacet{ Network::EHttpMethod::Get, prefix, path };
    }
    static FOperationFacet Post(FStringView prefix, std::initializer_list<FStringView> path) {
        return FOperationFacet{ Network::EHttpMethod::Post, prefix, path };
    }
    static FOperationFacet Put(FStringView prefix, std::initializer_list<FStringView> path) {
        return FOperationFacet{ Network::EHttpMethod::Put, prefix, path };
    }
    static FOperationFacet Delete(FStringView prefix, std::initializer_list<FStringView> path) {
        return FOperationFacet{ Network::EHttpMethod::Delete, prefix, path };
    }
};
//----------------------------------------------------------------------------
struct PPE_REMOTING_API FParameterLocationFacet : RTTI::FGenericUserFacet  {
    IRemotingEndpoint::EParameterLocation In;

    FParameterLocationFacet(IRemotingEndpoint::EParameterLocation in) NOEXCEPT
    :   In(in)
    {}
};
//----------------------------------------------------------------------------
struct PPE_REMOTING_API FParameterSchemaFacet : RTTI::FGenericUserFacet  {
    FStringView Schema;

    FParameterSchemaFacet(FStringView schema) NOEXCEPT
    :   Schema(schema)
    {}
};
//----------------------------------------------------------------------------
// Controls automatizing in FBasePoint
//----------------------------------------------------------------------------
enum class EEndpointFlags : u8 {
    Unknown = 0,

    AutomaticBinding    = 1<<0, // will bind all functions with a FOperationFunctionFacet
    AutomaticRegister   = 1<<1, // will register/unregister the endpoint in the server

    Automated = (AutomaticBinding|AutomaticRegister)
};
ENUM_FLAGS(EEndpointFlags);
RTTI_ENUM_HEADER(PPE_REMOTING_API, EEndpointFlags);
//----------------------------------------------------------------------------
// Provide automatic binding for RTTI functions and query dispatching
//----------------------------------------------------------------------------
class PPE_REMOTING_API FBaseEndpoint
:   public RTTI::FMetaObject
,   public IRemotingEndpoint {
    RTTI_CLASS_HEADER(PPE_REMOTING_API, FBaseEndpoint, RTTI::FMetaObject);
public:
    using FOperationKey = TTuple<Network::FName, Network::EHttpMethod>;
    using FOperationMap = HASHMAP(Remoting, FOperationKey, FOperation);

    const FOperationMap& Operations() const { return _operations; }

    const FString& EndpointPrefix() const NOEXCEPT override { return _endpointPrefix; }

    EEndpointFlags EndpointFlags() const { return _endpointFlags; }
    void SetEndpointFlags(EEndpointFlags value) { _endpointFlags = value; }

    void RTTI_EndpointAutomaticBinding();

    virtual void RTTI_Load(RTTI::ILoadContext& context) override;
    virtual void RTTI_Unload(RTTI::IUnloadContext& context) override;

#ifdef WITH_RTTI_VERIFY_PREDICATES
    virtual void RTTI_VerifyPredicates() const PPE_THROW() override;
#endif

protected:
    explicit FBaseEndpoint(RTTI::FConstructorTag) NOEXCEPT;
    explicit FBaseEndpoint(FString&& prefix) NOEXCEPT;

    void ExportAPI(
        FOpenAPI& api,
        const FStringView& root,
        const FOperationKey& key,
        const FOperation& operation,
        std::initializer_list<FStringView> tags ) const;

    void RegisterOperation(
        Network::EHttpMethod method,
        const Network::FName& prefix,
        const TMemoryView<const FStringView>& path,
        const RTTI::FMetaFunction& func );

    bool DispatchOperations(const FRemotingContext& ctx, FStringView relativePath);

    virtual void PrivateEndpointOpenAPI(FOpenAPI& api) const override;
    virtual void PrivateEndpointProcess(const FRemotingContext& ctx, const FStringView& relativePath) override;

private:
    FOperationMap _operations;
    FString _endpointPrefix;
    EEndpointFlags _endpointFlags{ Default };
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Remoting
} //!namespace PPE
