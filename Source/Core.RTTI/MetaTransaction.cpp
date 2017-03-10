#include "stdafx.h"

#include "MetaTransaction.h"

#include "MetaAtom.h"
#include "MetaClass.h"
#include "MetaDatabase.h"
#include "MetaObject.h"
#include "MetaObjectHelpers.h"
#include "MetaType.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FMetaTransaction::FMetaTransaction()
:   _loaded(false)
,   _unloaded(true) {}
//----------------------------------------------------------------------------
FMetaTransaction::FMetaTransaction(VECTOR(RTTI, PMetaObject)&& objects)
:   _objects(std::move(objects))
,   _loaded(false)
,   _unloaded(true) {}
//----------------------------------------------------------------------------
FMetaTransaction::~FMetaTransaction() {
    Assert(false == _loaded);
    Assert(_unloaded);
}
//----------------------------------------------------------------------------
void FMetaTransaction::Add(FMetaObject* object) {
    Assert(false == _loaded);
    Assert(true == _unloaded);

    Assert(object);
    Assert(false == object->RTTI_IsLoaded());
    Assert(false == object->RTTI_IsUnloaded());

    RTTI::PMetaObject o(object);
    Add_AssertUnique(_objects, std::move(o));
}
//----------------------------------------------------------------------------
void FMetaTransaction::Remove(FMetaObject* object) {
    Assert(false == _loaded);
    Assert(true == _unloaded);

    Assert(object);
    Assert(false == object->RTTI_IsLoaded());
    Assert(false == object->RTTI_IsUnloaded());

    RTTI::PMetaObject o(object);
    Remove_AssertExists(_objects, o);
}
//----------------------------------------------------------------------------
bool FMetaTransaction::Contains(const FMetaObject* object) const {
    Assert(object);
    for (const PMetaObject& o : _objects)
        if (o.get() == object)
            return true;
    return false;
}
//----------------------------------------------------------------------------
void FMetaTransaction::Load(FMetaLoadContext *context) {
    Assert(false == _loaded);
    Assert(true == _unloaded);

    _unloaded = false;

    FMetaDatabase& db = RTTI::MetaDB();

    for (const PMetaObject& o : _objects) {
        o->RTTI_Load(context);

        if (o->RTTI_IsExported())
            db.RegisterObject(o.get());
    }

    _loaded = true;
}
//----------------------------------------------------------------------------
void FMetaTransaction::Unload(FMetaUnloadContext *context) {
    Assert(true == _loaded);
    Assert(false == _unloaded);

    _loaded = false;

    FMetaDatabase& db = RTTI::MetaDB();

    for (const PMetaObject& o : _objects) {
        if (o->RTTI_IsExported())
            db.UnregisterObject(o.get());

        o->RTTI_Unload(context);
    }

    _unloaded = true;
}
//----------------------------------------------------------------------------
bool FMetaTransaction::Equals(const FMetaTransaction& other) const {
    return (_objects == other._objects);
}
//----------------------------------------------------------------------------
bool FMetaTransaction::DeepEquals(const FMetaTransaction& other) const {
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
