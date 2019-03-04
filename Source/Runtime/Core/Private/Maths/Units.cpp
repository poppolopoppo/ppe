#include "stdafx.h"

#include "Maths/Units.h"

#include "IO/Format.h"
#include "IO/TextWriter.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define UNITS_BEGIN(NAME)
#define UNITS_END()
#define UNITS_DECL(TAG, SYMBOL, NAME, RATIO, SMALLER) \
    EXTERN_TEMPLATE_CLASS_DEF(PPE_CORE_API) PPE::Units::TUnit< PPE::Units::TUnitTraits<PPE::Units::TAG::_Tag, RATIO, SMALLER> >; \
    FTextWriter& operator <<(FTextWriter& oss, \
        const PPE::Units::TUnit< PPE::Units::TUnitTraits<PPE::Units::TAG::_Tag, RATIO, SMALLER> >& unit) { \
            if (oss.Format().Width() > 2) \
                oss.Format().SetWidth(oss.Format().Width() - 3); \
            oss << unit.Value(); \
            return (*oss.FormatScope()) \
                << FTextFormat::PadRight(3, ' ') \
                << MakeStringView(" " STRINGIZE(SYMBOL)); \
    } \
    FWTextWriter& operator <<(FWTextWriter& oss, \
        const PPE::Units::TUnit< PPE::Units::TUnitTraits<PPE::Units::TAG::_Tag, RATIO, SMALLER> >& unit) { \
            if (oss.Format().Width() > 2) \
                oss.Format().SetWidth(oss.Format().Width() - 3); \
            oss << unit.Value(); \
            return (*oss.FormatScope()) \
                << FTextFormat::PadRight(3, L' ') \
                << MakeStringView(" " WSTRINGIZE(SYMBOL)); \
    }
//----------------------------------------------------------------------------
#include "Maths/Units.Definitions-inl.h"
//----------------------------------------------------------------------------
#undef UNITS_DECL
#undef UNITS_END
#undef UNITS_BEGIN
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
