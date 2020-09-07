#include "stdafx.h"

#include "MetaEnum.h"

#include "RTTI/Atom.h"
#include "RTTI/NativeTypes.h"

#include "IO/FormatHelpers.h"
#include "IO/StringBuilder.h"
#include "IO/TextWriter.h"

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FMetaEnum::FMetaEnum(const FName& name, EEnumFlags flags, size_t sizeInBytes, const FMetaModule* module) NOEXCEPT
:   _name(name)
,   _flags(flags)
,   _sizeInBytes(sizeInBytes)
,   _module(module) {
    Assert_NoAssume(not _name.empty());
    Assert_NoAssume(_sizeInBytes);
    Assert_NoAssume(_module);
}
//----------------------------------------------------------------------------
FMetaEnum::~FMetaEnum() = default;
//----------------------------------------------------------------------------
const FMetaEnumValue& FMetaEnum::NameToValue(const RTTI::FName& name) const {
    const FMetaEnumValue* const pValue = NameToValueIFP(name);
    Assert(pValue);
    return (*pValue);
}
//----------------------------------------------------------------------------
const FMetaEnumValue* FMetaEnum::NameToValueIFP(const RTTI::FName& name) const {
    Assert_NoAssume(not name.empty());

    const auto view = _values.MakeConstView();
    const auto it = view.FindIf([name](const FMetaEnumValue& v) {
        return (v.Name == name);
    });

    return (view.end() == it ? nullptr : std::addressof(*it));
}
//----------------------------------------------------------------------------
const FMetaEnumValue* FMetaEnum::NameToValueIFP(const FLazyName& name) const {
    Assert_NoAssume(not name.empty());

    const auto view = _values.MakeConstView();
    const auto it = view.FindIf([name](const FMetaEnumValue& v) {
        return (v.Name == name);
    });

    return (view.end() == it ? nullptr : std::addressof(*it));
}
//----------------------------------------------------------------------------
const FMetaEnumValue* FMetaEnum::NameToValueIFP(const FStringView& name) const {
    return NameToValueIFP(FLazyName{ name });
}
//----------------------------------------------------------------------------
const FMetaEnumValue& FMetaEnum::ValueToName(FMetaEnumOrd value) const {
    const FMetaEnumValue* const pValue = ValueToNameIFP(value);
    Assert(pValue);
    return (*pValue);
}
//----------------------------------------------------------------------------
const FMetaEnumValue* FMetaEnum::ValueToNameIFP(FMetaEnumOrd value) const {
    const auto view = _values.MakeConstView();
    const auto it = view.FindIf([value](const FMetaEnumValue& v) {
        return (v.Ord == value);
    });

    return (view.end() == it ? nullptr : std::addressof(*it));
}
//----------------------------------------------------------------------------
FString FMetaEnum::ValueToString(FMetaEnumOrd value) const {
    FExpansion expanded;
    Verify(ExpandValues(value, &expanded));

    FStringBuilder oss;
    auto sep = Fmt::NotFirstTime('|');

    for (const FMetaEnumValue& v : expanded)
        oss << sep << v.Name;

    return oss.ToString();
}
//----------------------------------------------------------------------------
bool FMetaEnum::ParseValue(const FStringView& str, FMetaEnumOrd* value) const {
    Assert(value);

    FMetaEnumOrd parsed{ 0 };

    if (IsFlags()) {
        FStringView input{ str }, name;
        while (Split(input, '|', name)) {
            if (const FMetaEnumValue* const enumFlag = NameToValueIFP(name)) {
                parsed |= enumFlag->Ord;
            }
            else {
                return false;
            }
        }
    }
    else {
        if (const FMetaEnumValue* const enumValue = NameToValueIFP(str)) {
            parsed = enumValue->Ord;
        }
        else {
            return false;
        }
    }

    *value = parsed;
    return true;
}
//----------------------------------------------------------------------------
bool FMetaEnum::ExpandValues(FMetaEnumOrd value, FExpansion* expansion) const {
    Assert(expansion);

    if (IsFlags()) {
        for (const FMetaEnumValue& v : _values) {
            if ((v.Ord & value) == v.Ord)
                expansion->push_back(v);
        }
    }
    else if (const FMetaEnumValue* v = ValueToNameIFP(value)) {
        expansion->push_back(*v);
    }

    return (not expansion->empty());
}
//----------------------------------------------------------------------------
bool FMetaEnum::ExpandValues(const FAtom& src, FExpansion* expansion) const {
    Assert_NoAssume(src);
    Assert_NoAssume(src.TypeFlags() & ETypeFlags::Enum);
    Assert_NoAssume(src.Traits()->AsScalar());
    Assert_NoAssume(src.Traits()->ToScalar().EnumClass() == this);

    switch (_sizeInBytes) {
    case sizeof(u8) :
        return ExpandValues(checked_cast<FMetaEnumOrd>(*static_cast<u8*>(src.Data())), expansion);
    case sizeof(u16):
        return ExpandValues(checked_cast<FMetaEnumOrd>(*static_cast<u16*>(src.Data())), expansion);
    case sizeof(u32):
        return ExpandValues(checked_cast<FMetaEnumOrd>(*static_cast<u32*>(src.Data())), expansion);
    case sizeof(u64):
        return ExpandValues(checked_cast<FMetaEnumOrd>(*static_cast<u64*>(src.Data())), expansion);
    }

    AssertNotImplemented();
}
//----------------------------------------------------------------------------
bool FMetaEnum::IsValidName(const FName& name) const NOEXCEPT {
    return (NameToValueIFP(name) != nullptr);
}
//----------------------------------------------------------------------------
bool FMetaEnum::IsValidName(const FAtom& src) const NOEXCEPT {
    Assert_NoAssume(src);

    FName name;
    return (src.PromoteCopy(MakeAtom(&name))
        ? IsValidName(name)
        : false );
}
//----------------------------------------------------------------------------
bool FMetaEnum::IsValidValue(FMetaEnumOrd value) const NOEXCEPT {
    if (IsFlags()) {
        for (const FMetaEnumValue& it : _values)
            if ((it.Ord & value) == it.Ord)
                value &= ~it.Ord;

        return (0 == value); // no unknown values
    }
    else {
        for (const FMetaEnumValue& it : _values)
            if (it.Ord == value)
                return true;

        return false;
    }
}
//----------------------------------------------------------------------------
bool FMetaEnum::IsValidValue(const FAtom& src) const NOEXCEPT {
    Assert_NoAssume(src);

    FMetaEnumOrd value = 0;
    return (src.PromoteCopy(MakeAtom(&value))
        ? IsValidValue(value)
        : false );
}
//----------------------------------------------------------------------------
void FMetaEnum::SetValue(const FAtom& dst, const FMetaEnumValue& v) const {
    Assert_NoAssume(dst);
    Assert_NoAssume(dst.TypeFlags() & ETypeFlags::Enum);
    Assert_NoAssume(dst.Traits()->AsScalar());
    Assert_NoAssume(dst.Traits()->ToScalar().EnumClass() == this);

    switch (_sizeInBytes) {
    case sizeof(u8) :
        *static_cast<u8*>(dst.Data()) = checked_cast<u8>(v.Ord);
        return;
    case sizeof(u16):
        *static_cast<u16*>(dst.Data()) = checked_cast<u16>(v.Ord);
        return;
    case sizeof(u32):
        *static_cast<u32*>(dst.Data()) = checked_cast<u32>(v.Ord);
        return;
    case sizeof(u64):
        *static_cast<u64*>(dst.Data()) = checked_cast<u64>(v.Ord);
        return;
    }

    AssertNotImplemented();
}
//----------------------------------------------------------------------------
void FMetaEnum::RegisterValue(FMetaEnumValue&& value) {
    Assert_NoAssume(not value.Name.empty());
    AssertMessage_NoAssume(
        L"there's already an enum entry with the same name",
        _values.MakeView().FindIf([&value](const FMetaEnumValue& it) {
            return (it.Name == value.Name);
        }) == _values.MakeConstView().end() );
    AssertMessage_NoAssume(
        L"there's already an enum entry with the same value",
        _values.MakeView().FindIf([&value](const FMetaEnumValue& it) {
            return (it.Name == value.Name);
        }) == _values.MakeConstView().end() );

    _values.push_back(std::move(value));
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
FTextWriter& operator <<(FTextWriter& oss, RTTI::EEnumFlags flags) {
    auto sep = Fmt::NotFirstTime('|');
    if (flags ^ RTTI::EEnumFlags::Flags         )  oss << sep << "Flags";
    if (flags ^ RTTI::EEnumFlags::Deprecated    )  oss << sep << "Deprecated";
    return oss;
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, RTTI::EEnumFlags flags) {
    auto sep = Fmt::NotFirstTime(L'|');
    if (flags ^ RTTI::EEnumFlags::Flags         )  oss << sep << L"Flags";
    if (flags ^ RTTI::EEnumFlags::Deprecated    )  oss << sep << L"Deprecated";
    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, const RTTI::FMetaEnumValue& value) {
    return oss << value.Name << '(' << value.Ord << ')';
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, const RTTI::FMetaEnumValue& value) {
    return oss << value.Name << L'(' << value.Ord << L')';
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
