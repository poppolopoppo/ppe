#pragma once

#include "Core.Serialize/Serialize.h"

#include "Core/Diagnostic/Exception.h"

namespace Core {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class SerializeException : public Exception {
public:
    SerializeException(const char* what) : Exception(what) {}
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace Core
