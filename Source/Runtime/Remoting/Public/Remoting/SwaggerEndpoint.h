#pragma once

#include "Remoting_fwd.h"

#include "RemotingEndpoint.h"

#include "IO/String.h"

namespace PPE {
namespace Remoting {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_REMOTING_API FSwaggerEndpoint final : public IRemotingEndpoint {
public:
    FSwaggerEndpoint() NOEXCEPT;
    virtual ~FSwaggerEndpoint();

    virtual const FString& EndpointPrefix() const NOEXCEPT override { return _prefix; }

    FString SwaggerApi(const FRemotingServer& server) const;

protected:
    virtual void PrivateEndpointOpenAPI(FOpenAPI& api) const override;
    virtual void PrivateEndpointProcess(const FRemotingContext& ctx, const FStringView& relativePath) override;

private:
    const FString _prefix;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Remoting
} //!namespace PPE
