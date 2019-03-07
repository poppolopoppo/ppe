#pragma once

#include "RTTI_fwd.h"

#include "Container/Vector.h"
#include "IO/TextWriter_fwd.h"
#include "RTTI/Typedefs.h"
#include "RTTI/TypeTraits.h"

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EEnumFlags : u32 {
    None        = 0,
    Flags       = 1<<0,
    Deprecated  = 1<<1,

    All         = UINT32_MAX
};
ENUM_FLAGS(EEnumFlags);
//----------------------------------------------------------------------------
struct FMetaEnumValue {
    FName Name;
    FMetaEnumOrd Value;
};
//----------------------------------------------------------------------------
class PPE_RTTI_API FMetaEnum : Meta::FNonCopyableNorMovable {
public:
    FMetaEnum(const FName& name, EEnumFlags flags, size_t sizeInBytes, const FMetaNamespace* metaNamespace);
    virtual ~FMetaEnum();

    const FName& Name() const { return _name; }
    EEnumFlags Flags() const { return _flags; }
    size_t SizeInBytes() const { return _sizeInBytes; }
    const FMetaNamespace* Namespace() const { return _namespace; }
    TMemoryView<const FMetaEnumValue> Values() const { return _values.MakeConstView(); }

    bool IsFlags() const        { return (_flags ^ EEnumFlags::Flags        ); }
    bool IsDeprecated() const   { return (_flags ^ EEnumFlags::Deprecated   ); }

    const FMetaEnumValue& DefaultValue() const { return (_values.front()/* always the first value as the default */); }

    const FMetaEnumValue& NameToValue(const RTTI::FName& name) const;
    const FMetaEnumValue* NameToValueIFP(const RTTI::FName& name) const;
    const FMetaEnumValue* NameToValueIFP(const FStringView& name) const;

    const FMetaEnumValue& ValueToName(FMetaEnumOrd value) const;
    const FMetaEnumValue* ValueToNameIFP(FMetaEnumOrd value) const;

    using FExpansion = VECTORINSITU(MetaEnum, const FMetaEnumValue*, 6);
    bool ExpandValues(FMetaEnumOrd value, FExpansion* expansion) const;
    bool ExpandValues(const FAtom& src, FExpansion* expansion) const;

    bool IsValidName(const FName& name) const;
    bool IsValidName(const FAtom& src) const;

    bool IsValidValue(FMetaEnumOrd value) const;
    bool IsValidValue(const FAtom& src) const;

    void SetValue(const FAtom& dst, const FMetaEnumValue& v) const;

    void RegisterValue(FMetaEnumValue&& value);

    // virtual helpers

    virtual PTypeTraits MakeTraits() const = 0;

private:
    FName _name;
    EEnumFlags _flags;
    size_t _sizeInBytes;
    const FMetaNamespace* _namespace;
    VECTORINSITU(MetaEnum, FMetaEnumValue, 6) _values;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// must be overloaded for each supported enum type (see RTTI_Macros.h)
//void* RTTI_Enum(void) NOEXCEPT = delete;
//----------------------------------------------------------------------------
template <typename T>
using TMetaEnum = Meta::TRemoveConst<
    Meta::TRemovePointer<decltype(
        RTTI_Enum(std::declval<Meta::TDecay<T>>())
    )> >;
//----------------------------------------------------------------------------
template <typename T>
const FMetaEnum* MetaEnum() {
    return TMetaEnum<T>::Get();
}
//----------------------------------------------------------------------------
inline FStringView MetaEnumName(const FMetaEnum* metaEnum) {
    Assert(metaEnum);
    return metaEnum->Name().MakeView();
}
//----------------------------------------------------------------------------
inline FMetaEnumOrd MetaEnumDefaultValue(const FMetaEnum* metaEnum) {
    Assert(metaEnum);
    return metaEnum->DefaultValue().Value;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
PPE_ASSUME_TYPE_AS_POD(RTTI::FMetaEnumValue);
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
