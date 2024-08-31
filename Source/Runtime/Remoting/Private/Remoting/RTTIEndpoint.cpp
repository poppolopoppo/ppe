// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Remoting/RTTIEndpoint.h"

#include "Remoting/OpenAPI.h"

#include "MetaClass.h"
#include "MetaDatabase.h"
#include "MetaEnum.h"
#include "MetaFunction.h"
#include "MetaModule.h"
#include "MetaProperty.h"
#include "MetaTransaction.h"
#include "RTTI/Any.h"
#include "RTTI/Exceptions.h"
#include "RTTI/Macros-impl.h"
#include "RTTI/TypeTraits.h"

#include "IO/String.h"
#include "IO/StringBuilder.h"

namespace PPE {
namespace Remoting {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Traits cache
//----------------------------------------------------------------------------
template <typename _Wrapper, typename _Traits>
static void WrapTraits_(PRTTITraits& cached, FRTTITraitsCache& cache, const _Traits& traits) {
    auto concrete = NEW_RTTI(_Wrapper);
    cached = concrete; // need 2 phase initialization here to avoid infinite recursion
    concrete->Initialize(cache, traits);
}
//----------------------------------------------------------------------------
static PRTTITraits FindOrAddTraits_(FRTTITraitsCache& cache, const RTTI::PTypeTraits& traits) {
    Assert(traits);

    PRTTITraits& cached = cache.FindOrAdd(traits);

    if (not cached) {
        if (const RTTI::IScalarTraits* scalar = traits->AsScalar())
            WrapTraits_<FRTTITraitsScalar>(cached, cache, *scalar);
        else if (const RTTI::ITupleTraits* tuple = traits->AsTuple())
            WrapTraits_<FRTTITraitsTuple>(cached, cache, *tuple);
        else if (const RTTI::IListTraits* list = traits->AsList())
            WrapTraits_<FRTTITraitsList>(cached, cache, *list);
        else if (const RTTI::IDicoTraits* dico = traits->AsDico())
            WrapTraits_<FRTTITraitsDico>(cached, cache, *dico);
        else
            AssertNotImplemented();

        return cache[traits]; // cached& may be invalid here due to recursion
    }

    return cached;
}
//----------------------------------------------------------------------------
static PRTTIClass FindOrAddClass_(FRTTITraitsCache& cache, const RTTI::FMetaClass& class_) {
    const PRTTITraits wrapped = FindOrAddTraits_(cache, class_.MakeTraits());
    Assert(wrapped);
    const PRTTIClass result = checked_cast<FRTTITraitsScalar>(wrapped)->ObjectClass;
    Assert(result);
    return result;
}
//----------------------------------------------------------------------------
static PRTTIEnum FindOrAddEnum_(FRTTITraitsCache& cache, const RTTI::FMetaEnum& enum_) {
    const PRTTITraits wrapped = FindOrAddTraits_(cache, enum_.MakeTraits());
    Assert(wrapped);
    const PRTTIEnum result = checked_cast<FRTTITraitsScalar>(wrapped)->EnumClass;
    Assert(result);
    return result;
}
//----------------------------------------------------------------------------
// RTTI meta-data
//----------------------------------------------------------------------------
RTTI_CLASS_BEGIN(Remoting, FRTTIParameter, Concrete)
RTTI_PROPERTY_PUBLIC_FIELD(Name)
RTTI_PROPERTY_PUBLIC_FIELD(Flags)
RTTI_PROPERTY_PUBLIC_FIELD(Traits)
RTTI_CLASS_END()
FRTTIParameter::FRTTIParameter(FRTTITraitsCache& cache, const RTTI::FMetaParameter& value) {
    Name = value.Name();
    Flags = value.Flags();
    Traits = FindOrAddTraits_(cache, value.Traits());
}
//----------------------------------------------------------------------------
RTTI_CLASS_BEGIN(Remoting, FRTTIFunction, Concrete)
RTTI_PROPERTY_PUBLIC_FIELD(Name)
RTTI_PROPERTY_PUBLIC_FIELD(Flags)
RTTI_PROPERTY_PUBLIC_FIELD(Result)
RTTI_PROPERTY_PUBLIC_FIELD(Parameters)
RTTI_CLASS_END()
FRTTIFunction::FRTTIFunction(FRTTITraitsCache& cache, const RTTI::FMetaFunction& value) {
    Name = value.Name();
    Flags = value.Flags();
    Parameters.assign(value.Parameters().Iterable().Map([&cache](const RTTI::FMetaParameter& prm) {
        return NEW_RTTI(FRTTIParameter, cache, prm);
    }));

    if (value.HasReturnValue())
        Result = FindOrAddTraits_(cache, value.Result());
}
//----------------------------------------------------------------------------
RTTI_CLASS_BEGIN(Remoting, FRTTIProperty, Concrete)
RTTI_PROPERTY_PUBLIC_FIELD(Name)
RTTI_PROPERTY_PUBLIC_FIELD(Flags)
RTTI_PROPERTY_PUBLIC_FIELD(Traits)
RTTI_CLASS_END()
FRTTIProperty::FRTTIProperty(FRTTITraitsCache& cache, const RTTI::FMetaProperty& value) {
    Name = value.Name();
    Flags = value.Flags();
    Traits = FindOrAddTraits_(cache, value.Traits());
}
//----------------------------------------------------------------------------
RTTI_CLASS_BEGIN(Remoting, FRTTIClass, Concrete)
RTTI_PROPERTY_PUBLIC_FIELD(Name)
RTTI_PROPERTY_PUBLIC_FIELD(Flags)
RTTI_PROPERTY_PUBLIC_FIELD(Functions)
RTTI_PROPERTY_PUBLIC_FIELD(Properties)
RTTI_PROPERTY_PUBLIC_FIELD(Parent)
RTTI_CLASS_END()
FRTTIClass::FRTTIClass(FRTTITraitsCache& cache, const RTTI::FMetaClass& value) {
    Name = value.Name();
    Flags = value.Flags();
    Functions.assign(value.SelfFunctions().Map([&cache](const RTTI::FMetaFunction& fn) {
        return NEW_RTTI(FRTTIFunction, cache, fn);
    }));
    Properties.assign(value.SelfProperties().Map([&cache](const RTTI::FMetaProperty& prp) {
        return NEW_RTTI(FRTTIProperty, cache, prp);
    }));

    if (const RTTI::FMetaClass* parent = value.Parent())
        Parent = FindOrAddClass_(cache, *parent);
}
//----------------------------------------------------------------------------
RTTI_CLASS_BEGIN(Remoting, FRTTIEnum, Concrete)
RTTI_PROPERTY_PUBLIC_FIELD(Name)
RTTI_PROPERTY_PUBLIC_FIELD(Flags)
RTTI_PROPERTY_PUBLIC_FIELD(Traits)
RTTI_PROPERTY_PUBLIC_FIELD(Keys)
RTTI_PROPERTY_PUBLIC_FIELD(Values)
RTTI_CLASS_END()
FRTTIEnum::FRTTIEnum(FRTTITraitsCache& cache, const RTTI::FMetaEnum& value) {
    Name = value.Name();
    Flags = value.Flags();
    Traits = FindOrAddTraits_(cache, value.MakeTraits());
    Keys.assign(value.Values().Map([](const RTTI::FMetaEnumValue& it) {
        return it.Name;
    }));
    Values.assign(value.Values().Map([](const RTTI::FMetaEnumValue& it) {
        return it.Ord;
    }));
}
//----------------------------------------------------------------------------
RTTI_CLASS_BEGIN(Remoting, FRTTITraits, Abstract)
RTTI_PROPERTY_PUBLIC_FIELD(Name)
RTTI_PROPERTY_PUBLIC_FIELD(Flags)
RTTI_PROPERTY_PUBLIC_FIELD(TypeId)
RTTI_PROPERTY_PUBLIC_FIELD(Alignment)
RTTI_PROPERTY_PUBLIC_FIELD(SizeInBytes)
RTTI_CLASS_END()
void FRTTITraits::Initialize(const RTTI::ITypeTraits& traits) {
    Name = traits.TypeName();
    Flags = traits.TypeFlags();
    TypeId = traits.TypeId();
    Alignment = checked_cast<u32>(traits.Alignment());
    SizeInBytes = checked_cast<u32>(traits.SizeInBytes());
}
//----------------------------------------------------------------------------
RTTI_CLASS_BEGIN(Remoting, FRTTITraitsScalar, Concrete)
RTTI_PROPERTY_PUBLIC_FIELD(ObjectClass)
RTTI_PROPERTY_PUBLIC_FIELD(EnumClass)
RTTI_CLASS_END()
void FRTTITraitsScalar::Initialize(FRTTITraitsCache& cache, const RTTI::IScalarTraits& traits) {
    FRTTITraits::Initialize(traits);
    if (const RTTI::FMetaClass* const class_ = traits.ObjectClass())
        ObjectClass = NEW_RTTI(FRTTIClass, cache, *class_);
    if (const RTTI::FMetaEnum* const enum_ = traits.EnumClass())
        EnumClass = NEW_RTTI(FRTTIEnum, cache, *enum_);
}
//----------------------------------------------------------------------------
RTTI_CLASS_BEGIN(Remoting, FRTTITraitsTuple, Concrete)
RTTI_PROPERTY_PUBLIC_FIELD(Tuple)
RTTI_CLASS_END()
void FRTTITraitsTuple::Initialize(FRTTITraitsCache& cache, const RTTI::ITupleTraits& traits) {
    FRTTITraits::Initialize(traits);
    Tuple.assign(traits.TupleTraits().Map([&cache](const RTTI::PTypeTraits& it) {
        return FindOrAddTraits_(cache, it);
    }));
}
//----------------------------------------------------------------------------
RTTI_CLASS_BEGIN(Remoting, FRTTITraitsList, Concrete)
RTTI_PROPERTY_PUBLIC_FIELD(Item)
RTTI_CLASS_END()
void FRTTITraitsList::Initialize(FRTTITraitsCache& cache, const RTTI::IListTraits& traits) {
    FRTTITraits::Initialize(traits);
    Item = FindOrAddTraits_(cache, traits.ValueTraits());
}
//----------------------------------------------------------------------------
RTTI_CLASS_BEGIN(Remoting, FRTTITraitsDico, Concrete)
RTTI_PROPERTY_PUBLIC_FIELD(Key)
RTTI_PROPERTY_PUBLIC_FIELD(Value)
RTTI_CLASS_END()
void FRTTITraitsDico::Initialize(FRTTITraitsCache& cache, const RTTI::IDicoTraits& traits) {
    FRTTITraits::Initialize(traits);
    Key = FindOrAddTraits_(cache, traits.KeyTraits());
    Value = FindOrAddTraits_(cache, traits.ValueTraits());
}
//----------------------------------------------------------------------------
RTTI_CLASS_BEGIN(Remoting, FRTTIModule, Concrete)
RTTI_PROPERTY_PUBLIC_FIELD(Name)
RTTI_PROPERTY_PUBLIC_FIELD(Classes)
RTTI_PROPERTY_PUBLIC_FIELD(Enums)
RTTI_CLASS_END()
FRTTIModule::FRTTIModule(FRTTITraitsCache& cache, const RTTI::FMetaModule& module) {
    Name = module.Name();
    Classes.assign(module.Classes().Map(
        [&cache](const RTTI::FMetaClass* class_) {
            return FindOrAddClass_(cache, *class_);
        }));
    Enums.assign(module.Enums().Map(
        [&cache](const RTTI::FMetaEnum* enum_) {
            return FindOrAddEnum_(cache, *enum_);
        }));
}
//----------------------------------------------------------------------------
RTTI_CLASS_BEGIN(Remoting, FRTTITransaction, Concrete)
RTTI_PROPERTY_PUBLIC_FIELD(Namespace)
RTTI_PROPERTY_PUBLIC_FIELD(Flags)
RTTI_PROPERTY_PUBLIC_FIELD(State)
RTTI_PROPERTY_PUBLIC_FIELD(TopObjects)
RTTI_CLASS_END()
FRTTITransaction::FRTTITransaction(const RTTI::FMetaTransaction& transaction) {
    Namespace = transaction.Namespace();
    Flags = transaction.Flags();
    State = transaction.State();
    TopObjects.assign(transaction.TopObjects().MakeView());
}
//----------------------------------------------------------------------------
// FRTTIEndpoint
//----------------------------------------------------------------------------
RTTI_CLASS_BEGIN(Remoting, FRTTIEndpoint, Concrete)
RTTI_FUNCTION_FACET(Traits, (), FOperationFacet::Get("traits", { }))
RTTI_FUNCTION_FACET(Trait, (name), FOperationFacet::Get("trait", { "{name}" }))
RTTI_FUNCTION_FACET(Objects, (), FOperationFacet::Get("objects", { }))
RTTI_FUNCTION_FACET(Object, (path), FOperationFacet::Get("object", { "{path}" }))
RTTI_FUNCTION_FACET(Object_Function, (path, function, args), FOperationFacet::Get("object/function", { "{path}", "{function}" }))
RTTI_FUNCTION_FACET(Object_Property, (path, property), FOperationFacet::Get("object/property", { "{path}", "{property}" }))
RTTI_FUNCTION_FACET(Namespaces, (), FOperationFacet::Get("namespaces", { }))
RTTI_FUNCTION_FACET(Transactions, (namespace), FOperationFacet::Get("transactions", { "{namespace}" }))
RTTI_FUNCTION_FACET(Modules, (), FOperationFacet::Get("modules", { }))
RTTI_FUNCTION_FACET(Module, (name), FOperationFacet::Get("module", { "{name}" }))
RTTI_FUNCTION_FACET(Classes, (), FOperationFacet::Get("classes", { }))
RTTI_FUNCTION_FACET(Class, (name), FOperationFacet::Get("class", { "{name}" }))
RTTI_FUNCTION_FACET(Enums, (), FOperationFacet::Get("enums", { }))
RTTI_FUNCTION_FACET(Enum, (name), FOperationFacet::Get("enum", { "{name}" }))
RTTI_CLASS_END()
//----------------------------------------------------------------------------
FRTTIEndpoint::FRTTIEndpoint() NOEXCEPT
:   FBaseEndpoint("/rtti")
{}
//----------------------------------------------------------------------------
// Traits
//----------------------------------------------------------------------------
FRTTIEndpoint::TList<FString> FRTTIEndpoint::Traits() const NOEXCEPT {
    const RTTI::FMetaDatabaseReadable db;
    return { db->Traits().Map(
        [](const auto& it) {
            return ToString(it.first);
        })
    };
}
//----------------------------------------------------------------------------
PRTTITraits FRTTIEndpoint::Trait(const FString& name) const {
    const RTTI::FMetaDatabaseReadable db;
    if (const RTTI::PTypeTraits traits = db->TraitsIFP(RTTI::FLazyName{ name.MakeView() }))
        return FindOrAddTraits_(*_cache.LockExclusive(), traits);

    PPE_THROW_IT(RTTI::FRTTIException("traits not found"));
}
//----------------------------------------------------------------------------
// Objects
//----------------------------------------------------------------------------
FRTTIEndpoint::TList<FString> FRTTIEndpoint::Objects() const NOEXCEPT {
    const RTTI::FMetaDatabaseReadable db;
    return { db->Objects().Keys().Map(
        [](const RTTI::FPathName& path) {
            FStringBuilder sb;
            sb << path;
            return sb.ToString();
        })
    };
}
//----------------------------------------------------------------------------
RTTI::PMetaObject FRTTIEndpoint::Object(const FString& path) const {
    RTTI::FLazyPathName key;
    if (RTTI::FLazyPathName::Parse(&key, path)) {
        const RTTI::FMetaDatabaseReadable db;
        return RTTI::PMetaObject(db->ObjectIFP(key));
    }

    PPE_THROW_IT(RTTI::FRTTIException("object not found"));
}
//----------------------------------------------------------------------------
RTTI::FAny FRTTIEndpoint::Object_Function(const FString& path, const FString& function, const VECTOR(Remoting, RTTI::FAny)& args) const {
    if (const RTTI::PMetaObject pObj = Object(path)) {
        const RTTI::FMetaFunction* pFunc = nullptr;
        if (pObj->RTTI_Function(function, &pFunc)) {
            RTTI::FAny result;
            RTTI::FAtom atomResult;
            if (pFunc->HasReturnValue()) {
                result.Reset(pFunc->Result());
                atomResult = result.InnerAtom();
            }

            STACKLOCAL_POD_ARRAY(RTTI::FAtom, prms, args.size());
            args.MakeView().Map([](const RTTI::FAny& arg) NOEXCEPT {
                return arg.InnerAtom();
            }).CopyTo(prms.begin());

            if (pFunc->InvokeCopy(*pObj, atomResult, prms))
                return result;

            PPE_THROW_IT(RTTI::FFunctionException("failed to invoke function", pFunc));
        }

        PPE_THROW_IT(RTTI::FObjectException("function not found", pObj.get()));
    }

    PPE_THROW_IT(RTTI::FRTTIException("object not found"));
}
//----------------------------------------------------------------------------
RTTI::FAny FRTTIEndpoint::Object_Property(const FString& path, const FString& property) const {
    if (const RTTI::PMetaObject pObj = Object(path)) {
        const RTTI::FMetaProperty* pProp = nullptr;
        if (pObj->RTTI_Property(property, &pProp)) {
            RTTI::FAny result;
            result.AssignCopy(pProp->Get(*pObj));
            return result;
        }

        PPE_THROW_IT(RTTI::FObjectException("property not found", pObj.get()));
    }

    PPE_THROW_IT(RTTI::FRTTIException("object not found"));
}
//----------------------------------------------------------------------------
// Transaction
//----------------------------------------------------------------------------
FRTTIEndpoint::TList<FString> FRTTIEndpoint::Namespaces() const NOEXCEPT {
    const RTTI::FMetaDatabaseReadable db;
    return { db->Namespaces().Map(
        [](const RTTI::FName& name) {
            return ToString(name);
        })
    };
}
//----------------------------------------------------------------------------
FRTTIEndpoint::TList<PRTTITransaction> FRTTIEndpoint::Transactions(const FString& namespace_) const {
    if (RTTI::FName::IsValidToken(namespace_)) {
        const RTTI::FMetaDatabaseReadable db;
        TList<PRTTITransaction> result;
        result.assign(db->TransactionIFP(namespace_)
            .Map([](const RTTI::SCMetaTransaction& transaction) {
                return NEW_RTTI(FRTTITransaction, *transaction);
            }));
        return result;
    }

    PPE_THROW_IT(RTTI::FRTTIException("namespace not found"));
}
//----------------------------------------------------------------------------
// Modules
//----------------------------------------------------------------------------
FRTTIEndpoint::TList<FString> FRTTIEndpoint::Modules() const NOEXCEPT {
    const RTTI::FMetaDatabaseReadable db;
    return { db->Modules().Map(
        [](const RTTI::FMetaModule* module_) {
            return ToString(module_->Name());
        })
    };
}
//----------------------------------------------------------------------------
PRTTIModule FRTTIEndpoint::Module(const FString& name) const {
    if (RTTI::FName::IsValidToken(name)) {
        if (const RTTI::FMetaModule* module = RTTI::FMetaDatabaseReadable{}->ModuleIFP(name)) {
            PRTTIModule result;
            result = NEW_RTTI(FRTTIModule, *_cache.LockExclusive(), *module);
            return result;
        }
    }

    PPE_THROW_IT(RTTI::FRTTIException("module not found"));
}
//----------------------------------------------------------------------------
// Classes
//----------------------------------------------------------------------------
FRTTIEndpoint::TList<FString> FRTTIEndpoint::Classes() const NOEXCEPT {
    const RTTI::FMetaDatabaseReadable db;
    return { db->Classes().Keys().Map(
        [](const RTTI::FName& name) {
            return ToString(name);
        })
    };
}
//----------------------------------------------------------------------------
PRTTIClass FRTTIEndpoint::Class(const FString& name) const {
    if (RTTI::FName::IsValidToken(name)) {
        if (const RTTI::FMetaClass* class_ = RTTI::FMetaDatabaseReadable{}->ClassIFP(name)) {
            PRTTIClass result;
            result = FindOrAddClass_(*_cache.LockExclusive(), *class_);
            return result;
        }

        PPE_THROW_IT(RTTI::FRTTIException("invalid token"));
    }

    PPE_THROW_IT(RTTI::FRTTIException("class not found"));
}
//----------------------------------------------------------------------------
// Enums
//----------------------------------------------------------------------------
FRTTIEndpoint::TList<FString> FRTTIEndpoint::Enums() const NOEXCEPT {
    const RTTI::FMetaDatabaseReadable db;
    return { db->Enums().Keys().Map(
        [](const RTTI::FName& name) {
            return ToString(name);
        })
    };
}
//----------------------------------------------------------------------------
PRTTIEnum FRTTIEndpoint::Enum(const FString& name) const {
    if (RTTI::FName::IsValidToken(name)) {
        if (const RTTI::FMetaEnum* enum_ = RTTI::FMetaDatabaseReadable{}->EnumIFP(name)) {
            PRTTIEnum result;
            result = FindOrAddEnum_(*_cache.LockExclusive(), *enum_);
            return result;
        }

        PPE_THROW_IT(RTTI::FRTTIException("invalid token"));
    }

    PPE_THROW_IT(RTTI::FRTTIException("enum not found"));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Remoting
} //!namespace PPE
