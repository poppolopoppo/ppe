#include "stdafx.h"

#include "MetaProperty.h"

#ifdef WITH_CORE_RTTI_PROPERTY_CHECKS
#   include "MetaObject.h"
#   include "Core/Diagnostic/Logger.h"
#endif

namespace Core {
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
#ifdef WITH_CORE_RTTI_PROPERTY_CHECKS
void FMetaProperty::CheckProperty_(const FMetaObject& obj, bool write) const {

    if (write && IsReadOnly()) {
        LOG(Error, L"[RTTI] Writing to readonly property \"{0}::{1}\" on \"{2}\" ({3})",
            obj.RTTI_Class()->Name(),
            _name,
            obj.RTTI_Name(),
            obj.RTTI_Flags() );
    }

    if (IsDeprecated()) {
        LOG(Warning, L"[RTTI] Using deprecated property \"{0}::{1}\" on \"{2}\" ({3})",
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
} //!namespace Core

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
std::basic_ostream<char>& operator <<(std::basic_ostream<char>& oss, RTTI::EPropertyFlags flags) {
    bool s = false;

    if (flags & RTTI::EPropertyFlags::Public)    { if (s) oss << '|'; else s = true; oss << "Public"; }
    if (flags & RTTI::EPropertyFlags::Protected) { if (s) oss << '|'; else s = true; oss << "Protected"; }
    if (flags & RTTI::EPropertyFlags::Private)   { if (s) oss << '|'; else s = true; oss << "Private"; }
    if (flags & RTTI::EPropertyFlags::ReadOnly)  { if (s) oss << '|'; else s = true; oss << "ReadOnly"; }
    if (flags & RTTI::EPropertyFlags::Deprecated){ if (s) oss << '|'; else s = true; oss << "Deprecated"; }
    if (flags & RTTI::EPropertyFlags::Member)    { if (s) oss << '|'; else s = true; oss << "Member"; }
    if (flags & RTTI::EPropertyFlags::Dynamic)   { if (s) oss << '|'; else s = true; oss << "Dynamic"; }

    return oss;
}
//----------------------------------------------------------------------------
std::basic_ostream<wchar_t>& operator <<(std::basic_ostream<wchar_t>& oss, RTTI::EPropertyFlags flags) {
    bool s = false;

    if (flags & RTTI::EPropertyFlags::Public)    { if (s) oss << L'|'; else s = true; oss << L"Public"; }
    if (flags & RTTI::EPropertyFlags::Protected) { if (s) oss << L'|'; else s = true; oss << L"Protected"; }
    if (flags & RTTI::EPropertyFlags::Private)   { if (s) oss << L'|'; else s = true; oss << L"Private"; }
    if (flags & RTTI::EPropertyFlags::ReadOnly)  { if (s) oss << L'|'; else s = true; oss << L"ReadOnly"; }
    if (flags & RTTI::EPropertyFlags::Deprecated){ if (s) oss << L'|'; else s = true; oss << L"Deprecated"; }
    if (flags & RTTI::EPropertyFlags::Member)    { if (s) oss << L'|'; else s = true; oss << L"Member"; }
    if (flags & RTTI::EPropertyFlags::Dynamic)   { if (s) oss << L'|'; else s = true; oss << L"Dynamic"; }

    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
