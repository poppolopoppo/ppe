#pragma once

#include "RTTI_fwd.h"

#include "RTTI/Typedefs.h"
#include "RTTI/TypeTraits.h"
#include "RTTI/UserFacet.h"

#include "Container/Vector.h"
#include "IO/TextWriter_fwd.h"

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EEnumFlags : u32 {
    // /!\ Report changes to MetaEnumHelpers.cpp

    None        = 0,
    Flags       = 1<<0,
    Deprecated  = 1<<1,

    All         = UINT32_MAX
};
ENUM_FLAGS(EEnumFlags);
//----------------------------------------------------------------------------
struct FMetaEnumValue {
    FName Name;
    FMetaEnumOrd Ord;
};
PPE_ASSUME_TYPE_AS_POD(FMetaEnumValue);
//----------------------------------------------------------------------------
class PPE_RTTI_API FMetaEnum : Meta::FNonCopyableNorMovable {
public:
    FMetaEnum(const FName& name, EEnumFlags flags, size_t sizeInBytes, const FMetaModule* module) NOEXCEPT;
    virtual ~FMetaEnum();

    const FName& Name() const { return _name; }
    EEnumFlags Flags() const { return _flags; }
    size_t SizeInBytes() const { return _sizeInBytes; }
    const FMetaModule* Module() const { return _module; }
    TMemoryView<const FMetaEnumValue> Values() const { return _values.MakeConstView(); }

    bool IsFlags() const        { return (_flags ^ EEnumFlags::Flags        ); }
    bool IsDeprecated() const   { return (_flags ^ EEnumFlags::Deprecated   ); }

    FMetaEnumFacet& Facets() { return _facets; }
    const FMetaEnumFacet& Facets() const { return _facets; }

    const FMetaEnumValue& DefaultValue() const { return (_values.front()/* always the first value as the default */); }

    const FMetaEnumValue& NameToValue(const RTTI::FName& name) const;
    const FMetaEnumValue* NameToValueIFP(const RTTI::FName& name) const;
    const FMetaEnumValue* NameToValueIFP(const FLazyName& name) const;
    const FMetaEnumValue* NameToValueIFP(const FStringView& name) const;

    const FMetaEnumValue& ValueToName(FMetaEnumOrd value) const;
    const FMetaEnumValue* ValueToNameIFP(FMetaEnumOrd value) const;

    FString ValueToString(FMetaEnumOrd value) const;
    bool ParseValue(const FStringView& str, FMetaEnumOrd* value) const;

    using FExpansion = VECTORINSITU(MetaEnum, FMetaEnumValue, 6);
    bool ExpandValues(FMetaEnumOrd value, FExpansion* expansion) const;
    bool ExpandValues(const FAtom& src, FExpansion* expansion) const;

    bool IsValidName(const FName& name) const NOEXCEPT;
    bool IsValidName(const FAtom& src) const NOEXCEPT;

    bool IsValidValue(FMetaEnumOrd value) const NOEXCEPT;
    bool IsValidValue(const FAtom& src) const NOEXCEPT;

    void SetValue(const FAtom& dst, const FMetaEnumValue& v) const;

    void RegisterValue(FMetaEnumValue&& value);

    // virtual helpers

    virtual PTypeTraits MakeTraits() const NOEXCEPT = 0;

private:
    FName _name;
    EEnumFlags _flags;
    size_t _sizeInBytes;
    const FMetaModule* _module;
    VECTORINSITU(MetaEnum, FMetaEnumValue, 6) _values;
    FMetaEnumFacet _facets;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Enum>
const FMetaEnum* MetaEnum() {
    STATIC_ASSERT(std::is_enum_v<_Enum>);
    return RTTI_Enum(_Enum{});
}
//----------------------------------------------------------------------------
inline FStringLiteral MetaEnumName(const FMetaEnum* metaEnum) {
    Assert(metaEnum);
    return metaEnum->Name().MakeLiteral();
}
//----------------------------------------------------------------------------
inline FMetaEnumOrd MetaEnumDefaultValue(const FMetaEnum* metaEnum) {
    Assert(metaEnum);
    return metaEnum->DefaultValue().Ord;
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
PPE_RTTI_API FTextWriter& operator <<(FTextWriter& oss, RTTI::EEnumFlags flags);
PPE_RTTI_API FWTextWriter& operator <<(FWTextWriter& oss, RTTI::EEnumFlags flags);
//----------------------------------------------------------------------------
PPE_RTTI_API FTextWriter& operator <<(FTextWriter& oss, const RTTI::FMetaEnumValue& value);
PPE_RTTI_API FWTextWriter& operator <<(FWTextWriter& oss, const RTTI::FMetaEnumValue& value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
