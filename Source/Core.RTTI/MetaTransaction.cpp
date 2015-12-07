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
MetaTransaction::MetaTransaction() {}
//----------------------------------------------------------------------------
MetaTransaction::~MetaTransaction() {
    Clear();
}
//----------------------------------------------------------------------------
void MetaTransaction::Export(const MetaClassName& name, const MetaClass *metaclass, bool allowOverride) {
    Assert(metaclass);
    const MetaClass *old = MetaClassDatabase::Instance().GetIFP(name);
    MetaClassDatabase::Instance().Add(name, metaclass, allowOverride);
    Insert_AssertUnique(_classes, name, MetaClassBinding{ old, metaclass });
}
//----------------------------------------------------------------------------
const MetaClass* MetaTransaction::GetIFP(const MetaClassName& name) const {
    const auto it = _classes.find(name);
    return (_classes.end() != it) ? it->second.New : nullptr;
}
//----------------------------------------------------------------------------
void MetaTransaction::Remove(const MetaClassName& name, const MetaClass* metaclass) {
    Assert(metaclass);
    const MetaClassBinding binding = Remove_ReturnValue(_classes, name);
    Assert(metaclass == binding.New);
    RevertBinding_(MetaClassDatabase::Instance(), name, binding);
}
//----------------------------------------------------------------------------
void MetaTransaction::Export(const MetaObjectName& name, MetaAtom *atom, bool allowOverride) {
    Assert(atom);
    const PMetaAtom old = MetaAtomDatabase::Instance().GetIFP(name);
    MetaAtomDatabase::Instance().Add(name, atom, allowOverride);
    Insert_AssertUnique(_atoms, name, MetaAtomBinding{ old, atom });
}
//----------------------------------------------------------------------------
void MetaTransaction::Export(const MetaObjectName& name, MetaObject* object, bool allowOverride) {
    const PMetaAtom atom = MakeAtom(PMetaObject(object));
    Export(name, atom.get(), allowOverride);
}
//----------------------------------------------------------------------------
const MetaAtom* MetaTransaction::GetIFP(const MetaObjectName& name) const {
    const auto it = _atoms.find(name);
    return (_atoms.end() != it) ? it->second.New.get() : nullptr;
}
//----------------------------------------------------------------------------
void MetaTransaction::Remove(const MetaObjectName& name, MetaAtom* atom) {
    Assert(atom);
    const MetaAtomBinding binding = Remove_ReturnValue(_atoms, name);
    Assert(atom == binding.New);
    RevertBinding_(MetaAtomDatabase::Instance(), name, binding);
}
//----------------------------------------------------------------------------
void MetaTransaction::Clear() {
    MetaAtomHashMap& atomDB = MetaAtomDatabase::Instance();
    foreachitem(it, _atoms)
        RevertBinding_(atomDB, it->first, it->second);
    _atoms.clear();

    MetaClassHashMap& classDB = MetaClassDatabase::Instance();
    foreachitem(it, _classes)
        RevertBinding_(classDB, it->first, it->second);
    _classes.clear();
}
//----------------------------------------------------------------------------
void MetaTransaction::RevertBinding_(MetaAtomHashMap& atoms, const MetaObjectName& name, const MetaAtomBinding& binding) {
    if (binding.Old)
        atoms.Add(name, binding.Old.get(), true);
    else
        atoms.Remove(name, binding.New.get());
}
//----------------------------------------------------------------------------
void MetaTransaction::RevertBinding_(MetaClassHashMap& instances, const MetaClassName& name, const MetaClassBinding& binding) {
    if (binding.Old)
        instances.Add(name, binding.Old, true);
    else
        instances.Remove(name, binding.New);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
