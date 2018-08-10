#include "stdafx.h"

#include "MetaProperty.h"

#include "RTTIExceptions.h"

#include "IO/FormatHelpers.h"
#include "IO/TextWriter.h"

#ifdef WITH_PPE_RTTI_PROPERTY_CHECKS
#   include "MetaObject.h"
#   include "Diagnostic/Logger.h"
namespace PPE {
namespace RTTI {
EXTERN_LOG_CATEGORY(PPE_RTTI_API, RTTI)
} //!namespace RTTI
} //!namespace PPE
#endif

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
#ifdef WITH_PPE_RTTI_PROPERTY_CHECKS
void FMetaProperty::CheckProperty_(const FMetaObject& obj, bool write) const {

    if (write && IsReadOnly()) {
        LOG(RTTI, Error, L"writing to readonly property \"{0}::{1}\" on \"{2}\" ({3})",
            obj.RTTI_Class()->Name(),
            _name,
            obj.RTTI_Name(),
            obj.RTTI_Flags());
        PPE_THROW_IT(FPropertyException("writing to readonly property", this));
    }

    if (IsDeprecated()) {
        LOG(RTTI, Warning, L"using deprecated property \"{0}::{1}\" on \"{2}\" ({3})",
            obj.RTTI_Class()->Name(),
            _name,
            obj.RTTI_Name(),
            obj.RTTI_Flags() );
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

    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
