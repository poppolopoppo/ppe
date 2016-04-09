#include "stdafx.h"

#include "RTTI_Tag-impl.h"

#ifdef OS_WINDOWS
#   pragma warning(disable: 4073) // initialiseurs placés dans la zone d'initialisation d'une bibliothèque
#   pragma init_seg(lib)
#else
#   error "missing compiler specific command"
#endif

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
RTTI_TAG_DEF(Default);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void MetaClassDeclList::Insert(const MetaClassBaseDecl* pdecl) {
    Assert(pdecl);
    const AtomicSpinLock::Scope scopeLock(Lock);
    Decls.PushFront(const_cast<MetaClassBaseDecl*>(pdecl));
}
//----------------------------------------------------------------------------
void MetaClassDeclList::Remove(const MetaClassBaseDecl* pdecl) {
    Assert(pdecl);
    const AtomicSpinLock::Scope scopeLock(Lock);
    Decls.Erase(const_cast<MetaClassBaseDecl*>(pdecl));
}
//----------------------------------------------------------------------------
void MetaClassDeclList::Start() const {
    const AtomicSpinLock::Scope scopeLock(Lock);
    typedef INTRUSIVELIST_ACCESSOR(&MetaClassBaseDecl::Node) accessor_type;
    for (MetaClassBaseDecl* node = Decls.Tail(); node; node = accessor_type::Prev(node))
        node->Create();
}
//----------------------------------------------------------------------------
void MetaClassDeclList::Shutdown() const {
    const AtomicSpinLock::Scope scopeLock(Lock);
    typedef INTRUSIVELIST_ACCESSOR(&MetaClassBaseDecl::Node) accessor_type;
    for (MetaClassBaseDecl* node = Decls.Head(); node; node = accessor_type::Next(node))
        node->Destroy();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
