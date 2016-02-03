#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core/Container/HashMap.h"
#include "Core/Memory/RefPtr.h"

namespace Core {
namespace RTTI {
class MetaAtomHashMap;
class MetaClassHashMap;
class MetaClassName;
class MetaObjectName;
class MetaLoadContext;
class MetaUnloadContext;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(MetaAtom);
FWD_REFPTR(MetaObject);
class MetaClass;
//----------------------------------------------------------------------------
class MetaTransaction : public RefCountable {
public:
    MetaTransaction();
    ~MetaTransaction();

    MetaTransaction(const MetaTransaction&) = delete;
    MetaTransaction& operator =(const MetaTransaction&) = delete;

    bool IsLoaded() const { return _loaded; }
    bool IsUnloaded() const { return _unloaded; }

    void Add(MetaObject* object);
    void Remove(MetaObject* object);
    bool Contains(const MetaObject* object) const;

    void Load(MetaLoadContext* context);
    void Unload(MetaUnloadContext* context);

private:
    VECTOR(RTTI, PMetaObject) _objects;

    bool _loaded : 1;
    bool _unloaded : 1;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
