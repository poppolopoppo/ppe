#pragma once

#include "Remoting_fwd.h"

#include "Memory/UniquePtr.h"
#include "Modular/Modular_fwd.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class IRemotingService {
public:
	virtual ~IRemotingService() NOEXCEPT = default;

	virtual const Remoting::FRemotingServer& Server() const = 0;

	virtual void RegisterEndpoint(Remoting::PBaseEndpoint&& endpoint) = 0;
	virtual void UnregisterEndpoint(const Remoting::PBaseEndpoint& endpoint) = 0;

public:
	static PPE_REMOTING_API void MakeDefault(
		URemotingService* remoting,
		const FModularDomain& domain );

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
