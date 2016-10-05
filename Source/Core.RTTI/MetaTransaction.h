#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core.RTTI/Typedefs.h"

#include "Core/Container/HashMap.h"
#include "Core/Memory/RefPtr.h"

namespace Core {
namespace RTTI {
class FMetaAtomHashMap;
class FMetaClassHashMap;
class FMetaLoadContext;
class FMetaUnloadContext;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(MetaAtom);
FWD_REFPTR(MetaObject);
class FMetaClass;
//----------------------------------------------------------------------------
class FMetaTransaction : public FRefCountable {
public:
    FMetaTransaction();
    explicit FMetaTransaction(VECTOR(RTTI, PMetaObject)&& objects);
    ~FMetaTransaction();

    FMetaTransaction(const FMetaTransaction&) = delete;
    FMetaTransaction& operator =(const FMetaTransaction&) = delete;

    bool IsLoaded() const { return _loaded; }
    bool IsUnloaded() const { return _unloaded; }

    void Add(FMetaObject* object);
    void Remove(FMetaObject* object);
    bool Contains(const FMetaObject* object) const;

    void Load(FMetaLoadContext* context);
    void Unload(FMetaUnloadContext* context);

    bool empty() const { return _objects.empty(); }
    size_t size() const { return _objects.size(); }
    size_t capacity() const { return _objects.capacity(); }
    void reserve(size_t count) { _objects.reserve(count); }

    TMemoryView<const PMetaObject> MakeView() const { return Core::MakeView(_objects); }

    bool Equals(const FMetaTransaction& other) const;
    bool DeepEquals(const FMetaTransaction& other) const;

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
