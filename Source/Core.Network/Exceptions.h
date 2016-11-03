#pragma once

#include "Core.Network/Network.h"

#include "Core/Diagnostic/Exception.h"

namespace Core {
namespace Network {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FNetworkException : public FException {
public:
    explicit FNetworkException(const char* what) : FException(what) {}
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
