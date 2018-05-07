#include "stdafx.h"

#include "TypeTraits.h"

#include "AtomVisitor.h"

#include "Core/IO/Format.h"
#include "Core/IO/FormatHelpers.h"
#include "Core/IO/String.h"
#include "Core/IO/StringBuilder.h"
#include "Core/IO/TextWriter.h"
#include "Core/Memory/HashFunctions.h"
#include "Core/Memory/MemoryView.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTypeId MakePairTypeId(const PTypeTraits& first, const PTypeTraits& second) {
    return Fingerprint32(TMemoryView<const FTypeId>({FTypeId(ETypeFlags::Pair), first->TypeId(), second->TypeId()}));
}
//----------------------------------------------------------------------------
FString MakePairTypeName(const PTypeTraits& first, const PTypeTraits& second) {
    CORE_LEAKDETECTOR_WHITELIST_SCOPE();
    return StringFormat("TPair<{0}, {1}>", first->TypeInfos().Name(), second->TypeInfos().Name());
}
//----------------------------------------------------------------------------
FTypeId MakeListTypeId(const PTypeTraits& value) {
    return Fingerprint32(TMemoryView<const FTypeId>({FTypeId(ETypeFlags::List), value->TypeId()}));
}
//----------------------------------------------------------------------------
FString MakeListTypeName(const PTypeTraits& value) {
    CORE_LEAKDETECTOR_WHITELIST_SCOPE();
    return StringFormat("TList<{0}>", value->TypeInfos().Name());
}
//----------------------------------------------------------------------------
FTypeId MakeDicoTypeId(const PTypeTraits& key, const PTypeTraits& value) {
    return Fingerprint32(TMemoryView<const FTypeId>({FTypeId(ETypeFlags::Dico), key->TypeId(), value->TypeId()}));
}
//----------------------------------------------------------------------------
FString MakeDicoTypeName(const PTypeTraits& key, const PTypeTraits& value) {
    CORE_LEAKDETECTOR_WHITELIST_SCOPE();
    return StringFormat("TDico<{0}, {1}>", key->TypeInfos().Name(), value->TypeInfos().Name());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool IPairTraits::Accept(IAtomVisitor* visitor, void* data) const {
    return visitor->Visit(this, data);
}
//----------------------------------------------------------------------------
bool IListTraits::Accept(IAtomVisitor* visitor, void* data) const {
    return visitor->Visit(this, data);
}
//----------------------------------------------------------------------------
bool IDicoTraits::Accept(IAtomVisitor* visitor, void* data) const {
    return visitor->Visit(this, data);
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
FTextWriter& operator <<(FTextWriter& oss, RTTI::ETypeFlags flags) {;
    auto sep = Fmt::NotFirstTime('|');
;
    if (flags & RTTI::ETypeFlags::Scalar)   { oss << sep << "Scalar";; }
    if (flags & RTTI::ETypeFlags::Pair)     { oss << sep << "Pair"; ;}
    if (flags & RTTI::ETypeFlags::List)     { oss << sep << "List"; }
    if (flags & RTTI::ETypeFlags::Dico)     { oss << sep << "Dico"; }
    if (flags & RTTI::ETypeFlags::Native)   { oss << sep << "Native"; }
    if (flags & RTTI::ETypeFlags::Object)   { oss << sep << "Object"; }

    return oss;
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, RTTI::ETypeFlags flags) {
    auto sep = Fmt::NotFirstTime(L'|');

    if (flags & RTTI::ETypeFlags::Scalar)   { oss << sep << L"Scalar"; }
    if (flags & RTTI::ETypeFlags::Pair)     { oss << sep << L"Pair"; }
    if (flags & RTTI::ETypeFlags::List)     { oss << sep << L"List"; }
    if (flags & RTTI::ETypeFlags::Dico)     { oss << sep << L"Dico"; }
    if (flags & RTTI::ETypeFlags::Native)   { oss << sep << L"Native"; }
    if (flags & RTTI::ETypeFlags::Object)   { oss << sep << L"Object"; }

    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
