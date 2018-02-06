#include "stdafx.h"

#include "MetaTransaction.h"

#include "AtomVisitor.h" // FCheckCircularReference_
#include "MetaDatabase.h"
#include "MetaObject.h"
#include "MetaObjectHelpers.h" // DeepEquals()

#include "Core/Diagnostic/Logger.h"
#include "Core/IO/Format.h"
#include "Core/IO/FormatHelpers.h"
#include "Core/IO/StringBuilder.h"
#include "Core/IO/TextWriter.h"

#ifdef WITH_CORE_ASSERT
#   define WITH_CORE_RTTI_TRANSACTION_CHECKS 1
#else
#   define WITH_CORE_RTTI_TRANSACTION_CHECKS 0
#endif

#if WITH_CORE_RTTI_TRANSACTION_CHECKS
#endif

namespace Core {
namespace RTTI {
EXTERN_LOG_CATEGORY(CORE_RTTI_API, RTTI);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if WITH_CORE_RTTI_TRANSACTION_CHECKS
namespace {
class FCheckCircularReference_ {
public:
    FCheckCircularReference_() {}
    ~FCheckCircularReference_() {
        Assert(_chainRef.empty());
    }

    bool empty() const { return _chainRef.empty(); }

    void Push(const FMetaObject* object) {
        Assert(object);

        _chainRef.push_back(object);

        const auto chainEnd = (_chainRef.end() - 1/* skip the item just added */);
        const auto it = std::find(_chainRef.begin(), chainEnd, object);

        if (chainEnd != it) {
#ifdef USE_DEBUG_LOGGER
            FWStringBuilder oss;
            oss << L"found a circular reference !" << Eol;

            const size_t prev = std::distance(_chainRef.begin(), it);

            Fmt::FWIndent indent = Fmt::FWIndent::UsingTabs();
            forrange(i, 0, _chainRef.size()) {
                const FMetaObject* ref = _chainRef[i];
                const FMetaClass* metaClass = ref->RTTI_Class();

                Format(oss, L"[{0:#2}]", i);
                oss << indent;
                    
                if (i == prev)
                    oss << L" ==> ";
                else if (i + 1 == _chainRef.size())
                    oss << L" [*] ";
                else
                    oss << L" --- ";

                oss << (void*)ref
                    << L" \"" << ref->RTTI_Name()
                    << L"\" : " << metaClass->Name()
                    << L" (" << metaClass->Flags()
                    << L")" << Eol;

                indent.Inc();
            }

            LOG(RTTI, Fatal, oss.ToString());
#else
            AssertNotReached();
#endif
        }
    }

    void Pop(const FMetaObject* object) {
        Assert(_chainRef.back() == object);

        _chainRef.pop_back();
    }
    
    struct FScope {
        FCheckCircularReference_& Check;
        const FMetaObject* const Object;

        FScope(FCheckCircularReference_& check, const FMetaObject* object)
            : Check(check), Object(object) {
            Check.Push(Object);
        }

        ~FScope() {
            Check.Pop(Object);
        }
    };

private:    
    VECTORINSITU_THREAD_LOCAL(RTTI, const FMetaObject*, 8) _chainRef;
};
} //!namespace
#endif //!WITH_CORE_RTTI_TRANSACTION_CHECKS
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FTransactionLoadContext : public ILoadContext {
public:
    FTransactionLoadContext(FMetaTransaction& outer)
        : _outer(&outer) {
        Assert(outer.IsUnloaded());
        Assert(outer._loadedObjects.empty());
        Assert(outer._exportedObjects.empty());
        Assert(outer._importedTransactions.empty());

        _outer->_flags = ETransactionFlags::Loading;
    }

    ~FTransactionLoadContext() {
        Assert(ETransactionFlags::Loading == _outer->_flags);
        Assert(_outer->_loadedObjects.size() >= _outer->_topObjects.size());
        Assert(_outer->_loadedObjects.size() >= _outer->_exportedObjects.size());

        _outer->_flags = ETransactionFlags::Loaded;
    }

    virtual void OnLoadObject(FMetaObject& object) override final {
        Assert(object.RTTI_IsLoaded());
        Assert(object.RTTI_IsTopObject() == Contains(_outer->_topObjects, &object));

#if WITH_CORE_RTTI_TRANSACTION_CHECKS
        const FCheckCircularReference_::FScope circularScope(_circularCheck, &object);
#endif

        object.RTTI_SetOuter(_outer.get(), nullptr);

        _outer->_loadedObjects.emplace_AssertUnique(&object);

        if (object.RTTI_IsExported())
            _outer->_exportedObjects.emplace_AssertUnique(&object);

        TReferencedObjects children;
        CollectReferencedObjects(object, children, 1/* only directly referenced objects */);

        for (FMetaObject* ref : children) {
            Assert(&object != ref);

            const FMetaTransaction* refOuter = ref->RTTI_Outer();
            if (refOuter && _outer != refOuter) {
                Assert(refOuter);
                Assert(refOuter->IsLoaded());
                Assert(ref->RTTI_IsExported());
                
                if (Add_Unique(_outer->_importedTransactions, SCMetaTransaction(refOuter))) {
#if WITH_CORE_RTTI_TRANSACTION_CHECKS
                    if (Contains(refOuter->_importedTransactions, _outer))
                        LOG(RTTI, Fatal, L"found a circular transaction import : {0} <=> {1}",
                            _outer->Name(), refOuter->Name());
#endif
                }
            }
            else {
                ref->RTTI_CallLoadIFN(*this);
                Assert(ref->RTTI_IsLoaded());
            }
        }
    }

private:
    SMetaTransaction _outer;
#if WITH_CORE_RTTI_TRANSACTION_CHECKS
    FCheckCircularReference_ _circularCheck;
#endif
};
//----------------------------------------------------------------------------
class FTransactionUnloadContext : public IUnloadContext {
public:
    FTransactionUnloadContext(FMetaTransaction& outer)
        : _outer(&outer) {
        Assert(outer.IsLoaded());

        _outer->_flags = ETransactionFlags::Unloading;
    }

    ~FTransactionUnloadContext() {
        Assert(ETransactionFlags::Unloading == _outer->_flags);

#if WITH_CORE_RTTI_TRANSACTION_CHECKS
        for (const SMetaObject& obj : _outer->_loadedObjects)
            Assert(obj->RTTI_IsUnloaded());
#endif

        _outer->_loadedObjects.clear_ReleaseMemory();
        _outer->_exportedObjects.clear_ReleaseMemory();
        _outer->_importedTransactions.clear_ReleaseMemory();

        _outer->_flags = ETransactionFlags::Unloaded;
    }

    virtual void OnUnloadObject(FMetaObject& object) override final {
        Assert(object.RTTI_IsLoaded());
        Assert(object.RTTI_IsTopObject() == Contains(_outer->_topObjects, &object));

        object.RTTI_SetOuter(nullptr, _outer.get());
    }

private:
    SMetaTransaction _outer;
};
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

    hash_t h = hash_value(object);
    const auto it = _loadedObjects.find_like(object, h);

    return (_loadedObjects.end() != it);
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
void FMetaTransaction::Load() {
    Assert(IsUnloaded());

    _loadedObjects.reserve(_topObjects.size());

    FTransactionLoadContext loadContext(*this);

    // explore all objects starting from roots
    for (const PMetaObject& object : _topObjects) {
        Assert(object);
        Assert(object->RTTI_IsTopObject());

        // Use IFN since cross referencing top objects could load themselves
        object->RTTI_CallLoadIFN(loadContext);
    }

#if WITH_CORE_RTTI_TRANSACTION_CHECKS
    // checks that everything was correctly dispatched by load context
    for (const PMetaObject& object : _topObjects) {
        Assert(_loadedObjects.find(object) != _loadedObjects.end());
        Assert(!object->RTTI_IsExported() || _exportedObjects.find(object) != _exportedObjects.end());
    }
#endif

    // finally exports everything to database
    FMetaDatabase& metaDB = MetaDB();
    metaDB.RegisterTransaction(this);

    for (const SMetaObject& object : _exportedObjects) {
        Assert(object->RTTI_Outer() == this);
        metaDB.RegisterObject(object);
    }

#ifdef USE_DEBUG_LOGGER
    FWStringBuilder oss;
    oss << L"loaded transaction '" << _name << L"' :" << Eol;

    Fmt::FWIndent indent = Fmt::FWIndent::UsingTabs();
    indent.Inc();

    oss << indent << L" - Top objects : " << _topObjects.size() << Eol;
    {
        size_t index = 0;
        const Fmt::FWIndent::FScope scopeIndent(indent);
        for (const PMetaObject& obj : _topObjects)
            oss << indent << L'[' 
                << (index++) << L"]  " 
                << (void*)obj.get() << L" : "
                << obj->RTTI_Class()->Name() << L" ("
                << obj->RTTI_Flags() << L')'
                << Eol;
    }
    oss << indent << L" - Exported objects : " << _exportedObjects.size() << Eol;
    {
        size_t index = 0;
        const Fmt::FWIndent::FScope scopeIndent(indent);
        for (const SMetaObject& obj : _exportedObjects)
            oss << indent << L'['
                << (index++) << L"]  "
                << (void*)obj.get() << L" : "
                << obj->RTTI_Class()->Name() << L" = '"
                << obj->RTTI_Name() << L"' ("
                << obj->RTTI_Flags() << L')'
                << Eol;
    }
    oss << indent << L" - Loaded objects : " << _loadedObjects.size() << Eol;
    {
        size_t index = 0;
        const Fmt::FWIndent::FScope scopeIndent(indent);
        for (const SMetaObject& obj : _loadedObjects)
            oss << indent << L'['
                << (index++) << L"]  "
                << (void*)obj.get() << L" : "
                << obj->RTTI_Class()->Name() << L" ("
                << obj->RTTI_Flags() << L')'
                << Eol;
    }
    oss << indent << L" - Imported transactions : " << _importedTransactions.size() << Eol;
    {
        size_t index = 0;
        const Fmt::FWIndent::FScope scopeIndent(indent);
        for (const SCMetaTransaction& outer : _importedTransactions)
            oss << indent << L'['
                << (index++) << L"]  "
                << (void*)outer.get() << L" = '"
                << outer->Name() << L"' ("
                << outer->Flags() << L')'
                << Eol;
    }

    LOG(RTTI, Info, oss.ToString());

#endif
}
//----------------------------------------------------------------------------
void FMetaTransaction::Unload() {
    Assert(IsLoaded());
    Assert(_loadedObjects.size() >= _topObjects.size());

    FTransactionUnloadContext unloadContext(*this);

    // first unregister everything from database
    FMetaDatabase& metaDB = MetaDB();

    for (const SMetaObject& object : _exportedObjects) {
        Assert(object->RTTI_Outer() == this);
        metaDB.UnregisterObject(object);
    }

    metaDB.UnregisterTransaction(this);

    // unload every loaded object
    for (const SMetaObject& object : _loadedObjects) {
        Assert(object);

        // Use IFN since cross referencing top objects could unload themselves
        object->RTTI_CallUnloadIFN(unloadContext);
    }
}
//----------------------------------------------------------------------------
void FMetaTransaction::Reload() {
    LOG(RTTI, Info, L"reloading transaction '{0}' ({1} top objects)", 
        _name, _topObjects.size() );

    Unload();
    Load();
}
//----------------------------------------------------------------------------
void FMetaTransaction::reserve(size_t capacity) {
    Assert(IsUnloaded());

    _topObjects.reserve(capacity);
}
//----------------------------------------------------------------------------
bool FMetaTransaction::DeepEquals(const FMetaTransaction& other) const {
    if (other._topObjects.size() != _topObjects.size())
        return false;

    forrange(i, 0, _topObjects.size()) {
        const FMetaObject* lhs = _topObjects[i].get();
        const FMetaObject* rhs = other._topObjects[i].get();
        if (not RTTI::DeepEquals(*lhs, *rhs))
            return false;
    }

    return true;
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
FTextWriter& operator <<(FTextWriter& oss, RTTI::ETransactionFlags flags) {
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
FWTextWriter& operator <<(FWTextWriter& oss, RTTI::ETransactionFlags flags) {
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
