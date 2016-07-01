#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core.RTTI/Typedefs.h"

#include "Core/Container/HashMap.h"
#include "Core/Memory/RefPtr.h"

namespace Core {
namespace RTTI {
class MetaAtomHashMap;
class MetaClassHashMap;
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
    explicit MetaTransaction(VECTOR(RTTI, PMetaObject)&& objects);
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

    bool empty() const { return _objects.empty(); }
    size_t size() const { return _objects.size(); }
    size_t capacity() const { return _objects.capacity(); }
    void reserve(size_t count) { _objects.reserve(count); }

    MemoryView<const PMetaObject> MakeView() const { return Core::MakeView(_objects); }

    bool Equals(const MetaTransaction& other) const;
    bool DeepEquals(const MetaTransaction& other) const;

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
