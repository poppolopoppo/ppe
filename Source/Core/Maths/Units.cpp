#include "stdafx.h"

#include "Units.h"

#include "IO/Format.h"
#include "IO/TextWriter.h"

namespace Core {
    //----------------------------------------------------------------------------
    //////////////////////////////////////////////////////////////////////////////
    //----------------------------------------------------------------------------
#define UNITS_BEGIN(NAME)
#define UNITS_END()
#define UNITS_DECL(TAG, SYMBOL, NAME, RATIO, SMALLER) \
    template class Core::Units::TUnit< Core::Units::TUnitTraits<Core::Units::TAG::_Tag, RATIO, SMALLER> >; \
    FTextWriter& operator <<(FTextWriter& oss, \
        const Core::Units::TUnit< Core::Units::TUnitTraits<Core::Units::TAG::_Tag, RATIO, SMALLER> >& unit) { \
            return (*oss.FormatScope()) \
                << unit.Value() \
                << FTextFormat::PadRight(3, ' ') \
                << MakeStringView(" " STRINGIZE(SYMBOL)); \
    } \
    FWTextWriter& operator <<(FWTextWriter& oss, \
        const Core::Units::TUnit< Core::Units::TUnitTraits<Core::Units::TAG::_Tag, RATIO, SMALLER> >& unit) { \
            return (*oss.FormatScope()) \
                << unit.Value() \
                << FTextFormat::PadRight(3, L' ') \
                << MakeStringView(L" " WSTRINGIZE(SYMBOL)); \
    }
//----------------------------------------------------------------------------
#include "Units.Definitions-inl.h"
//----------------------------------------------------------------------------
#undef UNITS_DECL
#undef UNITS_END
#undef UNITS_BEGIN
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
