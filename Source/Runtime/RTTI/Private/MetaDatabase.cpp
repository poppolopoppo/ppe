#include "stdafx.h"

#include "MetaDatabase.h"

#include "MetaClass.h"
#include "MetaEnum.h"
#include "MetaModule.h"
#include "MetaObject.h"
#include "MetaTransaction.h"
#include "RTTI/NativeTypes.h"
#include "RTTI/OpaqueData.h"

#include "Diagnostic/Logger.h"
#include "IO/TextWriter.h"

namespace PPE {
namespace RTTI {
EXTERN_LOG_CATEGORY(PPE_RTTI_API, RTTI)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void* FMetaDatabase::class_singleton_storage() NOEXCEPT {
    return singleton_type::make_singleton_storage(); // for shared libs
}
//----------------------------------------------------------------------------
FMetaDatabase::FMetaDatabase() {
    LOG(RTTI, Info, L"create meta database");

    InitializeNativeTypes_();
}
//----------------------------------------------------------------------------
FMetaDatabase::~FMetaDatabase() {
    LOG(RTTI, Info, L"destroy meta database");

    Assert(_transactions.empty());
    Assert(_objects.empty());
    Assert(_classes.empty());
}
//----------------------------------------------------------------------------
// Transactions
//----------------------------------------------------------------------------
void FMetaDatabase::RegisterTransaction(const FMetaTransaction* metaTransaction) {
    Assert(metaTransaction);
    Assert(metaTransaction->IsMounting());

    const FName& namespace_ = metaTransaction->Namespace();
    Assert(not namespace_.empty());

    LOG(RTTI, Info, L"register transaction in DB : namespace <'{0}'>", namespace_);

    PPE_LEAKDETECTOR_WHITELIST_SCOPE();

    Add_AssertUnique(_transactions.FindOrAdd(namespace_), SCMetaTransaction{ metaTransaction });
}
//----------------------------------------------------------------------------
void FMetaDatabase::UnregisterTransaction(const FMetaTransaction* metaTransaction) {
    Assert(metaTransaction);
    Assert(metaTransaction->IsUnmounting());

    const FName& namespace_ = metaTransaction->Namespace();
    Assert(not namespace_.empty());

    LOG(RTTI, Info, L"unregister transaction in DB : '{0}'", namespace_);

#if USE_PPE_ASSERT
    // Check that all objects from this transaction were unregistered
    for (const auto& it : _objects)
        Assert(it.second->RTTI_Outer() != metaTransaction);
#endif

    PPE_LEAKDETECTOR_WHITELIST_SCOPE();

    const auto it = _transactions.find(namespace_);
    Remove_AssertExists(it->second, SCMetaTransaction{ metaTransaction });

    if (it->second.empty())
        _transactions.erase(it);
}
//----------------------------------------------------------------------------
TMemoryView<const SCMetaTransaction> FMetaDatabase::Transaction(const FName& namespace_) const {
    Assert(not namespace_.empty());

    const TMemoryView<const SCMetaTransaction> v{ _transactions.at(namespace_) };

#if USE_PPE_ASSERT
    for (const SCMetaTransaction& t : v) {
        Assert_NoAssume(t->IsLoaded());
    }
#endif

    return v;
}
//----------------------------------------------------------------------------
TMemoryView<const SCMetaTransaction> FMetaDatabase::TransactionIFP(const FName& namespace_) const {
    Assert(not namespace_.empty());

    TMemoryView<const SCMetaTransaction> v;

    const auto it = _transactions.find(namespace_);
    if (_transactions.end() != it)
        v = it->second;

#if USE_PPE_ASSERT
    for (const SCMetaTransaction& t : v) {
        Assert_NoAssume(t->IsMounted());
    }
#endif

    return v;
}
//----------------------------------------------------------------------------
TMemoryView<const SCMetaTransaction> FMetaDatabase::TransactionIFP(const FStringView& namespace_) const {
    return TransactionIFP(FLazyName{ namespace_ });
}
//----------------------------------------------------------------------------
TMemoryView<const SCMetaTransaction> FMetaDatabase::TransactionIFP(const FLazyName& namespace_) const {
    Assert(not namespace_.empty());

    TMemoryView<const SCMetaTransaction> v;

    const auto it = _transactions.find_like(namespace_, namespace_.HashValue());
    if (_transactions.end() != it)
        v = it->second;

#if USE_PPE_ASSERT
    for (const SCMetaTransaction& t : v) {
        Assert_NoAssume(t->IsMounted());
    }
#endif

    return v;
}
//----------------------------------------------------------------------------
// Objects
//----------------------------------------------------------------------------
void FMetaDatabase::RegisterObject(FMetaObject* metaObject) {
    Assert(metaObject);
    Assert_NoAssume(metaObject->RTTI_Outer());
    Assert_NoAssume(metaObject->RTTI_Outer()->IsMounting());

    const FPathName exportPath{ FPathName::FromObject(*metaObject) };

    LOG(RTTI, Info, L"register object in DB : <{0}::{1}> '{2}'",
        metaObject->RTTI_Class()->Module()->Name(),
        metaObject->RTTI_Class()->Name(),
        exportPath );

    Assert_NoAssume(metaObject->RTTI_IsExported());
    Assert_NoAssume(metaObject->RTTI_IsLoaded());

    Assert_NoAssume(Contains(_modules, metaObject->RTTI_Class()->Module()));
    Assert_NoAssume(_transactions.Contains(exportPath.Namespace));

    PPE_LEAKDETECTOR_WHITELIST_SCOPE();

    Insert_AssertUnique(_objects, exportPath, SMetaObject(metaObject));
}
//----------------------------------------------------------------------------
void FMetaDatabase::UnregisterObject(FMetaObject* metaObject) {
    Assert(metaObject);
    Assert_NoAssume(metaObject->RTTI_Outer());
    Assert_NoAssume(metaObject->RTTI_Outer()->IsUnmounting());

    const FPathName exportPath{ FPathName::FromObject(*metaObject) };

    LOG(RTTI, Info, L"unregister object from DB : <{0}::{1}> '{2}'",
        metaObject->RTTI_Class()->Module()->Name(),
        metaObject->RTTI_Class()->Name(),
        exportPath );

    Assert_NoAssume(metaObject->RTTI_IsExported());
    Assert_NoAssume(metaObject->RTTI_IsLoaded());

    Assert_NoAssume(Contains(_modules, metaObject->RTTI_Class()->Module()));
    Assert_NoAssume(_transactions.Contains(exportPath.Namespace));

    PPE_LEAKDETECTOR_WHITELIST_SCOPE();

#if USE_PPE_ASSERT
    Remove_AssertExistsAndSameValue(_objects, exportPath, SMetaObject{ metaObject });
#else
    _objects.erase(exportPath);
#endif
}
//----------------------------------------------------------------------------
FMetaObject& FMetaDatabase::Object(const FPathName& pathName) const {
    Assert(not pathName.empty());
    Assert(not pathName.Namespace.empty());

#if USE_PPE_ASSERT
    return *ObjectIFP(pathName); // profit from additional debugging features
#else
    FMetaObject& obj = (*_objects.at(pathName));
    Assert(obj.RTTI_IsLoaded());

    return obj;
#endif
}
//----------------------------------------------------------------------------
FMetaObject* FMetaDatabase::ObjectIFP(const FPathName& pathName) const {
    Assert(not pathName.empty());
    Assert(not pathName.Namespace.empty());

    const auto it = _objects.find(pathName);
    if (_objects.end() != it) {
        Assert_NoAssume(it->second->RTTI_IsLoaded());
        return it->second.get();
    }
    else {
#if USE_PPE_ASSERT
        CLOG(TransactionIFP(pathName.Namespace).empty(),
            RTTI, Error, L"trying to fetch an object from an unmounted transaction : {0}", pathName);
#endif
        return nullptr;
    }
}
//----------------------------------------------------------------------------
FMetaObject* FMetaDatabase::ObjectIFP(const FStringView& text) const {
    Assert(not text.empty());

    FLazyPathName pathName;
    VerifyRelease(FLazyPathName::Parse(&pathName, text));

    return ObjectIFP(pathName);
}
//----------------------------------------------------------------------------
FMetaObject* FMetaDatabase::ObjectIFP(const FStringView& namespace_, const FStringView& identifier) const {
    return ObjectIFP(FLazyPathName{ FLazyName(namespace_),FLazyName(identifier) });
}
//----------------------------------------------------------------------------
FMetaObject* FMetaDatabase::ObjectIFP(const FLazyName& namespace_, const FLazyName& identifier) const {
    return ObjectIFP(FLazyPathName{ namespace_, identifier });
}
//----------------------------------------------------------------------------
FMetaObject* FMetaDatabase::ObjectIFP(const FLazyPathName& pathName) const {
	Assert(not pathName.empty());
	Assert(not pathName.Namespace.empty());

	const auto it = _objects.find_like(pathName, hash_value(pathName));
	if (_objects.end() != it) {
		Assert_NoAssume(it->second->RTTI_IsLoaded());
		return it->second.get();
	}
	else {
#if USE_PPE_ASSERT
		CLOG(TransactionIFP(pathName.Namespace).empty(),
			RTTI, Error, L"trying to fetch an object from an unmounted transaction : {0}", pathName);
#endif
		return nullptr;
	}
}
//----------------------------------------------------------------------------
// Modules
//----------------------------------------------------------------------------
void FMetaDatabase::RegisterModule(const FMetaModule* metaModule) {
    Assert(metaModule);

    LOG(RTTI, Info, L"register module in DB : <{0}>", metaModule->Name());

    Assert(metaModule->IsStarted());

    PPE_LEAKDETECTOR_WHITELIST_SCOPE();

    Add_AssertUnique(_modules, metaModule);

    for (const FMetaEnum* metaEnum : metaModule->Enums()) {
        Assert(metaEnum);
        Assert(metaEnum->Module() == metaModule);

        Insert_AssertUnique(_enums, metaEnum->Name(), metaEnum);
        RegisterTraits_(metaEnum->Name(), metaEnum->MakeTraits());
    }

    for (const FMetaClass* metaClass : metaModule->Classes()) {
        Assert(metaClass);
        Assert(metaClass->IsRegistered());
        Assert(metaClass->Module() == metaModule);

        Insert_AssertUnique(_classes, metaClass->Name(), metaClass);
        RegisterTraits_(metaClass->Name(), metaClass->MakeTraits());
    }
}
//----------------------------------------------------------------------------
void FMetaDatabase::UnregisterModule(const FMetaModule* metaModule) {
    Assert(metaModule);

    LOG(RTTI, Info, L"unregister module from DB : <{0}>", metaModule->Name());

    Assert(metaModule->IsStarted());

    PPE_LEAKDETECTOR_WHITELIST_SCOPE();

    for (const FMetaClass* metaClass : metaModule->Classes()) {
        Assert(metaClass);
        Assert(metaClass->IsRegistered());
        Assert(metaClass->Module() == metaModule);

        UnregisterTraits_(metaClass->Name(), metaClass->MakeTraits());
        Remove_AssertExistsAndSameValue(_classes, metaClass->Name(), metaClass);
    }

    for (const FMetaEnum* metaEnum : metaModule->Enums()) {
        Assert(metaEnum);
        Assert(metaEnum->Module() == metaModule);

        UnregisterTraits_(metaEnum->Name(), metaEnum->MakeTraits());
        Remove_AssertExistsAndSameValue(_enums, metaEnum->Name(), metaEnum);
    }

    Remove_AssertExists(_modules, metaModule);

#if USE_PPE_ASSERT
    // Check that no object belonging to this namespace if still referenced
    for (const auto& it : _objects) {
        const FMetaClass* metaClass = it.second->RTTI_Class();
        Assert(metaClass->Module() != metaModule);
    }
#endif
}
//----------------------------------------------------------------------------
const FMetaModule& FMetaDatabase::Module(const FName& name) const {
    Assert(not name.empty());

    const auto it = std::find_if(_modules.begin(), _modules.end(), [&name](const FMetaModule* metaModule) {
        return (metaModule->Name() == name);
    });
    AssertRelease(_modules.end() != it);

    return (**it);
}
//----------------------------------------------------------------------------
const FMetaModule* FMetaDatabase::ModuleIFP(const FName& name) const {
    Assert(not name.empty());

    const auto it = std::find_if(_modules.begin(), _modules.end(), [&name](const FMetaModule* metaModule) {
        return (metaModule->Name() == name);
    });

    return (_modules.end() == it ? nullptr : *it);
}
//----------------------------------------------------------------------------
const FMetaModule* FMetaDatabase::ModuleIFP(const FStringView& name) const {
    return ModuleIFP(FLazyName{ name });
}
//----------------------------------------------------------------------------
const FMetaModule* FMetaDatabase::ModuleIFP(const FLazyName& name) const {
    Assert(not name.empty());

    const auto it = std::find_if(_modules.begin(), _modules.end(), [&name](const FMetaModule* metaModule) {
        return (metaModule->Name() == name);
    });

    return (_modules.end() == it ? nullptr : *it);
}
//----------------------------------------------------------------------------
// Classes
//----------------------------------------------------------------------------
const FMetaClass& FMetaDatabase::Class(const FName& name) const {
    Assert(not name.empty());

    return (*_classes.at(name));
}
//----------------------------------------------------------------------------
const FMetaClass* FMetaDatabase::ClassIFP(const FName& name) const {
    Assert(not name.empty());

    const auto it = _classes.find(name);

    return (_classes.end() == it ? nullptr : it->second);
}
//----------------------------------------------------------------------------
const FMetaClass* FMetaDatabase::ClassIFP(const FStringView& name) const {
    return ClassIFP(FLazyName{ name });
}
//----------------------------------------------------------------------------
const FMetaClass* FMetaDatabase::ClassIFP(const FLazyName& name) const {
    Assert(not name.empty());

    const auto it = _classes.find_like(name, name.HashValue());

    return (_classes.end() == it ? nullptr : it->second);
}
//----------------------------------------------------------------------------
// Enums
//----------------------------------------------------------------------------
const FMetaEnum& FMetaDatabase::Enum(const FName& name) const {
    Assert(not name.empty());

    return (*_enums.at(name));
}
//----------------------------------------------------------------------------
const FMetaEnum* FMetaDatabase::EnumIFP(const FName& name) const {
    Assert(not name.empty());

    const auto it = _enums.find(name);
    return (_enums.end() == it ? nullptr : it->second);
}
//----------------------------------------------------------------------------
const FMetaEnum* FMetaDatabase::EnumIFP(const FStringView& name) const {
    return EnumIFP(FLazyName{ name });
}
//----------------------------------------------------------------------------
const FMetaEnum* FMetaDatabase::EnumIFP(const FLazyName& name) const {
    Assert(not name.empty());

    const auto it = _enums.find_like(name, name.HashValue());
    return (_enums.end() == it ? nullptr : it->second);
}
//----------------------------------------------------------------------------
// Traits
//----------------------------------------------------------------------------
const PTypeTraits& FMetaDatabase::Traits(const FName& name) const {
    Assert(not name.empty());

    return (_traits.at(name));
}
//----------------------------------------------------------------------------
PTypeTraits FMetaDatabase::TraitsIFP(const FName& name) const {
    Assert(not name.empty());

    const auto it = _traits.find(name);
    return (_traits.end() == it ? PTypeTraits{} : it->second);
}
//----------------------------------------------------------------------------
PTypeTraits FMetaDatabase::TraitsIFP(const FStringView& name) const {
    return TraitsIFP(FLazyName{ name });
}
//----------------------------------------------------------------------------
PTypeTraits FMetaDatabase::TraitsIFP(const FLazyName& name) const {
    Assert(not name.empty());

    const auto it = _traits.find_like(name, name.HashValue());
    return (_traits.end() == it ? PTypeTraits{} : it->second);
}
//----------------------------------------------------------------------------
void FMetaDatabase::InitializeNativeTypes_() {
#define RegisterNativeType_(_NAME, _TYPE, _TYPEID) \
    RegisterTraits_(RTTI::FName(STRINGIZE(_NAME)), MakeTraits(ENativeType::_NAME));

    FOREACH_RTTI_NATIVETYPES(RegisterNativeType_)

#undef RegisterNativeType_
#define RegisterAliasedType_(_NAME, _TYPE) \
    RegisterTraits_(RTTI::FName(STRINGIZE(_NAME)), MakeTraits<_TYPE>())

    RegisterAliasedType_(Byte2, byte2);
    RegisterAliasedType_(Byte4, byte4);
    RegisterAliasedType_(UByte2, ubyte2);
    RegisterAliasedType_(UByte4, ubyte4);
    RegisterAliasedType_(Short2, short2);
    RegisterAliasedType_(Short4, short4);
    RegisterAliasedType_(UShort2, ushort2);
    RegisterAliasedType_(UShort4, ushort4);
    RegisterAliasedType_(Word2, word2);
    RegisterAliasedType_(Word3, word3);
    RegisterAliasedType_(Word4, word4);
    RegisterAliasedType_(UWord2, uword2);
    RegisterAliasedType_(UWord3, uword3);
    RegisterAliasedType_(UWord4, uword4);
    RegisterAliasedType_(Float2, float2);
    RegisterAliasedType_(Float3, float3);
    RegisterAliasedType_(Float4, float4);
    RegisterAliasedType_(Float2x2, float2x2);
    RegisterAliasedType_(Float3x3, float3x3);
    RegisterAliasedType_(Float4x4, float4x4);

    RegisterAliasedType_(PathName, FPathName);

    RegisterAliasedType_(OpaqueArray, FOpaqueArray);
    RegisterAliasedType_(OpaqueData, FOpaqueData);

#undef RegisterAliasedType_
}
//----------------------------------------------------------------------------
void FMetaDatabase::RegisterTraits_(const FName& name, const PTypeTraits& traits) {
    Assert_NoAssume(not name.empty());
    Assert(traits);

    const auto result = _traits.insert(MakePair(name, traits));
    if (not result.second)
        Assert_NoAssume(result.first->second == traits);
}
//----------------------------------------------------------------------------
void FMetaDatabase::UnregisterTraits_(const FName& name, const PTypeTraits& traits) {
    Assert_NoAssume(not name.empty());
    Assert(traits);

    Remove_AssertExistsAndSameValue(_traits, name, traits);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FMetaDatabaseReadable::FMetaDatabaseReadable()
:   _db(FMetaDatabase::Get()/* Get() is not public */) {
    _db._lockRW.LockRead();
}
//----------------------------------------------------------------------------
FMetaDatabaseReadable::~FMetaDatabaseReadable() {
    _db._lockRW.UnlockRead();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FMetaDatabaseReadWritable::FMetaDatabaseReadWritable()
:   _db(FMetaDatabase::Get()/* Get() is not public */) {
    _db._lockRW.LockWrite();
}
//----------------------------------------------------------------------------
FMetaDatabaseReadWritable::~FMetaDatabaseReadWritable() {
    _db._lockRW.UnlockWrite();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE
