#include "stdafx.h"

#include "Atom.h"

#include "AtomVisitor.h"

#include "IO/StringBuilder.h"

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FString FAtom::ToString() const {
    FStringBuilder oss;
    oss << *this;
    return oss.ToString();
}
//----------------------------------------------------------------------------
FWString FAtom::ToWString() const {
    FWStringBuilder oss;
    oss << *this;
    return oss.ToString();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, const RTTI::FAtom& atom) {
    PrettyPrint(oss, atom);
    return oss;
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, const RTTI::FAtom& atom) {
    PrettyPrint(oss, atom);
    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE