#pragma once

#include "Core.RTTI/RTTI_Tag.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FMetaClassDeclList {
    mutable FAtomicSpinLock Lock;
    INTRUSIVELIST(&FMetaClassBaseDecl::Node) Decls;

    void Insert(const FMetaClassBaseDecl* pdecl);
    void Remove(const FMetaClassBaseDecl* pdecl);

    void Start() const;
    void Shutdown() const;
};
//----------------------------------------------------------------------------
#define RTTI_TAG_DEF(_Name) \
    namespace RTTI_Tag { \
        static Core::RTTI::FMetaClassDeclList CONCAT(gList_, _Name); \
        \
        void _Name::Register(const Core::RTTI::FMetaClassBaseDecl* pdecl) { CONCAT(gList_, _Name).Insert(pdecl); } \
        void _Name::Unregister(const Core::RTTI::FMetaClassBaseDecl* pdecl) { CONCAT(gList_, _Name).Remove(pdecl); } \
        \
        void _Name::Start() { CONCAT(gList_, _Name).Start(); } \
        void _Name::Shutdown() { CONCAT(gList_, _Name).Shutdown(); } \
    }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
