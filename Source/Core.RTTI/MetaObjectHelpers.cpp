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
bool Equals(const MetaObject& lhs, const MetaObject& rhs) {
    if (&lhs == &rhs)
        return true;

    const MetaClass *metaClass = lhs.RTTI_MetaClass();
    Assert(metaClass);

    if (rhs.RTTI_MetaClass() != metaClass)
        return false;

    const MetaObject* plhs = &lhs;
    const MetaObject* prhs = &rhs;

    const MetaProperty* notEquals = FindProperty(metaClass,
        [plhs, prhs](const MetaClass* pMetaClass, const MetaProperty* pProp) {
            UNUSED(pMetaClass);
            return (not pProp->Equals(plhs, prhs));
        });

    return (nullptr == notEquals);
}
//----------------------------------------------------------------------------
bool DeepEquals(const MetaObject& lhs, const MetaObject& rhs) {
    if (&lhs == &rhs)
        return true;

    const MetaClass *metaClass = lhs.RTTI_MetaClass();
    Assert(metaClass);

    if (rhs.RTTI_MetaClass() != metaClass)
        return false;

    const MetaObject* plhs = &lhs;
    const MetaObject* prhs = &rhs;

    const MetaProperty* notEquals = FindProperty(metaClass,
        [plhs, prhs](const MetaClass* pMetaClass, const MetaProperty* pProp) {
            UNUSED(pMetaClass);
            const bool equals = pProp->DeepEquals(plhs, prhs);
            return (not equals);
        });

    return (nullptr == notEquals);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
hash_t hash_value(const MetaObject& object) {
    const MetaClass *metaClass = object.RTTI_MetaClass();
    Assert(metaClass);

    const MetaObject* pObject = &object;

    hash_t h(CORE_HASH_VALUE_SEED);
    ForEachProperty(object.RTTI_MetaClass(),
        [pObject, &h](const MetaClass* pMetaClass, const MetaProperty* pProp) {
            UNUSED(pMetaClass);
            hash_combine(h, hash_t(pProp->HashValue(pObject)));
        });

    return h;
}
//----------------------------------------------------------------------------
u128 Fingerprint128(const MetaObject& object) {
    const MetaClass *metaClass = object.RTTI_MetaClass();
    Assert(metaClass);

    const MetaObject* pObject = &object;

    STACKLOCAL_POD_STACK(hash_t, hashValues, 128);
    ForEachProperty(object.RTTI_MetaClass(),
        [pObject, &hashValues](const MetaClass* pMetaClass, const MetaProperty* pProp) {
            UNUSED(pMetaClass);
            hashValues.Push(pProp->HashValue(pObject));
        });

    return Fingerprint128(hashValues.MakeView());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void Move(MetaObject& dst, MetaObject& src) {
    const MetaClass *metaClass = src.RTTI_MetaClass();
    Assert(metaClass);
    AssertRelease(dst.RTTI_MetaClass() == metaClass);

    MetaObject* const pDst = &dst;
    MetaObject* const pSrc = &src;

    ForEachProperty(metaClass,
        [pDst, pSrc](const MetaClass* pMetaClass, const MetaProperty* pProp) {
            UNUSED(pMetaClass);
            if (pProp->IsWritable())
                pProp->Move(pDst, pSrc);
        });
}
//----------------------------------------------------------------------------
void Copy(MetaObject& dst, const MetaObject& src) {
    const MetaClass *metaClass = src.RTTI_MetaClass();
    Assert(metaClass);
    AssertRelease(dst.RTTI_MetaClass() == metaClass);

    MetaObject* const pDst = &dst;
    const MetaObject* const pSrc = &src;

    ForEachProperty(metaClass,
        [pDst, pSrc](const MetaClass* pMetaClass, const MetaProperty* pProp) {
            UNUSED(pMetaClass);
            if (pProp->IsWritable())
                pProp->Copy(pDst, pSrc);
        });
}
//----------------------------------------------------------------------------
void Swap(MetaObject& lhs, MetaObject& rhs) {
    const MetaClass *metaClass = lhs.RTTI_MetaClass();
    Assert(metaClass);
    AssertRelease(rhs.RTTI_MetaClass() == metaClass);

    MetaObject* const plhs = &lhs;
    MetaObject* const prhs = &rhs;

    ForEachProperty(metaClass,
        [plhs, prhs](const MetaClass* pMetaClass, const MetaProperty* pProp) {
            UNUSED(pMetaClass);
            if (pProp->IsWritable())
                pProp->Swap(plhs, prhs);
        });
}
//----------------------------------------------------------------------------
MetaObject *NewCopy(const MetaObject& src) {
    MetaObject *const cpy = src.RTTI_MetaClass()->CreateInstance();
    Copy(*cpy, src);
    return cpy;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void DeepCopy(MetaObject& /* dst */, const MetaObject& /* src */) {
    // TODO (01/14) : MetaPropertyVisitor
    AssertNotImplemented();
}
//----------------------------------------------------------------------------
MetaObject *NewDeepCopy(const MetaObject& src) {
    const MetaClass* const metaClass = src.RTTI_MetaClass();
    Assert(metaClass);
    AssertRelease(false == metaClass->IsAbstract());
    MetaObject* const dst = metaClass->CreateInstance();
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
String ToString(const RTTI::MetaObject& object) {
    const RTTI::MetaClass *metaclass = object.RTTI_MetaClass();
    Assert(metaclass);
    return StringFormat("@{0} : {1} = \"{2}\"", &object, metaclass->Name(), object.RTTI_Name());
}
//----------------------------------------------------------------------------
String ToString(const RTTI::PMetaObject& pobject) {
    return (pobject) ? ToString(*pobject) : String();
}
//----------------------------------------------------------------------------
String ToString(const RTTI::PCMetaObject& pobject) {
    return (pobject) ? ToString(*pobject) : String();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
