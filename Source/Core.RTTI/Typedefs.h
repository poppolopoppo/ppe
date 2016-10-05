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
class FNameTokenTraits {
public:
    const std::locale& Locale() const { return std::locale::classic(); }
    bool FNameTokenTraits::IsAllowedChar(char ch) const {
        return IsAlnum(ch) || ch == '_' || ch == '-' || ch == '.';
    }
};
//----------------------------------------------------------------------------
BASICTOKEN_CLASS_DEF(FName, char, ECase::Insensitive, FNameTokenTraits);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
using TVector = VECTORINSITU(Container, T, 4);
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
using TPair = Core::TPair<_Key, _Value>;
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
using TDictionary = Core::TAssociativeVector<
    _Key,
    _Value,
    Meta::TEqualTo<_Key>,
    RTTI::TVector<RTTI::TPair<_Key COMMA _Value> >
>;
//----------------------------------------------------------------------------
INSTANTIATE_CLASS_TYPEDEF(BinaryData, RAWSTORAGE_ALIGNED(RTTI, u8, 16));
//----------------------------------------------------------------------------
INSTANTIATE_CLASS_TYPEDEF(OpaqueData, TDictionary<FName, PMetaAtom>);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
