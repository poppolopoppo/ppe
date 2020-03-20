#include "stdafx.h"

#include "RTTI/TypeTraits.h"

#include "MetaClass.h"
#include "MetaDatabase.h"
#include "MetaEnum.h"
#include "MetaObject.h"
#include "RTTI/AtomVisitor.h"
#include "RTTI/NativeTypes.h"
#include "RTTI/Typedefs.h"

#include "HAL/PlatformHash.h"
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
// Using FPlatformHash::CRC32() to be platform agnostic !
// This is very important since we want this id to be stable across all platforms
//----------------------------------------------------------------------------
FTypeId MakeTupleTypeId(const TMemoryView<const PTypeTraits>& elements) {
    STATIC_ASSERT(sizeof(FTypeId) == sizeof(u32));

    u32 h = u32(ETypeFlags::Tuple);
    foreachitem(it, elements)
        h = u32(FPlatformHash::CRC32((*it)->TypeId(), h));

    return FTypeId(h);
}
//----------------------------------------------------------------------------
FTypeId MakeListTypeId(const PTypeTraits& value) {
    return u32(FPlatformHash::CRC32(u32(ETypeFlags::List), value->TypeId()));
}
//----------------------------------------------------------------------------
FTypeId MakeDicoTypeId(const PTypeTraits& key, const PTypeTraits& value) {
    return u32(FPlatformHash::CRC32(u32(ETypeFlags::Dico),
        FPlatformHash::CRC32(key->TypeId(), value->TypeId()) ));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ETypeFlags MakeTupleTypeFlags(const TMemoryView<const PTypeTraits>& elements) {
    ETypeFlags flags =
        ETypeFlags::POD +
        ETypeFlags::Tuple +
        ETypeFlags::TriviallyDestructible;

    for (const PTypeTraits& it : elements) {
        const ETypeFlags elt_flags = it->TypeFlags();

        if (elt_flags ^ ETypeFlags::Object)
            flags = flags + ETypeFlags::Object;

        if (not (elt_flags ^ ETypeFlags::POD))
            flags = flags - ETypeFlags::POD;
        if (not (elt_flags ^ ETypeFlags::TriviallyDestructible))
            flags = flags - ETypeFlags::TriviallyDestructible;
    }

    return flags;
}
//----------------------------------------------------------------------------
ETypeFlags MakeListTypeFlags(const PTypeTraits& value) {
    ETypeFlags flags = ETypeFlags::List;

    if (value->TypeFlags() ^ ETypeFlags::Object)
        flags = flags + ETypeFlags::Object;

    return flags;
}
//----------------------------------------------------------------------------
ETypeFlags MakeDicoTypeFlags(const PTypeTraits& key, const PTypeTraits& value) {
    ETypeFlags flags = ETypeFlags::Dico;

    if ((key->TypeFlags() ^ ETypeFlags::Object) |
        (value->TypeFlags() ^ ETypeFlags::Object) )
        flags = flags + ETypeFlags::Object;

    return flags;
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
PPE_RTTI_API PTypeTraits MakeTraitsFromTypename(const FName& typename_) {
    const FMetaDatabaseReadable db;
    return db->TraitsIFP(typename_);
}
//----------------------------------------------------------------------------
PPE_RTTI_API PTypeTraits MakeTraitsFromTypename(const FStringView& typename_) {
    const FMetaDatabaseReadable db;
    return db->TraitsIFP(typename_);
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
    if (flags & RTTI::ETypeFlags::Object)   { oss << sep << "Object"; }
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
    if (flags & RTTI::ETypeFlags::Object)   { oss << sep << L"Object"; }
    if (flags & RTTI::ETypeFlags::Native)   { oss << sep << L"Native"; }
    if (flags & RTTI::ETypeFlags::POD)      { oss << sep << L"POD"; }
    if (flags & RTTI::ETypeFlags::TriviallyDestructible) { oss << sep << L"TriviallyDestructible"; }

    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, const RTTI::FNamedTypeInfos& typeInfos) {
    return oss
        << hash_t(typeInfos.Id()) << " : "
        << typeInfos.Name() << " ["
        << typeInfos.SizeInBytes() << "] ("
        << typeInfos.Flags() << ')';
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, const RTTI::FNamedTypeInfos& typeInfos) {
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
