#include "stdafx.h"

#include "MetaTransaction.h"

#include "MetaDatabase.h"
#include "MetaObject.h"

#ifdef WITH_CORE_ASSERT
#   define WITH_CORE_RTTI_TRANSACTION_CHECKS
#endif

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
class FCompositeLoadContext_ : public ILoadContext {
public:
    FCompositeLoadContext_(std::initializer_list<ILoadContext*> contexts)
        : _contexts(std::move(contexts))
    {}

    virtual void OnLoadObject(FMetaObject& object) override final {
        foreachitem(context, _contexts) {
            if (*context)
                (*context)->OnLoadObject(object);
        }
    }

private:
    VECTORINSITU_THREAD_LOCAL(RTTI, ILoadContext*, 2) _contexts;
};
//----------------------------------------------------------------------------
class FCompositeUnloadContext_ : public IUnloadContext {
public:
    FCompositeUnloadContext_(std::initializer_list<IUnloadContext*> contexts)
        : _contexts(std::move(contexts))
    {}

    virtual void OnUnloadObject(FMetaObject& object) override final {
        foreachitem(context, _contexts) {
            if (*context)
                (*context)->OnUnloadObject(object);
        }
    }

private:
    VECTORINSITU_THREAD_LOCAL(RTTI, IUnloadContext*, 2) _contexts;
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FMetaTransaction::FMetaTransaction(const FName& name)
    : _name(name)
    , _flags(ETransactionFlags::Unloaded) {
}
//----------------------------------------------------------------------------
FMetaTransaction::FMetaTransaction(const FName& name, VECTOR(RTTI, PMetaObject)&& objects)
    : FMetaTransaction(name) {
    _topObjects.assign(std::move(objects));
}
//----------------------------------------------------------------------------
FMetaTransaction::~FMetaTransaction() {
    Assert(not IsLoaded());
    Assert(IsUnloaded());
}
//----------------------------------------------------------------------------
bool FMetaTransaction::Contains(const FMetaObject* object) const {
    Assert(IsLoaded());

    return (_loadedObjects.end() != _loadedObjects.find(SCMetaObject(object)));
}
//----------------------------------------------------------------------------
void FMetaTransaction::RegisterObject(FMetaObject* object) {
    Assert(object);
    Assert(object->RTTI_IsUnloaded());
    Assert(IsUnloaded());

    Add_AssertUnique(_topObjects, PMetaObject(object));

    object->RTTI_MarkAsTopObject();
}
//----------------------------------------------------------------------------
void FMetaTransaction::UnregisterObject(FMetaObject* object) {
    Assert(object);
    Assert(object->RTTI_IsUnloaded());
    Assert(IsUnloaded());

    object->RTTI_UnmarkAsTopObject();

    Remove_AssertExists(_topObjects, PMetaObject(object));
}
//----------------------------------------------------------------------------
void FMetaTransaction::Load(ILoadContext* context) {
    Assert(IsUnloaded());
    Assert(_loadedObjects.empty());

    _flags = ETransactionFlags::Loading;

    FCompositeLoadContext_ transactionContext = {
        static_cast<ILoadContext*>(this),
        context
    };

    FMetaDatabase& metaDB = MetaDB();

    for (const PMetaObject& object : _topObjects) {
        Assert(object);
        Assert(object->RTTI_IsTopObject());

        // Use IFN since cross referencing top objects could load themselves
        object->RTTI_CallLoadIFN(&transactionContext);

        if (object->RTTI_IsExported())
            metaDB.RegisterObject(object.get());
    }

    _flags = ETransactionFlags::Loaded;

#ifdef WITH_CORE_RTTI_TRANSACTION_CHECKS
    // check that every top object meta class correctly called OnLoadObject()
    for (const PMetaObject& object : _topObjects) {
        Assert(object->RTTI_IsLoaded());
        Assert(_loadedObjects.end() != _loadedObjects.find(SCMetaObject(object.get())));
    }
#endif

    metaDB.RegisterTransaction(this);
}
//----------------------------------------------------------------------------
void FMetaTransaction::Unload(IUnloadContext* context) {
    Assert(context);
    Assert(IsLoaded());
    Assert(_loadedObjects.size() >= _topObjects.size());

    _flags = ETransactionFlags::Unloading;

    FMetaDatabase& metaDB = MetaDB();

    FCompositeUnloadContext_ transactionContext = {
        context,  // reverse load order
        static_cast<IUnloadContext*>(this)
    };

    for (const PMetaObject& object : _topObjects) {
        Assert(object);

        if (object->RTTI_IsExported())
            metaDB.UnregisterObject(object.get());

        // Use IFN since cross referencing top objects could unload themselves
        object->RTTI_CallUnloadIFN(&transactionContext);
    }

    _flags = ETransactionFlags::Unloaded;

#ifdef WITH_CORE_RTTI_TRANSACTION_CHECKS
    // check that each object of the transaction was unloaded (including non top objects)
    Assert(_loadedObjects.empty()); // or else OnUnloadObject() wasn't called by object
#endif

    _loadedObjects.clear_ReleaseMemory();
}
//----------------------------------------------------------------------------
void FMetaTransaction::OnLoadObject(FMetaObject& object) {
    Assert(ETransactionFlags::Loading == _flags);

    Insert_AssertUnique(_loadedObjects, SCMetaObject(&object));
}
//----------------------------------------------------------------------------
void FMetaTransaction::OnUnloadObject(FMetaObject& object) {
    Assert(ETransactionFlags::Loading == _flags);

    Remove_AssertExists(_loadedObjects, SCMetaObject(&object));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
std::basic_ostream<char>& operator <<(std::basic_ostream<char>& oss, RTTI::ETransactionFlags flags) {
    switch (flags) {
    case RTTI::ETransactionFlags::Unloaded:     return oss << "Unloaded";
    case RTTI::ETransactionFlags::Loading:      return oss << "Loading";
    case RTTI::ETransactionFlags::Loaded:       return oss << "Loaded";
    case RTTI::ETransactionFlags::Unloading:    return oss << "Unloading";
    default:
        AssertNotImplemented();
    }
    return oss;
}
//----------------------------------------------------------------------------
std::basic_ostream<wchar_t>& operator <<(std::basic_ostream<wchar_t>& oss, RTTI::ETransactionFlags flags) {
    switch (flags) {
    case RTTI::ETransactionFlags::Unloaded:     return oss << L"Unloaded";
    case RTTI::ETransactionFlags::Loading:      return oss << L"Loading";
    case RTTI::ETransactionFlags::Loaded:       return oss << L"Loaded";
    case RTTI::ETransactionFlags::Unloading:    return oss << L"Unloading";
    default:
        AssertNotImplemented();
    }
    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
