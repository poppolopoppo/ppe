#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core.RTTI/Atom.h"
#include "Core.RTTI/Typedefs.h"
#include "Core.RTTI/TypeTraits.h"

#if USE_CORE_RTTI_CHECKS
#   define WITH_CORE_RTTI_PROPERTY_CHECKS
#endif

namespace Core {
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
class CORE_RTTI_API FMetaProperty {
public:
    typedef FAtom (*makeatom_func)(const FMetaObject&, const PTypeTraits&);

    FMetaProperty(const FName& name, EPropertyFlags flags, const PTypeTraits& traits, ptrdiff_t memberOffset);
    FMetaProperty(const FName& name, EPropertyFlags flags, const PTypeTraits& traits, makeatom_func makeAtom);
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

#ifdef WITH_CORE_RTTI_PROPERTY_CHECKS
#   define CheckPropertyIFN(obj, write) CheckProperty_(obj, write)
#else
#   define CheckPropertyIFN(obj, write) NOOP()
#endif

    FAtom Get(const FMetaObject& obj) const {
        CheckPropertyIFN(obj, false);
        return MakeAtom_(obj);
    }

    bool CopyTo(const FMetaObject& obj, const FAtom& dst) const {
        CheckPropertyIFN(obj, false);
        return MakeAtom_(obj).CopyTo(dst);
    }

    bool MoveTo(FMetaObject& obj, const FAtom& dst) const {
        CheckPropertyIFN(obj, true);
        return MakeAtom_(obj).MoveTo(dst);
    }

    bool CopyFrom(FMetaObject& obj, const FAtom& src) const {
        CheckPropertyIFN(obj, true);
        return src.CopyTo(MakeAtom_(obj));
    }

    bool MoveFrom(FMetaObject& obj, FAtom& src) const {
        CheckPropertyIFN(obj, true);
        return src.MoveTo(MakeAtom_(obj));
    }

private:
#ifdef WITH_CORE_RTTI_PROPERTY_CHECKS
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
CORE_ASSUME_TYPE_AS_POD(RTTI::FMetaProperty);
} //!namespace Core

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
CORE_RTTI_API std::basic_ostream<char>& operator <<(std::basic_ostream<char>& oss, RTTI::EPropertyFlags flags);
CORE_RTTI_API std::basic_ostream<wchar_t>& operator <<(std::basic_ostream<wchar_t>& oss, RTTI::EPropertyFlags flags);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
