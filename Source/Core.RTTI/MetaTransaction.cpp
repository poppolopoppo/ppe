#include "stdafx.h"

#include "MetaTransaction.h"

#include "MetaAtom.h"
#include "MetaAtomDatabase.h"
#include "MetaClass.h"
#include "MetaClassDatabase.h"
#include "MetaObject.h"
#include "MetaObjectHelpers.h"
#include "MetaType.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
MetaTransaction::MetaTransaction()
:   _loaded(false)
,   _unloaded(true) {}
//----------------------------------------------------------------------------
MetaTransaction::MetaTransaction(VECTOR(RTTI, PMetaObject)&& objects)
:   _objects(std::move(objects))
,   _loaded(false)
,   _unloaded(true) {}
//----------------------------------------------------------------------------
MetaTransaction::~MetaTransaction() {
    Assert(false == _loaded);
    Assert(_unloaded);
}
//----------------------------------------------------------------------------
void MetaTransaction::Add(MetaObject* object) {
    Assert(false == _loaded);
    Assert(true == _unloaded);

    Assert(object);
    Assert(false == object->RTTI_IsLoaded());
    Assert(false == object->RTTI_IsUnloaded());

    RTTI::PMetaObject o(object);
    Add_AssertUnique(_objects, std::move(o));
}
//----------------------------------------------------------------------------
void MetaTransaction::Remove(MetaObject* object) {
    Assert(false == _loaded);
    Assert(true == _unloaded);

    Assert(object);
    Assert(false == object->RTTI_IsLoaded());
    Assert(false == object->RTTI_IsUnloaded());

    RTTI::PMetaObject o(object);
    Remove_AssertExists(_objects, o);
}
//----------------------------------------------------------------------------
bool MetaTransaction::Contains(const MetaObject* object) const {
    Assert(object);
    for (const PMetaObject& o : _objects)
        if (o.get() == object)
            return true;
    return false;
}
//----------------------------------------------------------------------------
void MetaTransaction::Load(MetaLoadContext *context) {
    Assert(false == _loaded);
    Assert(true == _unloaded);

    _unloaded = false;

    MetaAtomHashMap& db = MetaAtomDatabase::Instance();

    for (const PMetaObject& o : _objects) {
        o->RTTI_Load(context);

        if (o->RTTI_IsExported())
            db.Add(o.get());
    }

    _loaded = true;
}
//----------------------------------------------------------------------------
void MetaTransaction::Unload(MetaUnloadContext *context) {
    Assert(true == _loaded);
    Assert(false == _unloaded);

    _loaded = false;

    MetaAtomHashMap& db = MetaAtomDatabase::Instance();

    for (const PMetaObject& o : _objects) {
        if (o->RTTI_IsExported())
            db.Remove(o.get());

        o->RTTI_Unload(context);
    }

    _unloaded = true;
}
//----------------------------------------------------------------------------
bool MetaTransaction::Equals(const MetaTransaction& other) const {
    return (_objects == other._objects);
}
//----------------------------------------------------------------------------
bool MetaTransaction::DeepEquals(const MetaTransaction& other) const {
    if (_objects.size() != other._objects.size())
        return false;

    forrange(i, 0, _objects.size()) {
        const RTTI::PMetaObject& lhs = _objects[i];
        const RTTI::PMetaObject& rhs = other._objects[i];
        if (nullptr == lhs || nullptr == rhs) {
            if ((nullptr == lhs) != (nullptr == rhs))
                return false;
        }
        else if (false == RTTI::DeepEquals(*lhs, *rhs))
            return false;
    }
    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
