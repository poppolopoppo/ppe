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

    for (const UCMetaProperty& prop : metaClass->Properties())
        if (false == prop->Equals(&lhs, &rhs))
            return false;

    return true;
}
//----------------------------------------------------------------------------
bool DeepEquals(const MetaObject& lhs, const MetaObject& rhs) {
    if (&lhs == &rhs)
        return true;

    const MetaClass *metaClass = lhs.RTTI_MetaClass();
    Assert(metaClass);

    if (rhs.RTTI_MetaClass() != metaClass)
        return false;

    for (const UCMetaProperty& prop : metaClass->Properties())
        if (false == prop->DeepEquals(&lhs, &rhs))
            return false;

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
hash_t hash_value(const MetaObject& object) {
    const MetaClass *metaClass = object.RTTI_MetaClass();
    Assert(metaClass);

    hash_t h(CORE_HASH_VALUE_SEED);
    for (const UCMetaProperty& prop : metaClass->Properties())
        hash_combine(h, hash_t(prop->HashValue(&object)));

    return h;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void Move(MetaObject& dst, MetaObject& src) {
    const MetaClass *metaClass = src.RTTI_MetaClass();
    Assert(metaClass);
    Assert(dst.RTTI_MetaClass() == metaClass);

    for (const UCMetaProperty& prop : metaClass->Properties())
        if (prop->IsWritable())
            prop->Move(&dst, &src);
}
//----------------------------------------------------------------------------
void Copy(MetaObject& dst, const MetaObject& src) {
    const MetaClass *metaClass = src.RTTI_MetaClass();
    Assert(metaClass);
    Assert(dst.RTTI_MetaClass() == metaClass);

    for (const UCMetaProperty& prop : metaClass->Properties())
        if (prop->IsWritable())
            prop->Copy(&dst, &src);
}
//----------------------------------------------------------------------------
void Swap(MetaObject& lhs, MetaObject& rhs) {
    const MetaClass *metaClass = lhs.RTTI_MetaClass();
    Assert(metaClass);
    Assert(rhs.RTTI_MetaClass() == metaClass);

    for (const UCMetaProperty& prop : metaClass->Properties())
        if (prop->IsWritable())
            prop->Swap(&lhs, &rhs);
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
String ToString(const MetaObject& object) {
    const MetaClass *metaclass = object.RTTI_MetaClass();
    Assert(metaclass);
    return StringFormat("@{0} : {1} = \"{2}\"", &object, metaclass->Name(), object.RTTI_Name());
}
//----------------------------------------------------------------------------
String ToString(const PCMetaObject& pobject) {
    return (pobject) ? ToString(*pobject) : String();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
