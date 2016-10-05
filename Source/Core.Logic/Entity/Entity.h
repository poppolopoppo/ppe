#pragma once

#include "Core.Logic/Logic.h"

#include "Core.Logic/Component/Component_fwd.h"
#include "Core.Logic/Entity/Entity_fwd.h"

namespace Core {
namespace Logic {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FEntity {
public:
    friend class FEntityManager;

    STATIC_CONST_INTEGRAL(EntityID::value_type, InvalidID, -1);
    STATIC_CONST_INTEGRAL(EntityUID::value_type, InvalidUID, -1);

    FEntity(EntityID id, EntityUID uid) 
    :   _id(id)
    ,   _enabled(true), _deleting(false), _refreshing(false)
    ,   _componentFlags(0)
    ,   _uid(uid) {}
    FEntity() : FEntity(EntityID(InvalidID), EntityUID(InvalidUID)) {}

    EntityID ID() const { return EntityID(_id); }
    EntityUID UID() const { return _uid; }

    bool Enabled() const { return _enabled; }
    bool Deleting() const { return _deleting; }
    bool Refreshing() const { return _refreshing; }

    bool Valid() const { return _id != InvalidID; }

    ComponentFlag ComponentFlags() const { return _componentFlags; }

    void Enable() { _enabled = true; }
    void Disable() { _enabled = false; }

    void Swap(FEntity& other);

    static void Start();
    static void Shutdown();

private:
    u32 _id : 29;
    u32 _enabled : 1;
    u32 _deleting : 1;
    u32 _refreshing : 1;

    ComponentFlag _componentFlags;

    EntityUID _uid;
};
//----------------------------------------------------------------------------
STATIC_ASSERT(sizeof(FEntity) == sizeof(EntityID)+sizeof(ComponentFlag)+sizeof(EntityUID) );
//----------------------------------------------------------------------------
inline void swap(FEntity& lhs, FEntity& rhs) { lhs.Swap(rhs); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Logic
} //!namespace Core
