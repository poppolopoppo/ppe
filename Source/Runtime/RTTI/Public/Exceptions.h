#pragma once

#include "RTTI.h"

#include "Diagnostic/Exception.h"

namespace PPE {
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

    const FMetaProperty* Property() const { return _property; }

private:
    const FMetaProperty* _property;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE
