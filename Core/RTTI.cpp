#include "stdafx.h"

#include "RTTI.h"

#include "MetaAtomDatabase.h"
#include "MetaClassDatabase.h"

#include "MetaClassName.h"
#include "MetaObjectName.h"
#include "MetaPropertyName.h"

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
