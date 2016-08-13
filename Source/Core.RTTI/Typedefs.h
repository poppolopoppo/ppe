#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core/Container/AssociativeVector.h"
#include "Core/Container/Pair.h"
#include "Core/Container/RawStorage.h"
#include "Core/Container/Vector.h"
#include "Core/Container/Token.h"

#include "Core/IO/String.h"

#include "Core/Maths/ScalarVector_fwd.h"
#include "Core/Maths/ScalarMatrix_fwd.h"

namespace Core {
namespace RTTI {
FWD_REFPTR(MetaAtom);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class NameTokenTraits {
public:
    const std::locale& Locale() const { return std::locale::classic(); }
    bool NameTokenTraits::IsAllowedChar(char ch) const {
        return IsAlnum(ch) || ch == '_' || ch == '-' || ch == '.';
    }
};
//----------------------------------------------------------------------------
BASICTOKEN_CLASS_DEF(Name, char, Case::Insensitive, NameTokenTraits);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
using Vector = VECTORINSITU(Container, T, 4);
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
using Pair = Core::Pair<_Key, _Value>;
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
using Dictionary = Core::AssociativeVector<
    _Key,
    _Value,
    Meta::EqualTo<_Key>,
    RTTI::Vector<RTTI::Pair<_Key COMMA _Value> >
>;
//----------------------------------------------------------------------------
INSTANTIATE_CLASS_TYPEDEF(BinaryData, RAWSTORAGE_ALIGNED(RTTI, u8, 16));
//----------------------------------------------------------------------------
INSTANTIATE_CLASS_TYPEDEF(OpaqueData, Dictionary<Name, PMetaAtom>);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
