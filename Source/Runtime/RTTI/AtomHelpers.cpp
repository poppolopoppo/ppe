#include "stdafx.h"

#include "AtomHelpers.h"

#include "AtomVisitor.h"

#include "Core/IO/StringBuilder.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FStackLocalAtom::FStackLocalAtom(const void* data, size_t sizeInBytes, const PTypeTraits& traits)
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

    _traits->Construct(_data.Get());
}
//----------------------------------------------------------------------------
FStackLocalAtom::~FStackLocalAtom() {
    _traits->Destroy(_data.Get());

    if (not UsingSysAlloca())
        FreeAlloca(_data.Get(), _traits->SizeInBytes());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FString ToString(const RTTI::FAtom& atom) {
    FStringBuilder oss;
    RTTI::PrettyPrint(oss, atom);
    return oss.ToString();
}
//----------------------------------------------------------------------------
FWString ToWString(const RTTI::FAtom& atom) {
    FWStringBuilder oss;
    RTTI::PrettyPrint(oss, atom);
    return oss.ToString();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
