#pragma once

#include "Core.Serialize/XML/XML.h"

#include "Core/Container/Token.h"

namespace Core {
namespace XML {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class NameTokenTraits {
public:
    const std::locale& Locale() const { return std::locale::classic(); }
    bool IsAllowedChar(char ch) const;
};
//----------------------------------------------------------------------------
BEGIN_BASICTOKEN_CLASS_DEF(Name, char, Case::Insensitive, NameTokenTraits);

    static const Name& Any;

    static void Start(size_t reserve);
    static void Shutdown();

    friend inline bool operator ==(const Name& lhs, const Name& rhs) {
        return (lhs.c_str() == rhs.c_str() ||
                Any.c_str() == lhs.c_str() ||
                Any.c_str() == rhs.c_str() );
    }

    friend inline bool operator !=(const Name& lhs, const Name& rhs) {
        return not operator ==(lhs, rhs);
    }

END_BASICTOKEN_CLASS_DEF();
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace XML
} //!namespace Core
