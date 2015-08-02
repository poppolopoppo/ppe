#include "stdafx.h"

#include "MetaTransaction.h"

#include "Atom/MetaAtom.h"
#include "Atom/MetaAtomDatabase.h"
#include "Class/MetaClass.h"
#include "Class/MetaClassDatabase.h"
#include "Object/MetaObject.h"

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
    const MetaClass *old = MetaClassDatabase::Instance().GetIFP(name);
    MetaClassDatabase::Instance().Add(name, metaclass, allowOverride);

    _classes.push_back(MetaClassBinding{ name, old, metaclass });
}
//----------------------------------------------------------------------------
void MetaTransaction::Remove(const MetaClassName& name, const MetaClass * /* metaclass */) {
    const auto it = std::find_if(_classes.begin(), _classes.end(), [&name](const MetaClassBinding& binding) -> bool {
        return binding.Name == name;
    });

    Assert(_classes.end() != it);

    MetaClassHashMap& classes = MetaClassDatabase::Instance();
    if (it->Old)
        classes.Add(it->Name, it->Old, true);
    else
        classes.Remove(it->Name, it->New);

    _classes.erase(it);
}
//----------------------------------------------------------------------------
void MetaTransaction::Export(const MetaObjectName& name, MetaAtom *atom, bool allowOverride) {
    const PMetaAtom old = MetaAtomDatabase::Instance().GetIFP(name);
    MetaAtomDatabase::Instance().Add(name, atom, allowOverride);

    _atoms.push_back(MetaAtomBinding{ name, old, atom });
}
//----------------------------------------------------------------------------
void MetaTransaction::Remove(const MetaObjectName& name, MetaAtom * /* atom */) {
    const auto it = std::find_if(_atoms.begin(), _atoms.end(), [&name](const MetaAtomBinding& binding) -> bool {
        return binding.Name == name;
    });

    Assert(_atoms.end() != it);

    MetaAtomHashMap& atoms = MetaAtomDatabase::Instance();
    if (it->Old)
        atoms.Add(it->Name, it->Old.get(), true);
    else
        atoms.Remove(it->Name, it->New.get());

    _atoms.erase(it);
}
//----------------------------------------------------------------------------
void MetaTransaction::Clear() {
    MetaAtomHashMap& atoms = MetaAtomDatabase::Instance();
    for (auto it = _atoms.rbegin(); _atoms.rend() != it; ++it) {
        if (it->Old)
            atoms.Add(it->Name, it->Old.get(), true);
        else
            atoms.Remove(it->Name, it->New.get());
    }
    _atoms.clear();

    MetaClassHashMap& classes = MetaClassDatabase::Instance();
    for (auto it = _classes.rbegin(); _classes.rend() != it; ++it) {
        if (it->Old)
            classes.Add(it->Name, it->Old, true);
        else
            classes.Remove(it->Name, it->New);
    }
    _classes.clear();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
