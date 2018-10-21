#pragma once

#include "RTTI_fwd.h"

#include "RTTI/Atom.h"
#include "RTTI/Typedefs.h"
#include "RTTI/TypeTraits.h"

#include "IO/TextWriter_fwd.h"

#if USE_PPE_RTTI_CHECKS
#   define WITH_PPE_RTTI_PROPERTY_CHECKS
#endif

namespace PPE {
namespace RTTI {
class FMetaObject;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EPropertyFlags : u32 {
    Public       = 1<<0,
    Protected    = 1<<1,
    Private      = 1<<2,
    ReadOnly     = 1<<3,
    Deprecated   = 1<<4,
    Member       = 1<<5,
    Dynamic      = 1<<6,
};
ENUM_FLAGS(EPropertyFlags);
//----------------------------------------------------------------------------
class PPE_RTTI_API FMetaProperty {
public:
    FMetaProperty(const FName& name, EPropertyFlags flags, const PTypeTraits& traits, ptrdiff_t memberOffset);
    ~FMetaProperty();

    const FName& Name() const { return _name; }
    const PTypeTraits& Traits() const { return _traits; }
    EPropertyFlags Flags() const { return _flags; }

    bool IsPublic() const       { return (_flags ^ EPropertyFlags::Public       ); }
    bool IsProtected() const    { return (_flags ^ EPropertyFlags::Protected    ); }
    bool IsPrivate() const      { return (_flags ^ EPropertyFlags::Private      ); }
    bool IsReadOnly() const     { return (_flags ^ EPropertyFlags::ReadOnly     ); }
    bool IsDeprecated() const   { return (_flags ^ EPropertyFlags::Deprecated   ); }
    bool IsMember() const       { return (_flags ^ EPropertyFlags::Member       ); }
    bool IsDynamic() const      { return (_flags ^ EPropertyFlags::Dynamic      ); }

#ifdef WITH_PPE_RTTI_PROPERTY_CHECKS
#   define CheckPropertyIFN(obj, write) CheckProperty_(obj, write)
#else
#   define CheckPropertyIFN(obj, write) NOOP()
#endif

    FAtom Get(const FMetaObject& obj) const {
        CheckPropertyIFN(obj, false);
        return MakeAtom_(obj);
    }

    void CopyTo(const FMetaObject& obj, const FAtom& dst) const {
        CheckPropertyIFN(obj, false);
        MakeAtom_(obj).Copy(dst);
    }

    void MoveTo(FMetaObject& obj, const FAtom& dst) const {
        CheckPropertyIFN(obj, true);
        MakeAtom_(obj).Move(dst);
    }

    void CopyFrom(FMetaObject& obj, const FAtom& src) const {
        CheckPropertyIFN(obj, true);
        src.Copy(MakeAtom_(obj));
    }

    void MoveFrom(FMetaObject& obj, FAtom& src) const {
        CheckPropertyIFN(obj, true);
        src.Move(MakeAtom_(obj));
    }

    FAtom ResetToDefaultValue(FMetaObject& obj) const {
        CheckPropertyIFN(obj, true);
        FAtom value = MakeAtom_(obj);
        value.ResetToDefaultValue();
        return value;
    }

private:
#ifdef WITH_PPE_RTTI_PROPERTY_CHECKS
#   undef CheckPropertyIFN
    void CheckProperty_(const FMetaObject& obj, bool write) const;
#endif

    FName _name;
    PTypeTraits _traits;
    EPropertyFlags _flags;
    i32 _memberOffset;

    FORCE_INLINE FAtom MakeAtom_(const FMetaObject& obj) const {
        return FAtom((const u8*)&obj + _memberOffset, _traits);
    }
};
//----------------------------------------------------------------------------
template <typename T, typename _Class>
Meta::TEnableIf<std::is_base_of<FMetaObject, _Class>::value, FMetaProperty>
    MakeProperty(const FName& name, EPropertyFlags flags, T _Class::* member) {
    return FMetaProperty(
        name,
        flags,
        MakeTraits<T>(),
        (ptrdiff_t)&(((_Class*)0)->*member)
    );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
PPE_ASSUME_TYPE_AS_POD(RTTI::FMetaProperty);
} //!namespace PPE

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_RTTI_API FTextWriter& operator <<(FTextWriter& oss, RTTI::EPropertyFlags flags);
PPE_RTTI_API FWTextWriter& operator <<(FWTextWriter& oss, RTTI::EPropertyFlags flags);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
