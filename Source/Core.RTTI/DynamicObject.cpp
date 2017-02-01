#include "stdafx.h"

#include "DynamicObject.h"

#include "MetaClassDatabase.h"
#include "MetaClassSingleton.h"
#include "MetaProperty.h"
#include "MetaType.h"

#include "Core/Allocator/PoolAllocator-impl.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(RTTI, FDynamicProperty, )
//----------------------------------------------------------------------------
FDynamicProperty::FDynamicProperty(const FName& name)
    : FMetaProperty(name, FMetaProperty::Dynamic) {}
//----------------------------------------------------------------------------
FDynamicProperty::~FDynamicProperty() {}/*
//----------------------------------------------------------------------------
FMetaTypeInfo FDynamicProperty::TypeInfo() const {

}
//----------------------------------------------------------------------------
const IMetaTypeVirtualTraits *FDynamicProperty::Traits() const {

}
//----------------------------------------------------------------------------
bool FDynamicProperty::IsDefaultValue(const FMetaObject *object) const {

}
//----------------------------------------------------------------------------
FMetaAtom *FDynamicProperty::WrapMove(FMetaObject *src) const {

}
//----------------------------------------------------------------------------
FMetaAtom *FDynamicProperty::WrapCopy(const FMetaObject *src) const {

}
//----------------------------------------------------------------------------
bool FDynamicProperty::UnwrapMove(FMetaObject *dst, FMetaAtom *src) const {

}
//----------------------------------------------------------------------------
bool FDynamicProperty::UnwrapCopy(FMetaObject *dst, const FMetaAtom *src) const {

}
//----------------------------------------------------------------------------
void FDynamicProperty::MoveTo(FMetaObject *object, FMetaAtom *atom) const {

}
//----------------------------------------------------------------------------
void FDynamicProperty::CopyTo(const FMetaObject *object, FMetaAtom *atom) const {}
//----------------------------------------------------------------------------
void FDynamicProperty::MoveFrom(FMetaObject *object, FMetaAtom *atom) const {}
//----------------------------------------------------------------------------
void FDynamicProperty::CopyFrom(FMetaObject *object, const FMetaAtom *atom) const {}
//----------------------------------------------------------------------------
void FDynamicProperty::Move(FMetaObject *dst, FMetaObject *src) const {}
//----------------------------------------------------------------------------
void FDynamicProperty::Copy(FMetaObject *dst, const FMetaObject *src) const {}
//----------------------------------------------------------------------------
void FDynamicProperty::Swap(FMetaObject *lhs, FMetaObject *rhs) const {}
//----------------------------------------------------------------------------
bool FDynamicProperty::Equals(const FMetaObject *lhs, const FMetaObject *rhs) const {}
//----------------------------------------------------------------------------
bool FDynamicProperty::DeepEquals(const FMetaObject *lhs, const FMetaObject *rhs) const {}
//----------------------------------------------------------------------------
void *FDynamicProperty::RawPtr(FMetaObject *obj) const {}
//----------------------------------------------------------------------------
const void *FDynamicProperty::RawPtr(const FMetaObject *obj) const {}
//----------------------------------------------------------------------------
size_t FDynamicProperty::HashValue(const FMetaObject *object) const {} */
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(RTTI, FDynamicObject, )
//----------------------------------------------------------------------------
FDynamicObject::FDynamicObject() {}
//----------------------------------------------------------------------------
FDynamicObject::~FDynamicObject() {}
//----------------------------------------------------------------------------
FMetaAtom* FDynamicObject::GetValue(const FName& name) {
    return _values[name].get();
}
//----------------------------------------------------------------------------
const FMetaAtom* FDynamicObject::GetValue(const FName& name) const {
    return _values[name].get();
}
//----------------------------------------------------------------------------
FMetaAtom* FDynamicObject::TryGetValue(const FName& name) {
    PMetaAtom value;
    return (_values.TryGet(name, &value) ? value.get() : nullptr);
}
//----------------------------------------------------------------------------
const FMetaAtom* FDynamicObject::TryGetValue(const FName& name) const {
    PMetaAtom value;
    return (_values.TryGet(name, &value) ? value.get() : nullptr);
}
//----------------------------------------------------------------------------
void FDynamicObject::SetValue(const FName& name, const PMetaAtom& value) {
    _values[name] = value;
}
//----------------------------------------------------------------------------
void FDynamicObject::ClearValues() {
    _values.clear();
}
//----------------------------------------------------------------------------
const RTTI::FMetaClass *FDynamicObject::RTTI_MetaClass() const {
    return &_metaClass;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FDynamicObject::FMetaClass::FMetaClass()
:   RTTI::FInScopeMetaClass(FName("FDynamicObject"), RTTI::FMetaClass::Dynamic)
{}
//----------------------------------------------------------------------------
FDynamicObject::FMetaClass::~FMetaClass() {}
//----------------------------------------------------------------------------
const RTTI::FMetaClass* FDynamicObject::FMetaClass::VirtualParent() const {
    return nullptr;
}
//----------------------------------------------------------------------------
FMetaObject* FDynamicObject::FMetaClass::VirtualCreateInstance() const {
    return new FDynamicObject();
}
//----------------------------------------------------------------------------
void FDynamicObject::FMetaClass::Create() {
    Core::RTTI::TMetaClassSingleton< FDynamicObject >::Create();
}
//----------------------------------------------------------------------------
void FDynamicObject::FMetaClass::Destroy() {
    Core::RTTI::TMetaClassSingleton< FDynamicObject >::Destroy();
}
//----------------------------------------------------------------------------
bool FDynamicObject::FMetaClass::HasInstance() {
    return Core::RTTI::TMetaClassSingleton< FDynamicObject >::HasInstance();
}
//----------------------------------------------------------------------------
const FDynamicObject::FMetaClass *FDynamicObject::FMetaClass::Instance() {
    return &Core::RTTI::TMetaClassSingleton< FDynamicObject >::Instance();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
