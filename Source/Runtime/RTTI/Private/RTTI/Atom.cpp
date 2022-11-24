// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "RTTI/Atom.h"

#include "MetaObject.h"
#include "RTTI/AtomVisitor.h"
#include "RTTI/NativeTypes.h"
#include "RTTI/Typedefs.h"

#include "IO/StringBuilder.h"

namespace PPE {
namespace RTTI {
PPE_ASSERT_TYPE_IS_POD(FAtom);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <typename _Char>
static bool ParseAtom_(const TBasicStringConversion<_Char>& src, FAtom& dst) {
    Assert(dst);

    if (const IScalarTraits* const scalar = dst.Traits()->AsScalar())
        return scalar->FromString(dst.Data(), src);

    return false;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool FAtom::IsAny() const {
    // used when ENativeType is only fwd declared
    return (TypeId() == u32(ENativeType::Any));
}
//----------------------------------------------------------------------------
void* FAtom::Cast(const PTypeTraits& to) const {
    Assert(_data);
    return _traits->Cast(_data, to);
}
//----------------------------------------------------------------------------
bool FAtom::Equals(const FAtom& other) const {
    return _traits->Equals(Data(), other.Data());
}
//----------------------------------------------------------------------------
void FAtom::Copy(const FAtom& dst) const {
    Assert(dst && _traits);
    Assert(*dst.Traits() == *_traits);
    _traits->Copy(_data, dst.Data());
}
//----------------------------------------------------------------------------
void FAtom::Move(const FAtom& dst) {
    Assert(dst && _traits);
    Assert(*dst.Traits() == *_traits);
    _traits->Move(_data, dst.Data());
}
//----------------------------------------------------------------------------
bool FAtom::DeepEquals(const FAtom& other) const {
    return (_traits == other._traits && _traits->DeepEquals(_data, other._data));
}
//----------------------------------------------------------------------------
bool FAtom::DeepCopy(const FAtom& dst) const {
    if (_traits == dst._traits) {
        _traits->DeepCopy(_data, dst.Data());
        return true;
    }
    return false;
}
//----------------------------------------------------------------------------
FString FAtom::ToString() const {
    FStringBuilder oss;
    oss << *this;
    return oss.ToString();
}
//----------------------------------------------------------------------------
FWString FAtom::ToWString() const {
    FWStringBuilder oss;
    oss << *this;
    return oss.ToString();
}
//----------------------------------------------------------------------------
FAtom FAtom::FromObj(const PMetaObject& obj) NOEXCEPT {
    Assert(obj);
    return FAtom{ &obj, obj->RTTI_Traits() };
}
//----------------------------------------------------------------------------
bool FAtom::FromString(const FStringConversion& conv) const NOEXCEPT {
    Assert(Valid());
    if (const IScalarTraits* const pScalar = Traits()->AsScalar())
        return pScalar->FromString(_data, conv);
    return false;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, const RTTI::FAtom& atom) {
    PrettyPrint(oss, atom);
    return oss;
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, const RTTI::FAtom& atom) {
    PrettyPrint(oss, atom);
    return oss;
}
//----------------------------------------------------------------------------
bool operator >>(const FStringConversion& iss, FAtom* atom) {
    Assert(atom);
    return ParseAtom_(iss, *atom);
}
//----------------------------------------------------------------------------
bool operator >>(const FWStringConversion& iss, FAtom* atom) {
    Assert(atom);
    return ParseAtom_(FStringConversion{ WCHAR_TO_UTF_8(iss.Input) }, *atom);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE
