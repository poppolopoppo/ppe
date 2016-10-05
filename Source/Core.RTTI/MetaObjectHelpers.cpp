#include "stdafx.h"

#include "MetaObjectHelpers.h"

#include "MetaObject.h"

#include "MetaAtom.h"
#include "MetaAtomDatabase.h"
#include "MetaAtomVisitor.h"
#include "MetaClass.h"
#include "MetaProperty.h"

#include "Core/Container/Hash.h"
#include "Core/Container/HashMap.h"
#include "Core/Container/Token.h"
#include "Core/Container/Vector.h"

#include "Core/IO/Format.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Equals(const FMetaObject& lhs, const FMetaObject& rhs) {
    if (&lhs == &rhs)
        return true;

    const FMetaClass *metaClass = lhs.RTTI_MetaClass();
    Assert(metaClass);

    if (rhs.RTTI_MetaClass() != metaClass)
        return false;

    const FMetaObject* plhs = &lhs;
    const FMetaObject* prhs = &rhs;

    const FMetaProperty* notEquals = FindProperty(metaClass,
        [plhs, prhs](const FMetaClass* pMetaClass, const FMetaProperty* pProp) {
            UNUSED(pMetaClass);
            return (not pProp->Equals(plhs, prhs));
        });

    return (nullptr == notEquals);
}
//----------------------------------------------------------------------------
bool DeepEquals(const FMetaObject& lhs, const FMetaObject& rhs) {
    if (&lhs == &rhs)
        return true;

    const FMetaClass *metaClass = lhs.RTTI_MetaClass();
    Assert(metaClass);

    if (rhs.RTTI_MetaClass() != metaClass)
        return false;

    const FMetaObject* plhs = &lhs;
    const FMetaObject* prhs = &rhs;

    const FMetaProperty* notEquals = FindProperty(metaClass,
        [plhs, prhs](const FMetaClass* pMetaClass, const FMetaProperty* pProp) {
            UNUSED(pMetaClass);
            const bool equals = pProp->DeepEquals(plhs, prhs);
            return (not equals);
        });

    return (nullptr == notEquals);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
hash_t hash_value(const FMetaObject& object) {
    const FMetaClass *metaClass = object.RTTI_MetaClass();
    Assert(metaClass);

    const FMetaObject* pObject = &object;

    hash_t h(CORE_HASH_VALUE_SEED);
    ForEachProperty(metaClass,
        [pObject, &h](const FMetaClass* pMetaClass, const FMetaProperty* pProp) {
            UNUSED(pMetaClass);
            hash_combine(h, hash_t(pProp->HashValue(pObject)));
        });

    return h;
}
//----------------------------------------------------------------------------
u128 Fingerprint128(const FMetaObject& object) {
    const FMetaClass *metaClass = object.RTTI_MetaClass();
    Assert(metaClass);

    const FMetaObject* pObject = &object;

    STACKLOCAL_POD_STACK(hash_t, hashValues, 128);
    ForEachProperty(metaClass,
        [pObject, &hashValues](const FMetaClass* pMetaClass, const FMetaProperty* pProp) {
            UNUSED(pMetaClass);
            hashValues.Push(pProp->HashValue(pObject));
        });

    return Fingerprint128(hashValues.MakeView());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void Move(FMetaObject& dst, FMetaObject& src) {
    const FMetaClass *metaClass = src.RTTI_MetaClass();
    Assert(metaClass);
    AssertRelease(dst.RTTI_MetaClass() == metaClass);

    FMetaObject* const pDst = &dst;
    FMetaObject* const pSrc = &src;

    ForEachProperty(metaClass,
        [pDst, pSrc](const FMetaClass* pMetaClass, const FMetaProperty* pProp) {
            UNUSED(pMetaClass);
            if (pProp->IsWritable())
                pProp->Move(pDst, pSrc);
        });
}
//----------------------------------------------------------------------------
void Copy(FMetaObject& dst, const FMetaObject& src) {
    const FMetaClass *metaClass = src.RTTI_MetaClass();
    Assert(metaClass);
    AssertRelease(dst.RTTI_MetaClass() == metaClass);

    FMetaObject* const pDst = &dst;
    const FMetaObject* const pSrc = &src;

    ForEachProperty(metaClass,
        [pDst, pSrc](const FMetaClass* pMetaClass, const FMetaProperty* pProp) {
            UNUSED(pMetaClass);
            if (pProp->IsWritable())
                pProp->Copy(pDst, pSrc);
        });
}
//----------------------------------------------------------------------------
void Swap(FMetaObject& lhs, FMetaObject& rhs) {
    const FMetaClass *metaClass = lhs.RTTI_MetaClass();
    Assert(metaClass);
    AssertRelease(rhs.RTTI_MetaClass() == metaClass);

    FMetaObject* const plhs = &lhs;
    FMetaObject* const prhs = &rhs;

    ForEachProperty(metaClass,
        [plhs, prhs](const FMetaClass* pMetaClass, const FMetaProperty* pProp) {
            UNUSED(pMetaClass);
            if (pProp->IsWritable())
                pProp->Swap(plhs, prhs);
        });
}
//----------------------------------------------------------------------------
FMetaObject *NewCopy(const FMetaObject& src) {
    FMetaObject *const cpy = src.RTTI_MetaClass()->CreateInstance();
    Copy(*cpy, src);
    return cpy;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void DeepCopy(FMetaObject& /* dst */, const FMetaObject& /* src */) {
    // TODO (01/14) : MetaPropertyVisitor
    AssertNotImplemented();
}
//----------------------------------------------------------------------------
FMetaObject *NewDeepCopy(const FMetaObject& src) {
    const FMetaClass* const metaClass = src.RTTI_MetaClass();
    Assert(metaClass);
    AssertRelease(false == metaClass->IsAbstract());
    FMetaObject* const dst = metaClass->CreateInstance();
    Assert(dst);
    DeepCopy(*dst, src);
    return dst;
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
FString ToString(const RTTI::FMetaObject& object) {
    const RTTI::FMetaClass *metaclass = object.RTTI_MetaClass();
    Assert(metaclass);
    return StringFormat("@{0} : {1} = \"{2}\"", &object, metaclass->Name(), object.RTTI_Name());
}
//----------------------------------------------------------------------------
FString ToString(const RTTI::PMetaObject& pobject) {
    return (pobject) ? ToString(*pobject) : FString();
}
//----------------------------------------------------------------------------
FString ToString(const RTTI::PCMetaObject& pobject) {
    return (pobject) ? ToString(*pobject) : FString();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
