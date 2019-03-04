#pragma once

#include "RTTI.h"

#include "RTTI/Atom.h"

#include "Allocator/Alloca.h"
#include "Meta/PointerWFlags.h"

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define STACKLOCAL_ATOM(_NAME, _PTYPE_TRAITS) \
    const size_t CONCAT(_AllocaSizeInBytes_, _NAME) = (_PTYPE_TRAITS)->SizeInBytes(); \
    const PPE::RTTI::FStackLocalAtom _NAME( \
        SYSALLOCA_IFP( CONCAT(_AllocaSizeInBytes_, _NAME) ), \
        CONCAT(_AllocaSizeInBytes_, _NAME), \
        (_PTYPE_TRAITS) )
//----------------------------------------------------------------------------
class PPE_RTTI_API FStackLocalAtom {
public:
    FStackLocalAtom(const void* data, size_t sizeInBytes, const PTypeTraits& traits);
    ~FStackLocalAtom();

    FStackLocalAtom(const FStackLocalAtom& ) = delete;
    FStackLocalAtom& operator =(const FStackLocalAtom& ) = delete;

    FStackLocalAtom(FStackLocalAtom&& ) = delete;
    FStackLocalAtom& operator =(FStackLocalAtom&& ) = delete;

    bool UsingSysAlloca() const { return _data.Flag0(); }

    FAtom MakeAtom() const { return FAtom(_data.Get(), _traits); }
    operator FAtom() const { return MakeAtom(); }

private:
    Meta::TPointerWFlags<void> _data;
    PTypeTraits _traits;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_RTTI_API FString ToString(const RTTI::FAtom& atom);
//----------------------------------------------------------------------------
PPE_RTTI_API FWString ToWString(const RTTI::FAtom& atom);
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& operator << (TBasicTextWriter<_Char>& oss, const RTTI::FStackLocalAtom& stackLocalAtom) {
    return oss << stackLocalAtom.MakeAtom();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
