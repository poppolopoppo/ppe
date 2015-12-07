#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core/Diagnostic/Exception.h"

namespace Core {
namespace RTTI {
class MetaProperty;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class RTTIException : public Exception {
public:
    RTTIException(const char* what) : Exception(what) {}
};
//----------------------------------------------------------------------------
class PropertyException : public RTTIException {
public:
    PropertyException(const char* what, const MetaProperty* prop = nullptr) : RTTIException(what), _property(prop) {}

    const MetaProperty* Property() const { return _property; }

private:
    const MetaProperty* _property;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
