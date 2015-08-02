#include "stdafx.h"

#include "MetaObject.h"

#include "Atom/MetaAtom.h"
#include "Atom/MetaAtomDatabase.h"
#include "Class/MetaClass.h"
#include "Property/MetaProperty.h"

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
