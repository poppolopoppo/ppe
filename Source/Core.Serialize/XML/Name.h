#pragma once

#include "Core.Serialize/XML/XML.h"

#include "Core/Container/Token.h"

namespace Core {
namespace XML {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FNameTokenTraits {
public:
    const std::locale& Locale() const { return std::locale::classic(); }
    bool IsAllowedChar(char ch) const;
};
//----------------------------------------------------------------------------
BEGIN_BASICTOKEN_CLASS_DEF(FName, char, ECase::Insensitive, FNameTokenTraits);

    static const FName& Any;

    static void Start(size_t reserve);
    static void Shutdown();

    friend inline bool operator ==(const FName& lhs, const FName& rhs) {
        return (lhs.c_str() == rhs.c_str() ||
                Any.c_str() == lhs.c_str() ||
                Any.c_str() == rhs.c_str() );
    }

    friend inline bool operator !=(const FName& lhs, const FName& rhs) {
        return not operator ==(lhs, rhs);
    }

END_BASICTOKEN_CLASS_DEF();
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace XML
} //!namespace Core
