#include "stdafx.h"

#include "MetaEnum.h"

#include "RTTI/Atom.h"
#include "RTTI/TypeInfos.h"

#include "IO/FormatHelpers.h"
#include "IO/TextWriter.h"

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FMetaEnum::FMetaEnum(const FName& name, EEnumFlags flags, size_t sizeInBytes, const FMetaNamespace* metaNamespace)
:   _name(name)
,   _flags(flags)
,   _sizeInBytes(sizeInBytes)
,   _namespace(metaNamespace) {
    Assert_NoAssume(not _name.empty());
    Assert_NoAssume(_sizeInBytes);
    Assert_NoAssume(_namespace);
}
//----------------------------------------------------------------------------
FMetaEnum::~FMetaEnum()
{}
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
const FMetaEnumValue& FMetaEnum::ValueToName(i64 value) const {
    const FMetaEnumValue* const pValue = ValueToNameIFP(value);
    Assert(pValue);
    return (*pValue);
}
//----------------------------------------------------------------------------
const FMetaEnumValue* FMetaEnum::ValueToNameIFP(i64 value) const {
    const auto view = _values.MakeConstView();
    const auto it = view.FindIf([value](const FMetaEnumValue& v) {
        return (v.Value == value);
    });

    return (view.end() == it ? nullptr : std::addressof(*it));
}
//----------------------------------------------------------------------------
bool FMetaEnum::ExpandValues(i64 value, FExpansion* expansion) const {
    Assert(expansion);

    if (IsFlags()) {
        for (const FMetaEnumValue& v : _values) {
            if ((v.Value & value) == v.Value)
                expansion->push_back(&v);
        }
    }
    else if (const FMetaEnumValue* v = ValueToNameIFP(value)) {
        expansion->push_back(v);
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
        return ExpandValues(checked_cast<i64>(*reinterpret_cast<u8*>(src.Data())), expansion);
    case sizeof(u16):
        return ExpandValues(checked_cast<i64>(*reinterpret_cast<u16*>(src.Data())), expansion);
    case sizeof(u32):
        return ExpandValues(checked_cast<i64>(*reinterpret_cast<u32*>(src.Data())), expansion);
    case sizeof(u64):
        return ExpandValues(checked_cast<i64>(*reinterpret_cast<u64*>(src.Data())), expansion);
    }

    AssertNotImplemented();
}
//----------------------------------------------------------------------------
void FMetaEnum::SetValue(const FAtom& dst, const FMetaEnumValue& v) const {
    Assert_NoAssume(dst);
    Assert_NoAssume(dst.TypeFlags() & ETypeFlags::Enum);
    Assert_NoAssume(dst.Traits()->AsScalar());
    Assert_NoAssume(dst.Traits()->ToScalar().EnumClass() == this);

    switch (_sizeInBytes) {
    case sizeof(u8) :
        *reinterpret_cast<u8*>(dst.Data()) = checked_cast<u8>(v.Value);
        return;
    case sizeof(u16):
        *reinterpret_cast<u16*>(dst.Data()) = checked_cast<u16>(v.Value);
        return;
    case sizeof(u32):
        *reinterpret_cast<u32*>(dst.Data()) = checked_cast<u32>(v.Value);
        return;
    case sizeof(u64):
        *reinterpret_cast<u64*>(dst.Data()) = checked_cast<u64>(v.Value);
        return;
    }

    AssertNotImplemented();
}
//----------------------------------------------------------------------------
void FMetaEnum::RegisterValue(FMetaEnumValue&& value) {
    Assert_NoAssume(not value.Name.empty());
    Assert_NoAssume(
        "there's already an enum entry with the same name",
        _values.MakeView().FindIf([&value](const FMetaEnumValue& it) {
            return (it.Name == value.Name);
        }) == _values.MakeConstView().end() );
    Assert_NoAssume(
        "there's already an enum entry with the same value",
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
    return oss << value.Name << '(' << value.Value << ')';
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, const RTTI::FMetaEnumValue& value) {
    return oss << value.Name << L'(' << value.Value << L')';
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
