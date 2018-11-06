#include "stdafx.h"

#include "MetaProperty.h"

#include "RTTI/Exceptions.h"

#include "IO/FormatHelpers.h"
#include "IO/TextWriter.h"

#ifdef WITH_PPE_RTTI_PROPERTY_CHECKS
#   include "MetaObject.h"
#   include "Diagnostic/Logger.h"
#   define CheckPropertyIFN(obj, write) CheckProperty_(obj, write)
namespace PPE {
namespace RTTI {
EXTERN_LOG_CATEGORY(PPE_RTTI_API, RTTI)
} //!namespace RTTI
} //!namespace PPE
#else
#   define CheckPropertyIFN(obj, write) NOOP()
#endif //!WITH_PPE_RTTI_PROPERTY_CHECKS

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FMetaProperty::FMetaProperty(const FName& name, EPropertyFlags flags, const PTypeTraits& traits, ptrdiff_t memberOffset)
    : _name(name)
    , _traits(traits)
    , _flags(flags)
    , _memberOffset(checked_cast<i32>(memberOffset)) {
    Assert(not _name.empty());
    Assert(_traits.Valid());
    Assert(0 != u32(_flags));
}
//----------------------------------------------------------------------------
FMetaProperty::~FMetaProperty()
{}
//----------------------------------------------------------------------------
FAtom FMetaProperty::Get(const FMetaObject& obj) const {
    CheckPropertyIFN(obj, false);
    return MakeAtom_(obj);
}
//----------------------------------------------------------------------------
void FMetaProperty::CopyTo(const FMetaObject& obj, const FAtom& dst) const {
    CheckPropertyIFN(obj, false);
    MakeAtom_(obj).Copy(dst);
}
//----------------------------------------------------------------------------
void FMetaProperty::MoveTo(FMetaObject& obj, const FAtom& dst) const {
    CheckPropertyIFN(obj, true);
    MakeAtom_(obj).Move(dst);
}
//----------------------------------------------------------------------------
void FMetaProperty::CopyFrom(FMetaObject& obj, const FAtom& src) const {
    CheckPropertyIFN(obj, true);
    src.Copy(MakeAtom_(obj));
}
//----------------------------------------------------------------------------
void FMetaProperty::MoveFrom(FMetaObject& obj, FAtom& src) const {
    CheckPropertyIFN(obj, true);
    src.Move(MakeAtom_(obj));
}
//----------------------------------------------------------------------------
FAtom FMetaProperty::ResetToDefaultValue(FMetaObject& obj) const {
    CheckPropertyIFN(obj, true);
    FAtom value = MakeAtom_(obj);
    value.ResetToDefaultValue();
    return value;
}
//----------------------------------------------------------------------------
#ifdef WITH_PPE_RTTI_PROPERTY_CHECKS
void FMetaProperty::CheckProperty_(const FMetaObject& obj, bool write) const {

    if (write && IsReadOnly()) {
        LOG(RTTI, Error, L"writing to readonly property \"{0}::{1}\" on \"{2}\" ({3})",
            obj.RTTI_Class()->Name(),
            _name,
            obj.RTTI_Name(),
            obj.RTTI_Flags() );
        PPE_THROW_IT(FPropertyException("writing to readonly property", this));
    }

    if (IsDeprecated()) {
        LOG(RTTI, Warning, L"using deprecated property \"{0}::{1}\" on \"{2}\" ({3})",
            obj.RTTI_Class()->Name(),
            _name,
            obj.RTTI_Name(),
            obj.RTTI_Flags() );
    }

    if (write && obj.RTTI_IsFrozen()) {
        LOG(RTTI, Fatal, L"can't modify property \"{0}::{1}\" on frozen \"{2}\" ({3})",
            obj.RTTI_Class()->Name(),
            _name,
            obj.RTTI_Name(),
            obj.RTTI_Flags() );
        PPE_THROW_IT(FPropertyException("writing to readonly property", this));
    }
}
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, RTTI::EPropertyFlags flags) {
    auto sep = Fmt::NotFirstTime('|');

    if (flags & RTTI::EPropertyFlags::Public)    { oss << sep << "Public"; }
    if (flags & RTTI::EPropertyFlags::Protected) { oss << sep << "Protected"; }
    if (flags & RTTI::EPropertyFlags::Private)   { oss << sep << "Private"; }
    if (flags & RTTI::EPropertyFlags::ReadOnly)  { oss << sep << "ReadOnly"; }
    if (flags & RTTI::EPropertyFlags::Deprecated){ oss << sep << "Deprecated"; }
    if (flags & RTTI::EPropertyFlags::Member)    { oss << sep << "Member"; }
    if (flags & RTTI::EPropertyFlags::Dynamic)   { oss << sep << "Dynamic"; }
    if (flags & RTTI::EPropertyFlags::Transient) { oss << sep << "Transient"; }

    return oss;
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, RTTI::EPropertyFlags flags) {
    auto sep = Fmt::NotFirstTime(L'|');

    if (flags & RTTI::EPropertyFlags::Public)    { oss << sep << L"Public"; }
    if (flags & RTTI::EPropertyFlags::Protected) { oss << sep << L"Protected"; }
    if (flags & RTTI::EPropertyFlags::Private)   { oss << sep << L"Private"; }
    if (flags & RTTI::EPropertyFlags::ReadOnly)  { oss << sep << L"ReadOnly"; }
    if (flags & RTTI::EPropertyFlags::Deprecated){ oss << sep << L"Deprecated"; }
    if (flags & RTTI::EPropertyFlags::Member)    { oss << sep << L"Member"; }
    if (flags & RTTI::EPropertyFlags::Dynamic)   { oss << sep << L"Dynamic"; }
    if (flags & RTTI::EPropertyFlags::Transient) { oss << sep << L"Transient"; }

    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#ifdef WITH_PPE_RTTI_PROPERTY_CHECKS
#   undef CheckPropertyIFN
#endif
