#include "stdafx.h"

#include "TypeTraits.h"

#include "AtomVisitor.h"

#include "Core/IO/Format.h"
#include "Core/IO/String.h"
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
    return StringFormat("TPair<{0}, {1}>", first->TypeInfos().Name(), second->TypeInfos().Name());
}
//----------------------------------------------------------------------------
FTypeId MakeListTypeId(const PTypeTraits& value) {
    return Fingerprint32(TMemoryView<const FTypeId>({FTypeId(ETypeFlags::List), value->TypeId()}));
}
//----------------------------------------------------------------------------
FString MakeListTypeName(const PTypeTraits& value) {
    return StringFormat("TList<{0}>", value->TypeInfos().Name());
}
//----------------------------------------------------------------------------
FTypeId MakeDicoTypeId(const PTypeTraits& key, const PTypeTraits& value) {
    return Fingerprint32(TMemoryView<const FTypeId>({FTypeId(ETypeFlags::Dico), key->TypeId(), value->TypeId()}));
}
//----------------------------------------------------------------------------
FString MakeDicoTypeName(const PTypeTraits& key, const PTypeTraits& value) {
    return StringFormat("TDico<{0}, {1}>", key->TypeInfos().Name(), value->TypeInfos().Name());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool IPairTraits::Accept(IAtomVisitor* visitor, const FAtom& atom) const {
    return visitor->Visit(this, atom);
}
//----------------------------------------------------------------------------
bool IListTraits::Accept(IAtomVisitor* visitor, const FAtom& atom) const {
    return visitor->Visit(this, atom);
}
//----------------------------------------------------------------------------
bool IDicoTraits::Accept(IAtomVisitor* visitor, const FAtom& atom) const {
    return visitor->Visit(this, atom);
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
std::basic_ostream<char>& operator <<(std::basic_ostream<char>& oss, RTTI::ETypeFlags flags) {;
    bool s = false;;
;
    if (flags & RTTI::ETypeFlags::Scalar)   { if (s) oss << '|'; else s = true; oss << "Scalar";; }
    if (flags & RTTI::ETypeFlags::Pair)     { if (s) oss << '|'; else s = true; oss << "Pair"; ;}
    if (flags & RTTI::ETypeFlags::List)     { if (s) oss << '|'; else s = true; oss << "List"; }
    if (flags & RTTI::ETypeFlags::Dico)     { if (s) oss << '|'; else s = true; oss << "Dico"; }
    if (flags & RTTI::ETypeFlags::Native)   { if (s) oss << '|'; else s = true; oss << "Native"; }

    return oss;
}
//----------------------------------------------------------------------------
std::basic_ostream<wchar_t>& operator <<(std::basic_ostream<wchar_t>& oss, RTTI::ETypeFlags flags) {
    bool s = false;

    if (flags & RTTI::ETypeFlags::Scalar)   { if (s) oss << L'|'; else s = true; oss << L"Scalar"; }
    if (flags & RTTI::ETypeFlags::Pair)     { if (s) oss << L'|'; else s = true; oss << L"Pair"; }
    if (flags & RTTI::ETypeFlags::List)     { if (s) oss << L'|'; else s = true; oss << L"List"; }
    if (flags & RTTI::ETypeFlags::Dico)     { if (s) oss << L'|'; else s = true; oss << L"Dico"; }
    if (flags & RTTI::ETypeFlags::Native)   { if (s) oss << L'|'; else s = true; oss << L"Native"; }

    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
