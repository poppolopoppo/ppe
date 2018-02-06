#include "stdafx.h"

#include "MetaObjectHelpers.h"

#include "Atom.h"
#include "AtomVisitor.h"
#include "MetaClass.h"
#include "MetaObject.h"
#include "MetaProperty.h"
#include "NativeTypes.h"
#include "TypeTraits.h"

#include "Core/Container/Hash.h"
#include "Core/Container/Stack.h"
#include "Core/Memory/HashFunctions.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Equals(const FMetaObject& lhs, const FMetaObject& rhs) {
    if (&lhs == &rhs)
        return true;

    const FMetaClass* metaClass = lhs.RTTI_Class();
    Assert(metaClass);

    if (rhs.RTTI_Class() != metaClass)
        return false;

    for (const FMetaProperty* prop : metaClass->AllProperties()) {
        const FAtom lhsValue = prop->Get(lhs);
        const FAtom rhsValue = prop->Get(rhs);

        if (not lhsValue.Equals(rhsValue))
            return false;
    }

    return true;
}
//----------------------------------------------------------------------------
bool DeepEquals(const FMetaObject& lhs, const FMetaObject& rhs) {
    if (&lhs == &rhs)
        return true;

    const FMetaClass* metaClass = lhs.RTTI_Class();
    Assert(metaClass);

    if (rhs.RTTI_Class() != metaClass)
        return false;

    for (const FMetaProperty* prop : metaClass->AllProperties()) {
        const FAtom lhsValue = prop->Get(lhs);
        const FAtom rhsValue = prop->Get(rhs);

        if (not lhsValue.DeepEquals(rhsValue))
            return false;
    }

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void Move(const FMetaObject& src, FMetaObject& dst) {
    if (&src == &dst)
        return;

    const FMetaClass* metaClass = src.RTTI_Class();
    Assert(metaClass);
    Assert(dst.RTTI_Class() == metaClass);

    for (const FMetaProperty* prop : metaClass->AllProperties()) {
        FAtom srcValue = prop->Get(src);
        const FAtom dstValue = prop->Get(dst);

        srcValue.Move(dstValue);
    }
}
//----------------------------------------------------------------------------
void Copy(const FMetaObject& src, FMetaObject& dst) {
    if (&src == &dst)
        return;

    const FMetaClass* metaClass = src.RTTI_Class();
    Assert(metaClass);
    Assert(dst.RTTI_Class() == metaClass);

    for (const FMetaProperty* prop : metaClass->AllProperties()) {
        const FAtom srcValue = prop->Get(src);
        const FAtom dstValue = prop->Get(dst);

        srcValue.Copy(dstValue);
    }
}
//----------------------------------------------------------------------------
void Swap(FMetaObject& lhs, FMetaObject& rhs) {
    if (&lhs == &rhs)
        return;

    const FMetaClass* metaClass = lhs.RTTI_Class();
    Assert(metaClass);
    Assert(rhs.RTTI_Class() == metaClass);

    for (const FMetaProperty* prop : metaClass->AllProperties()) {
        const FAtom lhsValue = prop->Get(lhs);
        const FAtom rhsValue = prop->Get(rhs);

        lhsValue.SwapValue(rhsValue);
    }
}
//----------------------------------------------------------------------------
void Clone(const FMetaObject& src, PMetaObject& pdst) {
    const FMetaClass* metaClass = src.RTTI_Class();
    Assert(metaClass);

    if (not metaClass->CreateInstance(pdst))
        AssertNotReached();

    FMetaObject& dst = *pdst;
    for (const FMetaProperty* prop : metaClass->AllProperties()) {
        const FAtom srcValue = prop->Get(src);
        const FAtom dstValue = prop->Get(dst);

        srcValue.Copy(dstValue);
    }
}
//----------------------------------------------------------------------------
void DeepCopy(const FMetaObject& src, FMetaObject& dst) {
    if (&src == &dst)
        return;

    const FMetaClass* metaClass = src.RTTI_Class();
    Assert(metaClass);
    Assert(dst.RTTI_Class() == metaClass);

    for (const FMetaProperty* prop : metaClass->AllProperties()) {
        const FAtom srcValue = prop->Get(src);
        const FAtom dstValue = prop->Get(dst);

        srcValue.DeepCopy(dstValue);
    }
}
//----------------------------------------------------------------------------
void DeepClone(const FMetaObject& src, PMetaObject& pdst) {
    const FMetaClass* metaClass = src.RTTI_Class();
    Assert(metaClass);

    if (not metaClass->CreateInstance(pdst))
        AssertNotReached();

    FMetaObject& dst = *pdst;
    for (const FMetaProperty* prop : metaClass->AllProperties()) {
        const FAtom srcValue = prop->Get(src);
        const FAtom dstValue = prop->Get(dst);

        srcValue.DeepCopy(dstValue);
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void ResetToDefaultValue(FMetaObject& obj) {
    const FMetaClass* metaClass = obj.RTTI_Class();
    Assert(metaClass);

    for (const FMetaProperty* prop : metaClass->AllProperties())
        prop->ResetToDefaultValue(obj);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
hash_t hash_value(const FMetaObject& obj) {
    const FMetaClass* metaClass = obj.RTTI_Class();
    Assert(metaClass);

    hash_t h(CORE_HASH_VALUE_SEED);
    for (const FMetaProperty* prop : metaClass->AllProperties())
        hash_combine(h, prop->Get(obj));

    return h;
}
//----------------------------------------------------------------------------
u128 Fingerprint128(const FMetaObject& obj) {
    const FMetaClass* metaClass = obj.RTTI_Class();
    Assert(metaClass);

    STACKLOCAL_POD_STACK(hash_t, hashValues, metaClass->NumProperties(true));
    for (const FMetaProperty* prop : metaClass->AllProperties())
        hashValues.Push(prop->Get(obj).HashValue());

    return Core::Fingerprint128(hashValues.MakeConstView());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, const RTTI::FMetaObject& obj) {
    RTTI::PMetaObject pobj(const_cast<RTTI::FMetaObject*>(&obj));
    PrettyPrint(oss, InplaceAtom(pobj));
    RemoveRef_AssertReachZero_KeepAlive(pobj);
    return oss;
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, const RTTI::FMetaObject& obj) {
    RTTI::PMetaObject pobj(const_cast<RTTI::FMetaObject*>(&obj));
    PrettyPrint(oss, InplaceAtom(pobj));
    RemoveRef_AssertReachZero_KeepAlive(pobj);
    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
