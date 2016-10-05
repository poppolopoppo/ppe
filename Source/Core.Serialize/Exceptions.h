#pragma once

#include "Core.Serialize/Serialize.h"

#include "Core/Diagnostic/Exception.h"

namespace Core {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FSerializeException : public FException {
public:
    FSerializeException(const char* what) : FException(what) {}
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace Core
