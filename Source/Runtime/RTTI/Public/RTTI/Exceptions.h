#pragma once

#include "RTTI_fwd.h"

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
class FClassException : public FRTTIException {
public:
    FClassException(const char* what, const FMetaClass* klass = nullptr) : FRTTIException(what), _class(klass) {}

    const FMetaClass* Class() const { return _class; }

private:
    const FMetaClass* _class;
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
