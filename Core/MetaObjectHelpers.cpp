#include "stdafx.h"

#include "MetaObjectHelpers.h"

#include "HashMap.h"
#include "Token.h"
#include "Vector.h"

#include "MetaAtom.h"
#include "MetaAtomDatabase.h"

#include "MetaClass.h"
#include "MetaProperty.h"

#include "Format.h"
#include "Hash.h"

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

    for (const Pair<MetaPropertyName, const MetaProperty *>& prop : metaClass->Properties())
        if (!prop.second->Equals(&lhs, &rhs))
            return false;

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
size_t hash_value(const MetaObject& object) {
    const MetaClass *metaClass = object.RTTI_MetaClass();
    Assert(metaClass);

    using Core::hash_value;

    size_t h = 0;
    for (const Pair<MetaPropertyName, const MetaProperty *>& prop : metaClass->Properties())
        h = hash_value(h, prop.second->HashValue(&object));

    return h;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void Move(MetaObject& dst, MetaObject& src) {
    const MetaClass *metaClass = src.RTTI_MetaClass();
    Assert(metaClass);
    Assert(dst.RTTI_MetaClass() == metaClass);

    for (const Pair<MetaPropertyName, const MetaProperty *>& prop : metaClass->Properties())
        prop.second->Move(&dst, &src);
}
//----------------------------------------------------------------------------
void Copy(MetaObject& dst, const MetaObject& src) {
    const MetaClass *metaClass = src.RTTI_MetaClass();
    Assert(metaClass);
    Assert(dst.RTTI_MetaClass() == metaClass);

    for (const Pair<MetaPropertyName, const MetaProperty *>& prop : metaClass->Properties())
        prop.second->Copy(&dst, &src);
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
void DeepCopy(MetaObject& dst, const MetaObject& src) {
    // TODO (01/14) : MetaPropertyVisitor
    Assert(false);
}
//----------------------------------------------------------------------------
MetaObject *NewDeepCopy(const MetaObject& src) {
    // TODO (01/14) : MetaPropertyVisitor
    Assert(false);
    return nullptr;
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
