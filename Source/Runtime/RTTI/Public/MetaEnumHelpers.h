#pragma once

#include "RTTI_fwd.h"

#include "Allocator/TrackingMalloc.h"
#include "MetaEnum.h"
#include "MetaModule.h"

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Self, typename T>
class TInScopeMetaEnum : public FMetaEnum {
protected:
    TInScopeMetaEnum(const FName& name, EEnumFlags flags, const FMetaModule* metaNamespace) NOEXCEPT
    :   FMetaEnum(name, flags, sizeof(T), metaNamespace) {
        STATIC_ASSERT(std::is_enum_v<T>);
    }

    virtual PTypeTraits MakeTraits() const NOEXCEPT override final {
        return RTTI::MakeTraits<T>();
    }

public:
    class RTTI_FMetaEnumHandle : public FMetaEnumHandle {
    public:
        RTTI_FMetaEnumHandle() NOEXCEPT
            : FMetaEnumHandle(
                _Self::Module(),
                CreateMetaEnum_,
                [](FMetaEnum* metaEnum) { TRACKING_DELETE(MetaEnum, metaEnum); })
        {}
    };

    static const FMetaEnum* Get() NOEXCEPT {
        const RTTI_FMetaEnumHandle& handle = _Self::metaenum_handle();
        Assert(handle.Enum());
        return handle.Enum();
    }

private:
    static FMetaEnum* CreateMetaEnum_(const FMetaModule* metaNamespace) {
        return TRACKING_NEW(MetaEnum, _Self) { metaNamespace };
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Bind RTTI internal enums
PPE_RTTI_API const FMetaEnum* RTTI_Enum(ENativeType) NOEXCEPT;
PPE_RTTI_API const FMetaEnum* RTTI_Enum(EClassFlags) NOEXCEPT;
PPE_RTTI_API const FMetaEnum* RTTI_Enum(EEnumFlags) NOEXCEPT;
PPE_RTTI_API const FMetaEnum* RTTI_Enum(EFunctionFlags) NOEXCEPT;
PPE_RTTI_API const FMetaEnum* RTTI_Enum(EObjectFlags) NOEXCEPT;
PPE_RTTI_API const FMetaEnum* RTTI_Enum(EParameterFlags) NOEXCEPT;
PPE_RTTI_API const FMetaEnum* RTTI_Enum(EPropertyFlags) NOEXCEPT;
PPE_RTTI_API const FMetaEnum* RTTI_Enum(ETypeFlags) NOEXCEPT;
PPE_RTTI_API const FMetaEnum* RTTI_Enum(EVisitorFlags) NOEXCEPT;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_RTTI_API bool MetaEnumParse(const FMetaEnum& menum, const FStringView& str, FMetaEnumOrd* ord) NOEXCEPT;
//----------------------------------------------------------------------------
template <typename _Enum>
bool MetaEnumParse(const FStringView& str, _Enum* value) NOEXCEPT {
    Assert(value);
    FMetaEnumOrd ord;
    if (MetaEnumParse(*MetaEnum<_Enum>(), str, &ord)) {
        *value = static_cast<_Enum>(ord);
        return true;
    }
    return false;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE
