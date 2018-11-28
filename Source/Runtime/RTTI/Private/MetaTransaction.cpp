#include "stdafx.h"

#include "MetaTransaction.h"

#include "RTTI/AtomVisitor.h" // Linearize()

#include "MetaDatabase.h"
#include "MetaObject.h"
#include "MetaObjectHelpers.h" // DeepEquals()

#include "Container/HashSet.h"
#include "Diagnostic/Logger.h"
#include "IO/Format.h"
#include "IO/FormatHelpers.h"
#include "IO/StringBuilder.h"
#include "IO/TextWriter.h"

#ifdef WITH_PPE_ASSERT
#   define WITH_PPE_RTTI_TRANSACTION_CHECKS 1
#else
#   define WITH_PPE_RTTI_TRANSACTION_CHECKS 0
#endif

#if WITH_PPE_RTTI_TRANSACTION_CHECKS
#endif

namespace PPE {
namespace RTTI {
EXTERN_LOG_CATEGORY(PPE_RTTI_API, RTTI)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if WITH_PPE_RTTI_TRANSACTION_CHECKS
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

            FLogger::Log(
                GLogCategory_RTTI,
                FLogger::EVerbosity::Fatal,
                FLogger::FSiteInfo::Make(WIDESTRING(__FILE__), __LINE__),
                oss.ToString() );
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
    VECTORINSITU(MetaTransaction, const FMetaObject*, 8) _chainRef;
};
} //!namespace
#endif //!WITH_PPE_RTTI_TRANSACTION_CHECKS
//----------------------------------------------------------------------------
// Explore object graph and dump them sorted by discover order
namespace {
class FMetaTransactionLinearizer_ : public FBaseAtomVisitor {
public:
    FMetaTransactionLinearizer_(const FMetaTransaction& outer, FLinearizedTransaction& linearized)
        : FBaseAtomVisitor(EVisitorFlags::OnlyObjects
            + (outer.KeepDeprecated() ? EVisitorFlags::KeepDeprecated : EVisitorFlags::Default)
            + (outer.KeepTransient() ? EVisitorFlags::KeepTransient : EVisitorFlags::Default) )
        , _outer(outer)
        , _linearized(linearized) {
        Assert_NoAssume(KeepDeprecated() == _outer.KeepDeprecated());
        Assert_NoAssume(KeepTransient() == _outer.KeepTransient());

        _visiteds.reserve(Max( // in case outer wasn't loaded
            outer.NumLoadedObjects(),
            outer.NumTopObjects()) );
    }

    void Linearize(const PMetaObject& pobj) {
        FMetaTransactionLinearizer_::Visit(nullptr, const_cast<PMetaObject&>(pobj));
    }

public: //FBaseAtomVisitor
    virtual bool Visit(const IScalarTraits* scalar, PMetaObject& pobj) override final {
        if (pobj && _visiteds.insert_ReturnIfExists(pobj.get()) == false) {
            const bool intern = (pobj->RTTI_Outer() == &_outer);
            const bool result = (intern
                ? FBaseAtomVisitor::Visit(scalar, pobj)
                : true );

            // register after recursive traversal, so we get postfix order :
            if (_outer.KeepTransient() || not pobj->RTTI_IsTransient())
                _linearized.push_back(FMetaObjectRef::Make(pobj.get(), intern));

            return result;
        }
        return true;
    }

private:
    const FMetaTransaction& _outer;
    FLinearizedTransaction& _linearized;
    HASHSET(MetaTransaction, FMetaObject*) _visiteds;
};
} //!namespace
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

        _outer->_state = ETransactionState::Loading;
    }

    ~FTransactionLoadContext() {
        Assert(ETransactionState::Loading == _outer->_state);
        Assert(_outer->_loadedObjects.size() >= _outer->_topObjects.size());
        Assert(_outer->_loadedObjects.size() >= _outer->_exportedObjects.size());

        _outer->_state = ETransactionState::Loaded;
    }

    virtual void OnLoadObject(FMetaObject& object) override final {
        Assert(object.RTTI_IsLoaded());
        Assert(object.RTTI_IsTopObject() == Contains(_outer->_topObjects, &object));

#if WITH_PPE_RTTI_TRANSACTION_CHECKS
        const FCheckCircularReference_::FScope circularScope(_circularCheck, &object);
#endif

        object.RTTI_SetOuter(_outer.get(), nullptr);

        _outer->_loadedObjects.emplace_AssertUnique(&object);

        if (object.RTTI_IsExported())
            _outer->_exportedObjects.emplace_AssertUnique(&object);

        FReferencedObjects children;
        CollectReferencedObjects(object, children, 1/* only directly referenced objects */);

        for (FMetaObject* ref : children) {
            Assert(&object != ref);

            const FMetaTransaction* refOuter = ref->RTTI_Outer();
            if (refOuter && _outer != refOuter) {
                Assert(refOuter);
                Assert(refOuter->IsLoaded());
                Assert(ref->RTTI_IsExported());

                if (Add_Unique(_outer->_importedTransactions, SCMetaTransaction(refOuter))) {
#if WITH_PPE_RTTI_TRANSACTION_CHECKS
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
#if WITH_PPE_RTTI_TRANSACTION_CHECKS
    FCheckCircularReference_ _circularCheck;
#endif
};
//----------------------------------------------------------------------------
class FTransactionUnloadContext : public IUnloadContext {
public:
    FTransactionUnloadContext(FMetaTransaction& outer)
        : _outer(&outer) {
        Assert(outer.IsLoaded());

        _outer->_state = ETransactionState::Unloading;
    }

    ~FTransactionUnloadContext() {
        Assert(ETransactionState::Unloading == _outer->_state);

#if WITH_PPE_RTTI_TRANSACTION_CHECKS
        for (const SMetaObject& obj : _outer->_loadedObjects)
            Assert(obj->RTTI_IsUnloaded());
#endif

        _outer->_loadedObjects.clear_ReleaseMemory();
        _outer->_exportedObjects.clear_ReleaseMemory();
        _outer->_importedTransactions.clear_ReleaseMemory();

        _outer->_state = ETransactionState::Unloaded;
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
FMetaTransaction::FMetaTransaction(
    const FName& name,
    ETransactionFlags flags/* = ETransactionFlags::Default */)
    : _name(name)
    , _flags(flags)
    , _state(ETransactionState::Unloaded) {
}
//----------------------------------------------------------------------------
FMetaTransaction::FMetaTransaction(const FName& name, VECTOR(MetaTransaction, PMetaObject)&& objects)
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

#if WITH_PPE_RTTI_TRANSACTION_CHECKS
    // checks that everything was correctly dispatched by load context
    for (const PMetaObject& object : _topObjects) {
        Assert(_loadedObjects.find(object) != _loadedObjects.end());
        Assert(!object->RTTI_IsExported() || _exportedObjects.find(object) != _exportedObjects.end());
    }
#endif

    // finally exports everything to database
    {
        const FMetaDatabaseReadWritable metaDB;

        metaDB->RegisterTransaction(this);

        for (const SMetaObject& object : _exportedObjects) {
            Assert(object->RTTI_Outer() == this);
            metaDB->RegisterObject(object);
        }
    }

#ifdef USE_DEBUG_LOGGER
    LOG(RTTI, Info, L"loaded transaction '{0}' :", _name);

    FWStringBuilder oss;
    Fmt::FWIndent indent = Fmt::FWIndent::UsingTabs();
    indent.Inc();

    oss << indent << L" - Top objects : " << _topObjects.size() << Eol;
    {
        size_t index = 0;
        const Fmt::FWIndent::FScope scopeIndent(indent);
        for (const PMetaObject& obj : _topObjects)
            oss << indent << L'['
                << Fmt::PadLeft(index++, 3, L'0') << L"]  "
                << (void*)obj.get() << L" : "
                << obj->RTTI_Class()->Name() << L" ("
                << obj->RTTI_Flags() << L')'
                << Eol;

        FLogger::Log(
            GLogCategory_RTTI,
            FLogger::EVerbosity::Info,
            FLogger::FSiteInfo::Make(WIDESTRING(__FILE__), __LINE__),
            oss.ToString() );
    }
    oss << indent << L" - Exported objects : " << _exportedObjects.size() << Eol;
    {
        size_t index = 0;
        const Fmt::FWIndent::FScope scopeIndent(indent);
        for (const SMetaObject& obj : _exportedObjects)
            oss << indent << L'['
                << Fmt::PadLeft(index++, 3, L'0') << L"]  "
                << (void*)obj.get() << L" : "
                << obj->RTTI_Class()->Name() << L" = '"
                << obj->RTTI_Name() << L"' ("
                << obj->RTTI_Flags() << L')'
                << Eol;

        FLogger::Log(
            GLogCategory_RTTI,
            FLogger::EVerbosity::Info,
            FLogger::FSiteInfo::Make(WIDESTRING(__FILE__), __LINE__),
            oss.ToString() );
    }
    oss << indent << L" - Loaded objects : " << _loadedObjects.size() << Eol;
    {
        size_t index = 0;
        const Fmt::FWIndent::FScope scopeIndent(indent);
        for (const SMetaObject& obj : _loadedObjects)
            oss << indent << L'['
                << Fmt::PadLeft(index++, 3, L'0') << L"]  "
                << (void*)obj.get() << L" : "
                << obj->RTTI_Class()->Name() << L" ("
                << obj->RTTI_Flags() << L')'
                << Eol;

        FLogger::Log(
            GLogCategory_RTTI,
            FLogger::EVerbosity::Info,
            FLogger::FSiteInfo::Make(WIDESTRING(__FILE__), __LINE__),
            oss.ToString() );
    }
    oss << indent << L" - Imported transactions : " << _importedTransactions.size() << Eol;
    {
        size_t index = 0;
        const Fmt::FWIndent::FScope scopeIndent(indent);
        for (const SCMetaTransaction& outer : _importedTransactions)
            oss << indent << L'['
                << Fmt::PadLeft(index++, 3, L'0') << L"]  "
                << (void*)outer.get() << L" = '"
                << outer->Name() << L"' : "
                << outer->State() << L" ("
                << outer->Flags() << L')'
                << Eol;

        FLogger::Log(
            GLogCategory_RTTI,
            FLogger::EVerbosity::Info,
            FLogger::FSiteInfo::Make(WIDESTRING(__FILE__), __LINE__),
            oss.ToString() );
    }

#endif
}
//----------------------------------------------------------------------------
void FMetaTransaction::Unload() {
    Assert(IsLoaded());
    Assert(_loadedObjects.size() >= _topObjects.size());

    FTransactionUnloadContext unloadContext(*this);

    // first unregister everything from database
    {
        const FMetaDatabaseReadWritable metaDB;

        for (const SMetaObject& object : _exportedObjects) {
            Assert(object->RTTI_Outer() == this);
            metaDB->UnregisterObject(object);
        }

        metaDB->UnregisterTransaction(this);
    }

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
void FMetaTransaction::Linearize(FLinearizedTransaction* linearized) const {
    Assert(linearized);
    Assert(linearized->empty());

    FMetaTransactionLinearizer_ linearizer(*this, *linearized);

    // descend in each top object and append reference in order of discovery
    for (const auto& topObject : _topObjects)
        linearizer.Linearize(topObject);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FMetaObjectRef FMetaObjectRef::Make(FMetaObject* obj, bool intern) {
    FMetaObjectRef ref;
    ref.Reset(obj, intern && obj->RTTI_IsExported(), not intern);
    return ref;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FMetaTransaction::FLoadingScope::FLoadingScope(FMetaTransaction& transaction)
:   Transaction(transaction) {
    Transaction.Load();
}
//----------------------------------------------------------------------------
FMetaTransaction::FLoadingScope::~FLoadingScope() {
    Transaction.Unload();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, RTTI::ETransactionFlags flags) {
    auto sep = Fmt::NotFirstTime('|');

    if (flags == RTTI::ETransactionFlags::Default)
        return oss << "Default";
    if (flags ^ RTTI::ETransactionFlags::KeepDeprecated)
        oss << sep << "KeepDeprecated";
    if (flags ^ RTTI::ETransactionFlags::KeepTransient)
        oss << sep << "KeepTransient";

    return oss;
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, RTTI::ETransactionFlags flags) {
    auto sep = Fmt::NotFirstTime(L'|');

    if (flags == RTTI::ETransactionFlags::Default)
        return oss << L"Default";
    if (flags ^ RTTI::ETransactionFlags::KeepDeprecated)
        oss << sep << L"KeepDeprecated";
    if (flags ^ RTTI::ETransactionFlags::KeepTransient)
        oss << sep << L"KeepTransient";

    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, RTTI::ETransactionState state) {
    switch (state) {
    case RTTI::ETransactionState::Unloaded:     return oss << "Unloaded";
    case RTTI::ETransactionState::Loading:      return oss << "Loading";
    case RTTI::ETransactionState::Loaded:       return oss << "Loaded";
    case RTTI::ETransactionState::Unloading:    return oss << "Unloading";
    }
    AssertNotReached();
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, RTTI::ETransactionState state) {
    switch (state) {
    case RTTI::ETransactionState::Unloaded:     return oss << L"Unloaded";
    case RTTI::ETransactionState::Loading:      return oss << L"Loading";
    case RTTI::ETransactionState::Loaded:       return oss << L"Loaded";
    case RTTI::ETransactionState::Unloading:    return oss << L"Unloading";
    }
    AssertNotReached();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
