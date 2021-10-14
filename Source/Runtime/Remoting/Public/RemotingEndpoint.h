#pragma once

#include "Remoting_fwd.h"

#include "Container/AssociativeVector.h"
#include "IO/String.h"
#include "IO/TextWriter_fwd.h"
#include "Memory/PtrRef.h"
#include "Misc/Function.h"
#include "Meta/Optional.h"
#include "NetworkName.h"

namespace PPE {
namespace Remoting {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_REMOTING_API IRemotingEndpoint : Meta::FNonCopyable {
public:
    struct FOperation;

    enum EParameterLocation {
        Body,
        Cookie,
        FormData,
        Headers,
        Path,
        Query,
    };

    struct FParameter {
        Network::FName Name;
        void* UserData{ nullptr };
        EParameterLocation In{ Query };
        bool Required{ false };
        bool Structured{ false };
    };

    using FArguments = ASSOCIATIVE_VECTORINSITU(Remoting, TPtrRef<const FParameter>, FStringView, 4);
    using FProcessFunc = TFunction< bool(
        const FRemotingContext& ctx,
        IRemotingEndpoint& endpoint,
        const FOperation& operation,
        const FArguments& args
    ) NOEXCEPT >;

    struct FOperation {
        FProcessFunc Process;
        VECTORINSITU(Remoting, FStringView, 1) Path;
        VECTORINSITU(Remoting, FParameter, 1) Parameters;
        void* UserData{ nullptr };
        Network::FName Prefix;
        Network::EHttpMethod Method;
    };

    IRemotingEndpoint() = default;
    virtual ~IRemotingEndpoint() NOEXCEPT = default;

    virtual const FString& EndpointPrefix() const NOEXCEPT = 0;

    void EndpointOpenAPI(FOpenAPI* api) const;
    void EndpointProcess(const FRemotingContext& ctx);

protected:
    bool EndpointDispatchIFP(
        const FRemotingContext& ctx,
        FStringView relativePath,
        const FOperation& operation ) NOEXCEPT;

    virtual void PrivateEndpointOpenAPI(FOpenAPI& api) const = 0;

    virtual void PrivateEndpointProcess(
        const FRemotingContext& ctx,
        const FStringView& relativePath ) = 0;
};
//----------------------------------------------------------------------------
PPE_REMOTING_API FTextWriter& operator <<(FTextWriter& oss, IRemotingEndpoint::EParameterLocation in);
PPE_REMOTING_API FWTextWriter& operator <<(FWTextWriter& oss, IRemotingEndpoint::EParameterLocation in);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Remoting
} //!namespace PPE
