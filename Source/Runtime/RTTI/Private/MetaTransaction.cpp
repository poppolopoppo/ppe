#include "stdafx.h"

#include "MetaTransaction.h"

#include "RTTI/AtomVisitor.h"

#include "MetaDatabase.h"
#include "MetaObject.h"
#include "MetaObjectHelpers.h" // DeepEquals()/Linearize()/CollectReferences()

#include "Container/HashSet.h"
#include "Diagnostic/Logger.h"
#include "IO/Format.h"
#include "IO/FormatHelpers.h"
#include "IO/StringBuilder.h"
#include "IO/TextWriter.h"

#define WITH_PPE_RTTI_TRANSACTION_CHECKS (USE_PPE_ASSERT)

namespace PPE {
namespace RTTI {
EXTERN_LOG_CATEGORY(PPE_RTTI_API, RTTI)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
void LinearizeTransaction_(
    FLinearizedTransaction* linearized,
    const FMetaTransaction& outer,
    const TMemoryView<const PMetaObject>& topObjects) {
    Assert(linearized);
    Assert_NoAssume(linearized->ImportedRefs.empty());
    Assert_NoAssume(linearized->LoadedRefs.empty());
    Assert_NoAssume(linearized->ExportedRefs.empty());

    HASHSET(MetaTransaction, const FMetaObject*) visiteds;
    visiteds.reserve(topObjects.size());
    linearized->LoadedRefs.reserve(visiteds.size());

    CollectReferences(
        topObjects,
#if WITH_PPE_RTTI_TRANSACTION_CHECKS
        [linearized, &outer, &visiteds](const ITypeTraits&, FMetaObject& obj) {
#else
        [linearized, &visiteds](const ITypeTraits&, FMetaObject& obj) {
#endif
            if (not visiteds.insert_ReturnIfExists(&obj)) {
                if (obj.RTTI_IsExported()) {
                    if (obj.RTTI_Outer() == nullptr) {
                        linearized->ExportedRefs.emplace_back(&obj);
                    }
                    else {
                        AssertRelease(obj.RTTI_IsLoaded());

#if WITH_PPE_RTTI_TRANSACTION_CHECKS
                        CLOG(obj.RTTI_Outer()->Linearized().HasImport(outer), RTTI, Fatal,
                            L"found a circular transaction import : {0} <=> {1}",
                            outer.Namespace(), obj.RTTI_Outer()->Namespace() );
#endif

                        linearized->ImportedRefs.emplace_back(&obj);
                        return false; // don't recurse on imported objects
                    }
                }

                Assert_NoAssume(obj.RTTI_Outer() == nullptr); // can't reference non-exported, non-local objects
                return true;
            }
            else {
                return false; // don't recurse if already visited
            }

        },
        [linearized](const ITypeTraits&, FMetaObject& obj) {
            Add_AssertUnique(linearized->LoadedRefs, SMetaObject{ &obj }); // postfix order
            return true; // never fails recursion here
        },
        (EVisitorFlags::Default +
            (outer.KeepDeprecated() ? EVisitorFlags::KeepDeprecated : EVisitorFlags{ 0 }) +
            (outer.KeepTransient() ? EVisitorFlags::KeepTransient : EVisitorFlags{ 0 }))
        );

    Assert_NoAssume(linearized->LoadedRefs.size() >= topObjects.size());
    Assert_NoAssume(linearized->ExportedRefs.size() <= linearized->LoadedRefs.size());
}
//----------------------------------------------------------------------------
#if USE_PPE_LOGGER
void ReportLoadedTransaction_(
    const FMetaTransaction& outer,
    const FLinearizedTransaction& linearized) {
    LOG(RTTI, Info, L"loaded transaction '{0}' :", outer.Namespace());

    FWStringBuilder oss;
    Fmt::FWIndent indent = Fmt::FWIndent::UsingTabs();
    indent.Inc();

    oss << indent << L" - Top objects : " << outer.TopObjects().size() << Eol;
    {
        size_t index = 0;
        const Fmt::FWIndent::FScope scopeIndent(indent);
        for (const PMetaObject& obj : outer.TopObjects())
            oss << indent << L'['
            << Fmt::PadLeft(index++, 3, L'0') << L"]  "
            << Fmt::Pointer(obj.get()) << L" : "
            << obj->RTTI_Class()->Name() << L" ("
            << obj->RTTI_Flags() << L')'
            << Eol;

        FLogger::Log(
            LOG_CATEGORY_GET(RTTI),
            FLogger::EVerbosity::Info,
            FLogger::FSiteInfo::Make(WIDESTRING(__FILE__), __LINE__),
            oss.ToString());
    }
    oss << indent << L" - Exported objects : " << linearized.ExportedRefs.size() << Eol;
    {
        size_t index = 0;
        const Fmt::FWIndent::FScope scopeIndent(indent);
        for (const SMetaObject& obj : linearized.ExportedRefs)
            oss << indent << L'['
            << Fmt::PadLeft(index++, 3, L'0') << L"]  "
            << Fmt::Pointer(obj.get()) << L" : "
            << obj->RTTI_Class()->Name() << L" = '"
            << obj->RTTI_Name() << L"' ("
            << obj->RTTI_Flags() << L')'
            << Eol;

        FLogger::Log(
            LOG_CATEGORY_GET(RTTI),
            FLogger::EVerbosity::Info,
            FLogger::FSiteInfo::Make(WIDESTRING(__FILE__), __LINE__),
            oss.ToString());
    }
    oss << indent << L" - Loaded objects : " << linearized.LoadedRefs.size() << Eol;
    {
        size_t index = 0;
        const Fmt::FWIndent::FScope scopeIndent(indent);
        for (const SMetaObject& obj : linearized.LoadedRefs)
            oss << indent << L'['
            << Fmt::PadLeft(index++, 3, L'0') << L"]  "
            << Fmt::Pointer(obj.get()) << L" : "
            << obj->RTTI_Class()->Name() << L" ("
            << obj->RTTI_Flags() << L')'
            << Eol;

        FLogger::Log(
            LOG_CATEGORY_GET(RTTI),
            FLogger::EVerbosity::Info,
            FLogger::FSiteInfo::Make(WIDESTRING(__FILE__), __LINE__),
            oss.ToString());
    }
    oss << indent << L" - Imported objects : " << linearized.ImportedRefs.size() << Eol;
    {
        size_t index = 0;
        const Fmt::FWIndent::FScope scopeIndent(indent);
        for (const SMetaObject& obj : linearized.ImportedRefs)
            oss << indent << L'['
            << Fmt::PadLeft(index++, 3, L'0') << L"]  "
            << Fmt::Pointer(obj.get()) << L" : "
            << obj->RTTI_Class()->Name() << L" = '"
            << obj->RTTI_PathName() << L"' ("
            << obj->RTTI_Flags() << L')'
            << Eol;

        FLogger::Log(
            LOG_CATEGORY_GET(RTTI),
            FLogger::EVerbosity::Info,
            FLogger::FSiteInfo::Make(WIDESTRING(__FILE__), __LINE__),
            oss.ToString());
    }
}
#endif //!USE_PPE_LOGGER
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FTransactionLoadContext : public ILoadContext {
public:
    FTransactionLoadContext(FMetaTransaction& outer, ILoadContext* parent)
    :   _outer(&outer)
    ,   _parent(parent) {
        Assert(outer.IsUnloaded());

        _outer->_state = ETransactionState::Loading;
    }

    ~FTransactionLoadContext() {
        Assert(ETransactionState::Loading == _outer->_state);

#if WITH_PPE_RTTI_TRANSACTION_CHECKS
        for (const SMetaObject& obj : _outer->_linearized.LoadedRefs)
            Assert(obj->RTTI_IsLoaded());
#endif

        _outer->_state = ETransactionState::Loaded;
    }

    virtual void OnLoadObject(FMetaObject& object) override final {
        Assert(object.RTTI_IsLoaded());
        Assert(object.RTTI_IsTopObject() == Contains(_outer->_topObjects, &object));

        object.RTTI_SetOuter(_outer.get(), nullptr);

        if (_parent)
            _parent->OnLoadObject(object);
    }

private:
    const SMetaTransaction _outer;
    ILoadContext* const _parent;
};
//----------------------------------------------------------------------------
class FTransactionUnloadContext : public IUnloadContext {
public:
    FTransactionUnloadContext(FMetaTransaction& outer, IUnloadContext* parent)
    :   _outer(&outer)
    ,   _parent(parent) {
        Assert(outer.IsLoaded());

        _outer->_state = ETransactionState::Unloading;
    }

    ~FTransactionUnloadContext() {
        Assert(ETransactionState::Unloading == _outer->_state);

#if WITH_PPE_RTTI_TRANSACTION_CHECKS
        for (const SMetaObject& obj : _outer->_linearized.LoadedRefs)
            Assert(obj->RTTI_IsUnloaded());
#endif

        _outer->_state = ETransactionState::Unloaded;
    }

    virtual void OnUnloadObject(FMetaObject& object) override final {
        Assert(object.RTTI_IsLoaded());
        Assert(object.RTTI_IsTopObject() == Contains(_outer->_topObjects, &object));

        if (_parent)
            _parent->OnUnloadObject(object);

        object.RTTI_SetOuter(nullptr, _outer.get());
    }

private:
    const SMetaTransaction _outer;
    IUnloadContext* const _parent;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FMetaTransaction::FMetaTransaction(const FName& namespace_, ETransactionFlags flags/* = ETransactionFlags::Default */)
:   _namespace(namespace_)
,   _flags(flags)
,   _state(ETransactionState::Unloaded) {
}
//----------------------------------------------------------------------------
FMetaTransaction::~FMetaTransaction() {
    Assert_NoAssume(not IsLoaded());
    Assert_NoAssume(IsUnloaded());
}
//----------------------------------------------------------------------------
void FMetaTransaction::Add(FMetaObject* object) {
    return Add(PMetaObject{ object });
}
//----------------------------------------------------------------------------
void FMetaTransaction::Add(PMetaObject&& robject) {
    Assert(robject);
    Assert_NoAssume(robject->RTTI_IsUnloaded());
    Assert_NoAssume(IsUnloaded());

    robject->RTTI_MarkAsTopObject();

    Add_AssertUnique(_topObjects, std::move(robject));
}
//----------------------------------------------------------------------------
void FMetaTransaction::Remove(FMetaObject* object) {
    Assert(object);
    Assert_NoAssume(object->RTTI_IsUnloaded());
    Assert_NoAssume(IsUnloaded());

    object->RTTI_UnmarkAsTopObject();

    Remove_AssertExists(_topObjects, PMetaObject(object));
}
//----------------------------------------------------------------------------
void FMetaTransaction::Load(ILoadContext* load/* = nullptr */) {
    Assert_NoAssume(IsUnloaded());

#if WITH_PPE_RTTI_TRANSACTION_CHECKS
    AssertRelease(CheckCircularReferences(_topObjects));
#endif

    // the transaction is linearized when loaded, this simplifies every subsequent task
    LinearizeTransaction_(&_linearized, *this, _topObjects.MakeConstView());

    FTransactionLoadContext loadContext(*this, load);
    for (const SMetaObject& ref : _linearized.LoadedRefs) {
        // don't use RTTI_CallLoadIFN() since we guarantee iteration order
        ref->RTTI_Load(loadContext);
    }

#if USE_PPE_LOGGER
    ReportLoadedTransaction_(*this, _linearized);
#endif //!USE_PPE_LOGGER
}
//----------------------------------------------------------------------------
void FMetaTransaction::Mount() {
    Assert_NoAssume(IsLoaded());

    LOG(RTTI, Info, L"mount transaction '{0}'", _namespace);

    // exports everything to database

    _state = ETransactionState::Mounting;

    const FMetaDatabaseReadWritable metaDB;

    metaDB->RegisterTransaction(this);

    for (const SMetaObject& ref : _linearized.ExportedRefs) {
        Assert_NoAssume(ref->RTTI_Outer() == this);
        metaDB->RegisterObject(ref);
    }

    _state = ETransactionState::Mounted;
}
//----------------------------------------------------------------------------
void FMetaTransaction::Unmount() {
    Assert_NoAssume(IsMounted());

    LOG(RTTI, Info, L"unmount transaction '{0}'", _namespace);

    // unregister everything from database

    _state = ETransactionState::Unmounting;

    const FMetaDatabaseReadWritable metaDB;

    for (const SMetaObject& ref : _linearized.ExportedRefs) {
        Assert_NoAssume(ref->RTTI_Outer() == this);
        metaDB->UnregisterObject(ref);
    }

    metaDB->UnregisterTransaction(this);

    _state = ETransactionState::Loaded;
}
//----------------------------------------------------------------------------
void FMetaTransaction::Unload(IUnloadContext* unload/* = nullptr */) {
    Assert_NoAssume(IsLoaded());
    Assert_NoAssume(_linearized.LoadedRefs.size() >= _topObjects.size());

    LOG(RTTI, Info, L"unload transaction '{0}'", _namespace);

    // unload every loaded object and clear linearized data

    FTransactionUnloadContext unloadContext(*this, unload);
    reverseforeachitem(ref, _linearized.LoadedRefs) {
        // don't use RTTI_CallUnloadIFN() since we guarantee iteration order
        (*ref)->RTTI_Unload(unloadContext);
    }
}
//----------------------------------------------------------------------------
void FMetaTransaction::Reload() {
    LOG(RTTI, Info, L"reloading transaction '{0}' ({1} top objects)",
        _namespace, _topObjects.size() );

    const bool wasMounted = IsMounted();

    if (wasMounted)
        Unmount();

    Unload();
    Load();

    if (wasMounted)
        Mount();
}
//----------------------------------------------------------------------------
void FMetaTransaction::LoadAndMount() {
    Load();
    Mount();
}
//----------------------------------------------------------------------------
void FMetaTransaction::UnmountAndUnload() {
    Unmount();
    Unload();
}
//----------------------------------------------------------------------------
void FMetaTransaction::reserve(size_t capacity) {
    Assert(IsUnloaded());

    _topObjects.reserve(capacity);
}
//----------------------------------------------------------------------------
bool FMetaTransaction::DeepEquals(const FMetaTransaction& other) const {
    return std::equal(
        _topObjects.begin(), _topObjects.end(),
        other._topObjects.begin(), other._topObjects.end(),
        [](const PMetaObject& lhs, const PMetaObject& rhs) {
            Assert(lhs);
            Assert(rhs);
            return RTTI::DeepEquals(*lhs, *rhs);
        });
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FLinearizedTransaction::FLinearizedTransaction() = default;
//----------------------------------------------------------------------------
FLinearizedTransaction::~FLinearizedTransaction() = default;
//----------------------------------------------------------------------------
bool FLinearizedTransaction::HasImport(const FMetaTransaction& other) const {
    Assert_NoAssume(ImportedRefs.size() < 32); // could be a performance issue

    for (const SMetaObject& ref : ImportedRefs) {
        if (ref->RTTI_Outer() == &other)
            return true;
    }
    return false;
}
//----------------------------------------------------------------------------
void FLinearizedTransaction::Reset() {
    ImportedRefs.clear_ReleaseMemory();
    LoadedRefs.clear_ReleaseMemory();
    ExportedRefs.clear_ReleaseMemory();
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
    case RTTI::ETransactionState::Mounting:     return oss << "Mounting";
    case RTTI::ETransactionState::Mounted:      return oss << "Mounted";
    case RTTI::ETransactionState::Unmounting:   return oss << "Unmounting";
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
    case RTTI::ETransactionState::Mounting:     return oss << L"Mounting";
    case RTTI::ETransactionState::Mounted:      return oss << L"Mounted";
    case RTTI::ETransactionState::Unmounting:   return oss << L"Unmounting";
    case RTTI::ETransactionState::Unloading:    return oss << L"Unloading";
    }
    AssertNotReached();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
