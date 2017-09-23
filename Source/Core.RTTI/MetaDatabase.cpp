#include "stdafx.h"

#include "MetaDatabase.h"

#include "MetaAtom.h"
#include "MetaClass.h"
#include "MetaNamespace.h"
#include "MetaObject.h"

#include "Core/Diagnostic/Logger.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static FMetaObject* CastAtomToObject_(const TPair<const FName, PMetaAtom>& it) {
    const auto* pObjectAtom = it.second->As<PMetaObject>();
    return (pObjectAtom ? pObjectAtom->Wrapper().get() : nullptr);
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FMetaDatabase::FMetaDatabase() {}
//----------------------------------------------------------------------------
FMetaDatabase::~FMetaDatabase() {
    WRITESCOPELOCK(_barrier);
    Assert(_namespaces.empty());
    _atoms.clear();
}
//----------------------------------------------------------------------------
void FMetaDatabase::RegisterNamespace(const FMetaNamespace* metaNamespace) {
    Assert(metaNamespace);

    LOG(Info, L"[RTTI] Register namespace <{0}>", metaNamespace->Name());

    WRITESCOPELOCK(_barrier);
    Insert_AssertUnique(_namespaces, metaNamespace->Name(), metaNamespace);
}
//----------------------------------------------------------------------------
void FMetaDatabase::UnregisterNamespace(const FMetaNamespace* metaNamespace) {
    Assert(metaNamespace);

    LOG(Info, L"[RTTI] Unregister namespace <{0}>", metaNamespace->Name());

    WRITESCOPELOCK(_barrier);
    Remove_AssertExistsAndSameValue(_namespaces, metaNamespace->Name(), metaNamespace);
}
//----------------------------------------------------------------------------
const FMetaClass* FMetaDatabase::FindClass(const FName& name) const {
    Assert(not name.empty());

    READSCOPELOCK(_barrier);
    for (const auto& it : _namespaces) {
        if (const FMetaClass* metaClass = it.second->FindClass(name))
            return metaClass;
    }
    AssertNotReached();
    return nullptr;
}
//----------------------------------------------------------------------------
const FMetaClass* FMetaDatabase::FindClassIFP(const FName& name) const {
    Assert(not name.empty());

    READSCOPELOCK(_barrier);
    for (const auto& it : _namespaces) {
        if (const FMetaClass* metaClass = it.second->FindClassIFP(name))
            return metaClass;
    }
    return nullptr;
}
//----------------------------------------------------------------------------
void FMetaDatabase::AllClasses(TCollector<const FMetaClass*>& instances) const {
    READSCOPELOCK(_barrier);
    for (const auto& it : _namespaces) {
        it.second->AllClasses(instances);
    }
}
//----------------------------------------------------------------------------
const FMetaNamespace* FMetaDatabase::FindNamespace(const FName& name) const {
    Assert(not name.empty());

    READSCOPELOCK(_barrier);
    return _namespaces.at(name);
}
//----------------------------------------------------------------------------
const FMetaNamespace* FMetaDatabase::FindNamespaceIFP(const FName& name) const {
    Assert(not name.empty());

    READSCOPELOCK(_barrier);
    const auto it = _namespaces.find(name);
    return (_namespaces.end() == it ? nullptr : it->second);
}
//----------------------------------------------------------------------------
void FMetaDatabase::AllNamespaces(TCollector<const FMetaNamespace*>& instances) const {
    READSCOPELOCK(_barrier);
    instances.assign(
        MakeValueIterator(_namespaces.begin()),
        MakeValueIterator(_namespaces.end()) );
}
//----------------------------------------------------------------------------
void FMetaDatabase::RegisterAtom(const FName& name, FMetaAtom* metaAtom, bool allowOverride) {
    Assert(not name.empty());
    Assert(metaAtom);

    LOG(Info, L"[RTTI] Register atom '{0}' <{1}>", name, metaAtom->TypeInfo().Name);

    WRITESCOPELOCK(_barrier);
    PMetaAtom& dst = _atoms[name];
    AssertRelease(allowOverride || not dst);
    dst = metaAtom;
}
//----------------------------------------------------------------------------
void FMetaDatabase::UnregisterAtom(const FName& name, FMetaAtom* metaAtom) {
    Assert(not name.empty());
    Assert(metaAtom);

    LOG(Info, L"[RTTI] Unregister atom '{0}' <{1}>", name, metaAtom->TypeInfo().Name);

    WRITESCOPELOCK(_barrier);
    const PMetaAtom scopeAtom(metaAtom);
    Remove_AssertExistsAndSameValue(_atoms, name, scopeAtom);
}
//----------------------------------------------------------------------------
FMetaAtom* FMetaDatabase::FindAtom(const FName& name) const {
    Assert(not name.empty());

    READSCOPELOCK(_barrier);
    return _atoms.at(name).get();
}
//----------------------------------------------------------------------------
FMetaAtom* FMetaDatabase::FindAtomIFP(const FName& name) const {
    Assert(not name.empty());

    READSCOPELOCK(_barrier);
    const auto it = _atoms.find(name);
    return (it != _atoms.end() ? it->second.get() : nullptr);
}
//----------------------------------------------------------------------------
void FMetaDatabase::AllAtoms(TCollector<PMetaAtom>& instances) const {
    READSCOPELOCK(_barrier);
    instances.assign(
        MakeValueIterator(_atoms.begin()),
        MakeValueIterator(_atoms.end()) );
}
//----------------------------------------------------------------------------
void FMetaDatabase::RegisterObject(FMetaObject* object) {
    Assert(object);
    Assert(FindClassIFP(object->RTTI_MetaClass()->Name()));

    PMetaObject pObject(object);
    RegisterAtom(pObject->RTTI_Name(), MakeAtom(std::move(pObject)), false);
}
//----------------------------------------------------------------------------
void FMetaDatabase::UnregisterObject(FMetaObject* object) {
    Assert(object);
    Assert(FindClassIFP(object->RTTI_MetaClass()->Name()));

    ONLY_IF_ASSERT(PMetaAtom atom);
    const FName name = object->RTTI_Name();
    Assert(not name.empty());

    LOG(Info, L"[RTTI] Unregister atom '{0}' <{1}>", name, TypeInfo<PMetaObject>().Name);
    {
        WRITESCOPELOCK(_barrier);
#ifdef WITH_CORE_ASSERT
        atom = Remove_ReturnValue(_atoms, name);
#else
        Remove_AssertExists(_atoms, name);
#endif
    }

    Assert_NoAssume(atom);
    Assert_NoAssume(atom->Cast<PMetaObject>()->Wrapper() == object);
}
//----------------------------------------------------------------------------
FMetaObject* FMetaDatabase::FindObject(const FName& name) const {
    Assert(not name.empty());

    const FMetaAtom* pAtom = FindAtom(name);
    return pAtom->Cast<PMetaObject>()->Wrapper().get();
}
//----------------------------------------------------------------------------
FMetaObject* FMetaDatabase::FindObjectIFP(const FName& name) const {
    Assert(not name.empty());

    const FMetaAtom* pAtom = FindAtomIFP(name);
    return (pAtom ? pAtom->Cast<PMetaObject>()->Wrapper().get() : nullptr);
}
//----------------------------------------------------------------------------
void FMetaDatabase::AllObjects(TCollector<PMetaObject>& instances) const {
    READSCOPELOCK(_barrier);

    forrange(it,
        MakeOutputIterator(_atoms.begin(), &CastAtomToObject_),
        MakeOutputIterator(_atoms.end(), &CastAtomToObject_) ) {
        if (*it != nullptr)
            instances.emplace_back(*it);
    }
}
//----------------------------------------------------------------------------
void FMetaDatabase::FindObjectsByClass(const FMetaClass* metaClass, TCollector<PMetaObject>& instances) const {
    Assert(metaClass);

    READSCOPELOCK(_barrier);

    forrange(it,
        MakeOutputIterator(_atoms.begin(), &CastAtomToObject_),
        MakeOutputIterator(_atoms.end(), &CastAtomToObject_) ) {
        if (*it != nullptr && (*it)->RTTI_MetaClass() == metaClass)
            instances.emplace_back(*it);
    }
}
//----------------------------------------------------------------------------
void FMetaDatabase::FindObjectsInheritingClass(const FMetaClass* metaClass, TCollector<PMetaObject>& instances) const {
    Assert(metaClass);

    READSCOPELOCK(_barrier);

    forrange(it,
        MakeOutputIterator(_atoms.begin(), &CastAtomToObject_),
        MakeOutputIterator(_atoms.end(), &CastAtomToObject_)) {
        if (*it != nullptr && (*it)->RTTI_MetaClass()->InheritsFrom(metaClass))
            instances.emplace_back(*it);
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
