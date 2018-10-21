#include "stdafx.h"

#include "RTTI/TypeTraits.h"

#include "RTTI/AtomVisitor.h"

#include "IO/Format.h"
#include "IO/FormatHelpers.h"
#include "IO/String.h"
#include "IO/StringBuilder.h"
#include "IO/TextWriter.h"
#include "Memory/HashFunctions.h"
#include "Memory/MemoryView.h"

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTypeId MakeTupleTypeId(const TMemoryView<const PTypeTraits>& elements) {
    STACKLOCAL_POD_ARRAY(FTypeId, signature, elements.size() + 1);
    signature[0] = FTypeId(ETypeFlags::Tuple);

    forrange(i, 0, elements.size())
        signature[i + 1] = elements[i]->TypeId();

    return Fingerprint32(signature.Cast<const FTypeId>());
}
//----------------------------------------------------------------------------
ETypeFlags MakeTupleTypeFlags(const TMemoryView<const PTypeTraits>& elements) {
    ETypeFlags is_pod = ETypeFlags::POD;
    ETypeFlags is_trivially_destructible = ETypeFlags::TriviallyDestructible;

    foreachitem(elt, elements) {
        const ETypeFlags elt_flags = (*elt)->TypeFlags();
        if (not (elt_flags ^ ETypeFlags::POD))
            is_pod = ETypeFlags(0);
        if (not (elt_flags ^ ETypeFlags::POD))
            is_trivially_destructible = ETypeFlags(0);
    }

    return (ETypeFlags::Tuple + is_pod + is_trivially_destructible);
}
//----------------------------------------------------------------------------
FString MakeTupleTypeName(const TMemoryView<const PTypeTraits>& elements) {
    PPE_LEAKDETECTOR_WHITELIST_SCOPE();

    FStringBuilder oss;
    oss << "TTuple<";

    auto sep = Fmt::NotFirstTime(", ");
    for (const auto& elt : elements)
        oss << sep << elt->TypeInfos().Name();

    oss << '>';

    return oss.ToString();
}
//----------------------------------------------------------------------------
FTypeId MakeListTypeId(const PTypeTraits& value) {
    return Fingerprint32(TMemoryView<const FTypeId>({FTypeId(ETypeFlags::List), value->TypeId()}));
}
//----------------------------------------------------------------------------
FString MakeListTypeName(const PTypeTraits& value) {
    PPE_LEAKDETECTOR_WHITELIST_SCOPE();
    return StringFormat("TList<{0}>", value->TypeInfos().Name());
}
//----------------------------------------------------------------------------
FTypeId MakeDicoTypeId(const PTypeTraits& key, const PTypeTraits& value) {
    return Fingerprint32(TMemoryView<const FTypeId>({FTypeId(ETypeFlags::Dico), key->TypeId(), value->TypeId()}));
}
//----------------------------------------------------------------------------
FString MakeDicoTypeName(const PTypeTraits& key, const PTypeTraits& value) {
    PPE_LEAKDETECTOR_WHITELIST_SCOPE();
    return StringFormat("TDico<{0}, {1}>", key->TypeInfos().Name(), value->TypeInfos().Name());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool ITupleTraits::Accept(IAtomVisitor* visitor, void* data) const {
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
} //!namespace PPE

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, RTTI::ETypeFlags flags) {;
    auto sep = Fmt::NotFirstTime('|');
;
    if (flags & RTTI::ETypeFlags::Scalar)   { oss << sep << "Scalar";; }
    if (flags & RTTI::ETypeFlags::Tuple)    { oss << sep << "Tuple"; ;}
    if (flags & RTTI::ETypeFlags::List)     { oss << sep << "List"; }
    if (flags & RTTI::ETypeFlags::Dico)     { oss << sep << "Dico"; }
    if (flags & RTTI::ETypeFlags::Native)   { oss << sep << "Native"; }
    if (flags & RTTI::ETypeFlags::Enum)     { oss << sep << "Enum"; }
    if (flags & RTTI::ETypeFlags::POD)      { oss << sep << "POD"; }
    if (flags & RTTI::ETypeFlags::TriviallyDestructible) { oss << sep << "TriviallyDestructible"; }

    return oss;
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, RTTI::ETypeFlags flags) {
    auto sep = Fmt::NotFirstTime(L'|');

    if (flags & RTTI::ETypeFlags::Scalar)   { oss << sep << L"Scalar"; }
    if (flags & RTTI::ETypeFlags::Tuple)    { oss << sep << L"Tuple"; ;}
    if (flags & RTTI::ETypeFlags::List)     { oss << sep << L"List"; }
    if (flags & RTTI::ETypeFlags::Dico)     { oss << sep << L"Dico"; }
    if (flags & RTTI::ETypeFlags::Enum)     { oss << sep << L"Enum"; }
    if (flags & RTTI::ETypeFlags::Native)   { oss << sep << L"Native"; }
    if (flags & RTTI::ETypeFlags::POD)      { oss << sep << L"POD"; }
    if (flags & RTTI::ETypeFlags::TriviallyDestructible) { oss << sep << L"TriviallyDestructible"; }

    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, const RTTI::FTypeInfos& typeInfos) {
    return oss
        << hash_t(typeInfos.Id()) << " : "
        << typeInfos.Name() << " ["
        << typeInfos.SizeInBytes() << "] ("
        << typeInfos.Flags() << ')';
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, const RTTI::FTypeInfos& typeInfos) {
    return oss
        << hash_t(typeInfos.Id()) << L" : "
        << typeInfos.Name() << L" ["
        << typeInfos.SizeInBytes() << L"] ("
        << typeInfos.Flags() << L')';
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
