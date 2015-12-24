#include "stdafx.h"

#include "MetaObject.h"

#include "MetaAtom.h"
#include "MetaClass.h"
#include "MetaClassDatabase.h"
#include "MetaClassSingleton.h"
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
MetaObject::MetaClass::MetaClass()
:   RTTI::MetaClass("MetaObject", RTTI::MetaClass::Abstract, nullptr)
{}
//----------------------------------------------------------------------------
MetaObject::MetaClass::~MetaClass() {}
//----------------------------------------------------------------------------
void MetaObject::MetaClass::Create() {
    Core::RTTI::MetaClassSingleton< MetaObject >::Create();
}
//----------------------------------------------------------------------------
void MetaObject::MetaClass::Destroy() {
    Core::RTTI::MetaClassSingleton< MetaObject >::Destroy();
}
//----------------------------------------------------------------------------
bool MetaObject::MetaClass::HasInstance() {
    return Core::RTTI::MetaClassSingleton< MetaObject >::HasInstance();
}
//----------------------------------------------------------------------------
const MetaObject::MetaClass *MetaObject::MetaClass::Instance() {
    return &Core::RTTI::MetaClassSingleton< MetaObject >::Instance();
}
//----------------------------------------------------------------------------
Core::RTTI::MetaObject *MetaObject::MetaClass::VirtualCreateInstance() const {
    return new MetaObject();
}
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
    Assert(0 == (_state & Exported));

    _name = name;
    _state = Flags(_state | Exported);
}
//----------------------------------------------------------------------------
void MetaObject::RTTI_Unexport() {
    Assert(!_name.empty());
    Assert(Exported == (_state & Exported));

    _name = MetaObjectName();
    _state = Flags(_state & ~Exported);

    Assert(_name.empty());
}
//----------------------------------------------------------------------------
void MetaObject::RTTI_Load(MetaLoadContext * /* context */) {
    Assert(0 == (_state & Loaded));
    Assert(0 == (_state & Unloaded));
    _state = Flags(_state | Loaded);
}
//----------------------------------------------------------------------------
void MetaObject::RTTI_Unload(MetaUnloadContext * /* context */) {
    Assert(1 == (_state & Loaded));
    Assert(0 == (_state & Unloaded));
    _state = Flags((_state & ~Loaded) | Unloaded);
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
