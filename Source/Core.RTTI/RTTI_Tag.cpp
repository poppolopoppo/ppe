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
void FMetaClassDeclList::Insert(const FMetaClassBaseDecl* pdecl) {
    Assert(pdecl);
    const FAtomicSpinLock::FScope scopeLock(Lock);
    Decls.PushFront(const_cast<FMetaClassBaseDecl*>(pdecl));
}
//----------------------------------------------------------------------------
void FMetaClassDeclList::Remove(const FMetaClassBaseDecl* pdecl) {
    Assert(pdecl);
    const FAtomicSpinLock::FScope scopeLock(Lock);
    Decls.Erase(const_cast<FMetaClassBaseDecl*>(pdecl));
}
//----------------------------------------------------------------------------
void FMetaClassDeclList::Start() const {
    const FAtomicSpinLock::FScope scopeLock(Lock);
    typedef INTRUSIVELIST_ACCESSOR(&FMetaClassBaseDecl::Node) accessor_type;
    for (FMetaClassBaseDecl* node = Decls.Tail(); node; node = accessor_type::Prev(node))
        node->Create();
}
//----------------------------------------------------------------------------
void FMetaClassDeclList::Shutdown() const {
    const FAtomicSpinLock::FScope scopeLock(Lock);
    typedef INTRUSIVELIST_ACCESSOR(&FMetaClassBaseDecl::Node) accessor_type;
    for (FMetaClassBaseDecl* node = Decls.Head(); node; node = accessor_type::Next(node))
        node->Destroy();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
