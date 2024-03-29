#pragma once

#include "RTTI_fwd.h"

#include "Diagnostic/Exception.h"

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FRTTIException : public FException {
public:
    explicit FRTTIException(const char* what)
        : FException(what) {}

#if USE_PPE_EXCEPTION_DESCRIPTION
    virtual FTextWriter& Description(FTextWriter& oss) const override {
        return oss;
    }
#endif
};
//----------------------------------------------------------------------------
class FClassException : public FRTTIException {
public:
    FClassException(const char* what, const FMetaClass* klass = nullptr)
        : FRTTIException(what), _class(klass) {}

    const FMetaClass* Class() const { return _class; }

#if USE_PPE_EXCEPTION_DESCRIPTION
    PPE_RTTI_API virtual FTextWriter& Description(FTextWriter& oss) const override;
#endif

private:
    const FMetaClass* _class;
};
//----------------------------------------------------------------------------
class FFunctionException : public FRTTIException {
    public:
    FFunctionException(const char* what, const FMetaFunction* func = nullptr)
        : FRTTIException(what), _function(func) {}

    const FMetaFunction* Function() const { return _function; }

#if USE_PPE_EXCEPTION_DESCRIPTION
    PPE_RTTI_API virtual FTextWriter& Description(FTextWriter& oss) const override;
#endif

    private:
    const FMetaFunction* _function;
};
//----------------------------------------------------------------------------
class FObjectException : public FRTTIException {
public:
    FObjectException(const char* what, const FMetaObject* obj = nullptr)
        : FRTTIException(what), _object(obj) {}

    const FMetaObject* Object() const { return _object; }

#if USE_PPE_EXCEPTION_DESCRIPTION
    PPE_RTTI_API virtual FTextWriter& Description(FTextWriter& oss) const override;
#endif

private:
    const FMetaObject* _object;
};
//----------------------------------------------------------------------------
class FPropertyException : public FRTTIException {
public:
    FPropertyException(const char* what, const FMetaProperty* prop = nullptr)
        : FRTTIException(what), _property(prop) {}

    const FMetaProperty* Property() const { return _property; }

#if USE_PPE_EXCEPTION_DESCRIPTION
    PPE_RTTI_API virtual FTextWriter& Description(FTextWriter& oss) const override;
#endif

private:
    const FMetaProperty* _property;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE
