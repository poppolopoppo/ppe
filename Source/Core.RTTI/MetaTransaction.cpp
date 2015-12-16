#include "stdafx.h"

#include "MetaTransaction.h"

#include "MetaAtom.h"
#include "MetaAtomDatabase.h"
#include "MetaClass.h"
#include "MetaClassName.h"
#include "MetaClassDatabase.h"
#include "MetaObject.h"
#include "MetaObjectName.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
MetaTransaction::MetaTransaction()
:   _loaded(false)
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
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
