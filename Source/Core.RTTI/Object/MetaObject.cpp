#include "stdafx.h"

#include "MetaObject.h"

#include "RTTI/Atom/MetaAtom.h"
#include "RTTI/Atom/MetaAtomDatabase.h"
#include "RTTI/Class/MetaClass.h"
#include "RTTI/Property/MetaProperty.h"

#include "Container/Hash.h"
#include "Container/HashMap.h"
#include "Container/Token.h"
#include "Container/Vector.h"

#include "IO/Format.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
MetaObject::MetaObject()
:   _state(None) {}
//----------------------------------------------------------------------------
MetaObject::~MetaObject() {}
//----------------------------------------------------------------------------
void MetaObject::RTTI_Export(const MetaObjectName& name) {
    Assert(!name.empty());
    Assert(_name.empty());

    _name = name;

    MetaAtomDatabase::Instance().Add(this);
}
//----------------------------------------------------------------------------
void MetaObject::RTTI_Unexport() {
    Assert(!_name.empty());

    MetaAtomDatabase::Instance().Remove(this);

    _name = MetaObjectName();
    Assert(_name.empty());
}
//----------------------------------------------------------------------------
void MetaObject::RTTI_Load(MetaLoadContext * /* context */) {
    Assert(0 == (_state & Loaded));
}
//----------------------------------------------------------------------------
void MetaObject::RTTI_Unload(MetaUnloadContext * /* context */) {
    Assert(0 == (_state & Unloaded));
}
//----------------------------------------------------------------------------
void MetaObject::RTTI_CallLoadIFN(MetaLoadContext *context) {
    if (0 == (_state & Loaded)) {
        RTTI_Load(context);
        Assert(Loaded == (_state & Loaded));
    }
}
//----------------------------------------------------------------------------
void MetaObject::RTTI_CallUnloadIFN(MetaUnloadContext *context) {
    Assert(Loaded == (_state & Loaded));
    if (0 == (_state & Unloaded)) {
        RTTI_Unload(context);
        Assert(Unloaded == (_state & Unloaded));
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
