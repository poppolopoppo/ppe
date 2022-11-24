// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Entity.h"

#include "Component/IComponent.h"
#include "Component/ComponentContainer.h"

#include "Entity/EntityContainer.h"
#include "Entity/EntityTag.h"

namespace PPE {
namespace Logic {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FEntity::Start() {
    FEntityTag::Start(64);
}
//----------------------------------------------------------------------------
void FEntity::Shutdown() {
    FEntityTag::Shutdown();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Logic
} //!namespace PPE
