#include "stdafx.h"

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
FWTextWriter& FClassException::Description(FWTextWriter& oss) const {
    return oss
        << MakeCStringView(What())
        << L": in class <"
        << _class->Name()
        << L"> !";
}
#endif
//----------------------------------------------------------------------------
#if USE_PPE_EXCEPTION_DESCRIPTION
FWTextWriter& FPropertyException::Description(FWTextWriter& oss) const {
    return oss
        << MakeCStringView(What())
        << L": in property ["
        << _property->Name()
        << L"] !";
}
#endif
//----------------------------------------------------------------------------
#if USE_PPE_EXCEPTION_DESCRIPTION
FWTextWriter& FObjectException::Description(FWTextWriter& oss) const {
    return oss
        << MakeCStringView(What())
        << L": in object <"
        << _object->RTTI_Class()->Name()
        << L"> '"
        << _object->RTTI_Name()
        << L"' !";
}
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE
