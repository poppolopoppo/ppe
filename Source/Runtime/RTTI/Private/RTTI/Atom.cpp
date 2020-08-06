#include "stdafx.h"

#include "RTTI/Atom.h"

#include "MetaObject.h"
#include "RTTI/AtomVisitor.h"
#include "RTTI/NativeTypes.h"

#include "IO/StringBuilder.h"

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_ASSERT_TYPE_IS_POD(FAtom);
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
    return _traits->Equals(*this, other);
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
    return true;
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
FAtom FAtom::FromObj(const PMetaObject& obj) {
    Assert(obj);
    return FAtom{ &obj, obj->RTTI_Traits() };
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
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
