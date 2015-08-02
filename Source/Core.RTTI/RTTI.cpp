#include "stdafx.h"

#include "RTTI.h"

#include "Atom/MetaAtomDatabase.h"
#include "Class/MetaClassDatabase.h"
#include "Class/MetaClassName.h"
#include "Object/MetaObjectName.h"
#include "Property/MetaPropertyName.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void RTTIStartup::Start() {
    MetaClassName::Start(32);
    MetaPropertyName::Start(128);
    MetaObjectName::Start(128);

    MetaClassDatabase::Create();
    MetaAtomDatabase::Create();
}
//----------------------------------------------------------------------------
void RTTIStartup::Shutdown() {
    MetaAtomDatabase::Destroy();
    MetaClassDatabase::Destroy();

    MetaObjectName::Shutdown();
    MetaPropertyName::Shutdown();
    MetaClassName::Shutdown();
}
//----------------------------------------------------------------------------
void RTTIStartup::Clear() {
    MetaAtomDatabase::Instance().Clear();
    MetaClassDatabase::Instance().Clear();

    MetaObjectName::Clear();
    MetaPropertyName::Clear();
    MetaClassName::Clear();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
