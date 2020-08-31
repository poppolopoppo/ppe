#pragma once

#include "Remoting_fwd.h"

#include "RTTI_fwd.h"

#include "RemotingEndpoint.h"

#include "Container/StringHashMap.h"

namespace PPE {
namespace Remoting {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_REMOTING_API FRTTIEndpoint final : public IRemotingEndpoint {
public:
    FRTTIEndpoint() NOEXCEPT;
    virtual ~FRTTIEndpoint() NOEXCEPT;

protected:
    virtual void ProcessImpl(const FRemotingContext& ctx, const FStringView& relativePath) override;

private:
    RTTI::PMetaTransaction _transaction;

    using FDispatchFunc = void(*)(const FRemotingContext& ctx, const FStringView& args);
    STRINGVIEW_HASHMAP_MEMOIZE(Remoting, FDispatchFunc, ECase::Insensitive) _dispatch;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Remoting
} //!namespace PPE
