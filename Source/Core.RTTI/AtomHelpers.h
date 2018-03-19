#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core.RTTI/Atom.h"

#include "Core/Allocator/Alloca.h"
#include "Core/Memory/RefPtr.h"
#include "Core/Meta/PointerWFlags.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define STACKLOCAL_ATOM(_NAME, _PTYPE_TRAITS) \
    const size_t CONCAT(_AllocaSizeInBytes_, _NAME) = (_PTYPE_TRAITS)->SizeInBytes(); \
    const Core::RTTI::FStackLocalAtom _NAME( \
        SYSALLOCA_IFP( CONCAT(_AllocaSizeInBytes_, _NAME) ), \
        CONCAT(_AllocaSizeInBytes_, _NAME), \
        (_PTYPE_TRAITS) )
//----------------------------------------------------------------------------
class FStackLocalAtom {
public:
    FStackLocalAtom(const void* data, size_t sizeInBytes, const PTypeTraits& traits)
        : _traits(traits) {
        Assert(_traits);
        Assert(_traits->SizeInBytes() == sizeInBytes);

        bool usingSysAlloca = true;
        if (nullptr == data) {
            usingSysAlloca = false;
            data = Alloca(sizeInBytes);
        }

        Assert(data);
        _data.Reset((void*)data, usingSysAlloca, false/* unused */);

        _traits->Create(_data.Get());
    }

    ~FStackLocalAtom() {
        _traits->Destroy(_data.Get());

        if (not UsingSysAlloca())
            FreeAlloca(_data.Get());
    }

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
} //!namespace Core

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
CORE_RTTI_API FString ToString(const RTTI::FAtom& atom);
//----------------------------------------------------------------------------
CORE_RTTI_API FWString ToWString(const RTTI::FAtom& atom);
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& operator << (TBasicTextWriter<_Char>& oss, const RTTI::FStackLocalAtom& stackLocalAtom) {
    return oss << stackLocalAtom.MakeAtom();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
