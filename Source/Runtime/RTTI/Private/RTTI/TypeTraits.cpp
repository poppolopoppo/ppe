#include "stdafx.h"

#include "RTTI/TypeTraits.h"

#include "MetaClass.h"
#include "MetaDatabase.h"
#include "MetaEnum.h"
#include "MetaObject.h"
#include "RTTI/AtomVisitor.h"
#include "RTTI/NativeTypes.h"
#include "RTTI/Typedefs.h"

#include "Diagnostic/DebugFunction.h"
#include "HAL/PlatformHash.h"
#include "IO/Format.h"
#include "IO/FormatHelpers.h"
#include "IO/String.h"
#include "IO/StringBuilder.h"
#include "IO/TextWriter.h"
#include "Memory/HashFunctions.h"
#include "Memory/MemoryView.h"

namespace PPE {
STATIC_ASSERT(Meta::is_pod_v<RTTI::PTypeTraits>);
namespace RTTI {
STATIC_ASSERT(Meta::is_pod_v<PTypeTraits>);
STATIC_ASSERT(Meta::has_trivial_destructor<ITypeTraits>::value);
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

        if (is_object_v(elt_flags))
            flags = flags + ETypeFlags::Object;

        if (not is_pod_v(elt_flags))
            flags = flags - ETypeFlags::POD;
        if (not is_trivially_destructible_v(elt_flags))
            flags = flags - ETypeFlags::TriviallyDestructible;
    }

    return flags;
}
//----------------------------------------------------------------------------
ETypeFlags MakeListTypeFlags(const PTypeTraits& value) {
    ETypeFlags flags = ETypeFlags::List;

    if (is_object_v(value->TypeFlags()))
        flags = flags + ETypeFlags::Object;

    return flags;
}
//----------------------------------------------------------------------------
ETypeFlags MakeDicoTypeFlags(const PTypeTraits& key, const PTypeTraits& value) {
    ETypeFlags flags = ETypeFlags::Dico;

    if (is_object_v(key->TypeFlags()) | is_object_v(value->TypeFlags()))
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
FTextWriter& operator <<(FTextWriter& oss, RTTI::ETypeFlags flags) {
    auto sep = Fmt::NotFirstTime('|');

    if (flags & RTTI::ETypeFlags::Scalar) { oss << sep << "Scalar";; }
    if (flags & RTTI::ETypeFlags::Tuple) { oss << sep << "Tuple"; ; }
    if (flags & RTTI::ETypeFlags::List) { oss << sep << "List"; }
    if (flags & RTTI::ETypeFlags::Dico) { oss << sep << "Dico"; }

    if (flags & RTTI::ETypeFlags::Arithmetic) { oss << sep << "Arithmetic"; }
    if (flags & RTTI::ETypeFlags::Boolean) { oss << sep << "Boolean"; }
    if (flags & RTTI::ETypeFlags::Enum) { oss << sep << "Enum"; }
    if (flags & RTTI::ETypeFlags::FloatingPoint) { oss << sep << "FloatingPoint"; }
    if (flags & RTTI::ETypeFlags::Native) { oss << sep << "Native"; }
    if (flags & RTTI::ETypeFlags::Object) { oss << sep << "Object"; }
    if (flags & RTTI::ETypeFlags::String) { oss << sep << "String"; }
    if (flags & RTTI::ETypeFlags::SignedIntegral) { oss << sep << "SignedIntegral"; }
    if (flags & RTTI::ETypeFlags::UnsignedIntegral) { oss << sep << "UnsignedIntegral"; }

    if (flags & RTTI::ETypeFlags::POD) { oss << sep << "POD"; }
    if (flags & RTTI::ETypeFlags::TriviallyDestructible) { oss << sep << "TriviallyDestructible"; }

    return oss;
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, RTTI::ETypeFlags flags) {
    auto sep = Fmt::NotFirstTime(L'|');

    if (flags & RTTI::ETypeFlags::Scalar) { oss << sep << L"Scalar";; }
    if (flags & RTTI::ETypeFlags::Tuple) { oss << sep << L"Tuple"; ; }
    if (flags & RTTI::ETypeFlags::List) { oss << sep << L"List"; }
    if (flags & RTTI::ETypeFlags::Dico) { oss << sep << L"Dico"; }

    if (flags & RTTI::ETypeFlags::Arithmetic) { oss << sep << L"Arithmetic"; }
    if (flags & RTTI::ETypeFlags::Boolean) { oss << sep << L"Boolean"; }
    if (flags & RTTI::ETypeFlags::Enum) { oss << sep << L"Enum"; }
    if (flags & RTTI::ETypeFlags::FloatingPoint) { oss << sep << L"FloatingPoint"; }
    if (flags & RTTI::ETypeFlags::Native) { oss << sep << L"Native"; }
    if (flags & RTTI::ETypeFlags::Object) { oss << sep << L"Object"; }
    if (flags & RTTI::ETypeFlags::String) { oss << sep << L"String"; }
    if (flags & RTTI::ETypeFlags::SignedIntegral) { oss << sep << L"SignedIntegral"; }
    if (flags & RTTI::ETypeFlags::UnsignedIntegral) { oss << sep << L"UnsignedIntegral"; }

    if (flags & RTTI::ETypeFlags::POD) { oss << sep << L"POD"; }
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
} //!namespace RTTI
} //!namespace PPE

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DEBUG_FUNCTION(PPE_RTTI_API, DebugTraitsTypeName, FStringView, (const RTTI::ITypeTraits& traits), {
    return traits.TypeName();
})
//----------------------------------------------------------------------------
DEBUG_FUNCTION(PPE_RTTI_API, DebugTraitsTypeInfos, RTTI::FTypeInfos, (const RTTI::ITypeTraits& traits), {
    return traits.TypeInfos();
})
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
