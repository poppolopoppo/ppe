#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core/Diagnostic/Exception.h"

namespace Core {
namespace RTTI {
class FMetaProperty;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FRTTIException : public FException {
public:
    explicit FRTTIException(const char* what) : FException(what) {}
};
//----------------------------------------------------------------------------
class FPropertyException : public FRTTIException {
public:
    FPropertyException(const char* what, const FMetaProperty* prop = nullptr) : FRTTIException(what), _property(prop) {}

    const FMetaProperty* EProperty() const { return _property; }

private:
    const FMetaProperty* _property;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
