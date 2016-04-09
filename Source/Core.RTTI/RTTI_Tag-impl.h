#pragma once

#include "Core.RTTI/RTTI_Tag.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct MetaClassDeclList {
    mutable AtomicSpinLock Lock;
    INTRUSIVELIST(&MetaClassBaseDecl::Node) Decls;

    void Insert(const MetaClassBaseDecl* pdecl);
    void Remove(const MetaClassBaseDecl* pdecl);

    void Start() const;
    void Shutdown() const;
};
//----------------------------------------------------------------------------
#define RTTI_TAG_DEF(_Name) \
    namespace RTTI_Tag { \
        static Core::RTTI::MetaClassDeclList CONCAT(gList_, _Name); \
        \
        void _Name::Register(const Core::RTTI::MetaClassBaseDecl* pdecl) { CONCAT(gList_, _Name).Insert(pdecl); } \
        void _Name::Unregister(const Core::RTTI::MetaClassBaseDecl* pdecl) { CONCAT(gList_, _Name).Remove(pdecl); } \
        \
        void _Name::Start() { CONCAT(gList_, _Name).Start(); } \
        void _Name::Shutdown() { CONCAT(gList_, _Name).Shutdown(); } \
    }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
