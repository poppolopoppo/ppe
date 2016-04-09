#include "stdafx.h"

#include "DynamicObject.h"

#include "MetaClassDatabase.h"
#include "MetaClassSingleton.h"
#include "MetaProperty.h"

#include "Core/Allocator/PoolAllocator-impl.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(RTTI, DynamicProperty, )
//----------------------------------------------------------------------------
DynamicProperty::DynamicProperty(const MetaPropertyName& name)
    : MetaProperty(name, MetaProperty::Dynamic) {}
//----------------------------------------------------------------------------
DynamicProperty::~DynamicProperty() {}/*
//----------------------------------------------------------------------------
MetaTypeInfo DynamicProperty::TypeInfo() const {

}
//----------------------------------------------------------------------------
const IMetaTypeVirtualTraits *DynamicProperty::Traits() const {

}
//----------------------------------------------------------------------------
bool DynamicProperty::IsDefaultValue(const MetaObject *object) const {

}
//----------------------------------------------------------------------------
MetaAtom *DynamicProperty::WrapMove(MetaObject *src) const {

}
//----------------------------------------------------------------------------
MetaAtom *DynamicProperty::WrapCopy(const MetaObject *src) const {

}
//----------------------------------------------------------------------------
bool DynamicProperty::UnwrapMove(MetaObject *dst, MetaAtom *src) const {

}
//----------------------------------------------------------------------------
bool DynamicProperty::UnwrapCopy(MetaObject *dst, const MetaAtom *src) const {

}
//----------------------------------------------------------------------------
void DynamicProperty::MoveTo(MetaObject *object, MetaAtom *atom) const {

}
//----------------------------------------------------------------------------
void DynamicProperty::CopyTo(const MetaObject *object, MetaAtom *atom) const {}
//----------------------------------------------------------------------------
void DynamicProperty::MoveFrom(MetaObject *object, MetaAtom *atom) const {}
//----------------------------------------------------------------------------
void DynamicProperty::CopyFrom(MetaObject *object, const MetaAtom *atom) const {}
//----------------------------------------------------------------------------
void DynamicProperty::Move(MetaObject *dst, MetaObject *src) const {}
//----------------------------------------------------------------------------
void DynamicProperty::Copy(MetaObject *dst, const MetaObject *src) const {}
//----------------------------------------------------------------------------
void DynamicProperty::Swap(MetaObject *lhs, MetaObject *rhs) const {}
//----------------------------------------------------------------------------
bool DynamicProperty::Equals(const MetaObject *lhs, const MetaObject *rhs) const {}
//----------------------------------------------------------------------------
bool DynamicProperty::DeepEquals(const MetaObject *lhs, const MetaObject *rhs) const {}
//----------------------------------------------------------------------------
void *DynamicProperty::RawPtr(MetaObject *obj) const {}
//----------------------------------------------------------------------------
const void *DynamicProperty::RawPtr(const MetaObject *obj) const {}
//----------------------------------------------------------------------------
size_t DynamicProperty::HashValue(const MetaObject *object) const {} */
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(RTTI, DynamicObject, )
//----------------------------------------------------------------------------
DynamicObject::DynamicObject() {}
//----------------------------------------------------------------------------
DynamicObject::~DynamicObject() {}
//----------------------------------------------------------------------------
MetaAtom* DynamicObject::GetValue(const MetaPropertyName& name) {
    return _values[name].get();
}
//----------------------------------------------------------------------------
const MetaAtom* DynamicObject::GetValue(const MetaPropertyName& name) const {
    return _values[name].get();
}
//----------------------------------------------------------------------------
MetaAtom* DynamicObject::TryGetValue(const MetaPropertyName& name) {
    PMetaAtom value;
    return (_values.TryGet(name, &value) ? value.get() : nullptr);
}
//----------------------------------------------------------------------------
const MetaAtom* DynamicObject::TryGetValue(const MetaPropertyName& name) const {
    PMetaAtom value;
    return (_values.TryGet(name, &value) ? value.get() : nullptr);
}
//----------------------------------------------------------------------------
void DynamicObject::SetValue(const MetaPropertyName& name, const PMetaAtom& value) {
    _values[name] = value;
}
//----------------------------------------------------------------------------
void DynamicObject::ClearValues() {
    _values.clear();
}
//----------------------------------------------------------------------------
const RTTI::MetaClass *DynamicObject::RTTI_MetaClass() const {
    return &_metaClass;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DynamicObject::MetaClass::MetaClass()
:   RTTI::InScopeMetaClass("DynamicObject", RTTI::MetaClass::Dynamic)
{}
//----------------------------------------------------------------------------
DynamicObject::MetaClass::~MetaClass() {}
//----------------------------------------------------------------------------
const RTTI::MetaClass* DynamicObject::MetaClass::VirtualParent() const {
    return nullptr;
}
//----------------------------------------------------------------------------
MetaObject* DynamicObject::MetaClass::VirtualCreateInstance() const {
    return new DynamicObject();
}
//----------------------------------------------------------------------------
void DynamicObject::MetaClass::Create() {
    Core::RTTI::MetaClassSingleton< DynamicObject >::Create();
}
//----------------------------------------------------------------------------
void DynamicObject::MetaClass::Destroy() {
    Core::RTTI::MetaClassSingleton< DynamicObject >::Destroy();
}
//----------------------------------------------------------------------------
bool DynamicObject::MetaClass::HasInstance() {
    return Core::RTTI::MetaClassSingleton< DynamicObject >::HasInstance();
}
//----------------------------------------------------------------------------
const DynamicObject::MetaClass *DynamicObject::MetaClass::Instance() {
    return &Core::RTTI::MetaClassSingleton< DynamicObject >::Instance();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
