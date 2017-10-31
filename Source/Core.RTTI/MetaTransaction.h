#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core.RTTI/Typedefs.h"

#include "Core/Container/HashSet.h"
#include "Core/Container/Vector.h"
#include "Core/Memory/RefPtr.h"

namespace Core {
namespace RTTI {
FWD_REFPTR(MetaObject);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class ETransactionFlags : u32 {
    Unloaded    = 0,
    Loading     ,
    Loaded      ,
    Unloading   ,
};
//----------------------------------------------------------------------------
class ILoadContext {
public:
    virtual ~ILoadContext() {}
    virtual void OnLoadObject(FMetaObject& object) = 0;
};
//----------------------------------------------------------------------------
class IUnloadContext {
public:
    virtual ~IUnloadContext() {}
    virtual void OnUnloadObject(FMetaObject& object) = 0;
};
//----------------------------------------------------------------------------
FWD_REFPTR(MetaTransaction);
class CORE_RTTI_API EMPTY_BASES FMetaTransaction
    : protected ILoadContext
    , protected IUnloadContext
    , public FRefCountable {
public:
    explicit FMetaTransaction(const FName& name);
    FMetaTransaction(const FName& name, VECTOR(RTTI, PMetaObject)&& objects);
    virtual ~FMetaTransaction();

    const FName& Name() const { return _name; }
    ETransactionFlags Flags() const { return _flags; }

    bool IsLoaded() const   { return (_flags == ETransactionFlags::Loaded); }
    bool IsUnloaded() const { return (_flags == ETransactionFlags::Unloaded); }

    bool Contains(const FMetaObject* object) const;
    void RegisterObject(FMetaObject* object);
    void UnregisterObject(FMetaObject* object);

    void Load(ILoadContext* context);
    void Unload(IUnloadContext* context);

    TMemoryView<const PMetaObject> TopObjects() const {
        return _topObjects.MakeConstView();
    }

    const HASHSET(RTTI, SCMetaObject)& LoadedObjects() const {
        return _loadedObjects;
    }

protected: // ILoadContext + IUnloadContxt
    virtual void OnLoadObject(FMetaObject& object) override;
    virtual void OnUnloadObject(FMetaObject& object) override;

private:
    FName _name;
    ETransactionFlags _flags;
    VECTOR(RTTI, PMetaObject) _topObjects;
    HASHSET(RTTI, SCMetaObject) _loadedObjects;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
CORE_RTTI_API std::basic_ostream<char>& operator <<(std::basic_ostream<char>& oss, RTTI::ETransactionFlags flags);
CORE_RTTI_API std::basic_ostream<wchar_t>& operator <<(std::basic_ostream<wchar_t>& oss, RTTI::ETransactionFlags flags);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
