#pragma once

#include "Remoting_fwd.h"

#include "IO/String.h"
#include "Misc/Function.h"

#if USE_PPE_ASSERT
#    include <atomic>
#endif

#include "Diagnostic/Benchmark.h"

namespace PPE {
namespace Remoting {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_REMOTING_API IRemotingEndpoint : Meta::FNonCopyable {
public:
    explicit IRemotingEndpoint(FString&& path) NOEXCEPT;
    virtual ~IRemotingEndpoint() NOEXCEPT;

    const FString& Path() const { return _path; }

    void Process(const FRemotingContext& ctx);

protected:
    virtual void ProcessImpl(const FRemotingContext& ctx, const FStringView& relativePath) = 0;

    const FString _path;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Remoting
} //!namespace PPE
