// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "RTTI/Exceptions.h"

#include "MetaTransaction.h"

#if USE_PPE_EXCEPTION_DESCRIPTION
#   include "MetaClass.h"
#   include "MetaObject.h"
#   include "MetaProperty.h"
#   include "RTTI/Typedefs.h"

#   include "IO/TextWriter.h"
#endif

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if USE_PPE_EXCEPTION_DESCRIPTION
FTextWriter& FClassException::Description(FTextWriter& oss) const {
    return oss
        << MakeCStringView(What())
        << ": in class <"
        << _class->Name()
        << "> !";
}
#endif
//----------------------------------------------------------------------------
#if USE_PPE_EXCEPTION_DESCRIPTION
FTextWriter& FFunctionException::Description(FTextWriter& oss) const {
    return oss
        << MakeCStringView(What())
        << ": in function ("
        << _function->Name()
        << ") !";
}
#endif
//----------------------------------------------------------------------------
#if USE_PPE_EXCEPTION_DESCRIPTION
FTextWriter& FPropertyException::Description(FTextWriter& oss) const {
    return oss
        << MakeCStringView(What())
        << ": in property ["
        << _property->Name()
        << "] !";
}
#endif
//----------------------------------------------------------------------------
#if USE_PPE_EXCEPTION_DESCRIPTION
FTextWriter& FObjectException::Description(FTextWriter& oss) const {
    return oss
        << MakeCStringView(What())
        << ": in object <"
        << _object->RTTI_Class()->Name()
        << "> '"
        << _object->RTTI_Name()
        << "' !";
}
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE
