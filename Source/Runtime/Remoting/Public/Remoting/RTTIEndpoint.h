#pragma once

#include "Remoting_fwd.h"

#include "BaseEndpoint.h"
#include "RemotingEndpoint.h"

#include "RTTI_fwd.h"
#include "RTTI/Macros.h"

#include "Container/Vector.h"
#include "Thread/ThreadSafe.h"

namespace PPE {
namespace Remoting {
FWD_REFPTR(RTTIParameter);
FWD_REFPTR(RTTIFunction);
FWD_REFPTR(RTTIProperty);
FWD_REFPTR(RTTIClass);
FWD_REFPTR(RTTIEnum);
FWD_REFPTR(RTTITraits);
FWD_REFPTR(RTTITraitsScalar);
FWD_REFPTR(RTTITraitsTuple);
FWD_REFPTR(RTTITraitsList);
FWD_REFPTR(RTTITraitsDico);
FWD_REFPTR(RTTIModule);
FWD_REFPTR(RTTITransaction);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
using FRTTITraitsCache = HASHMAP(Remoting, RTTI::PTypeTraits, PRTTITraits);
//----------------------------------------------------------------------------
class PPE_REMOTING_API FRTTIParameter final : public RTTI::FMetaObject {
    RTTI_CLASS_HEADER(PPE_REMOTING_API, FRTTIParameter, RTTI::FMetaObject);

    RTTI::FName Name;
    RTTI::EParameterFlags Flags{};
    PRTTITraits Traits;

    FRTTIParameter() = default;
    FRTTIParameter(FRTTITraitsCache& cache, const RTTI::FMetaParameter& value);
};
//----------------------------------------------------------------------------
class PPE_REMOTING_API FRTTIFunction final : public RTTI::FMetaObject {
    RTTI_CLASS_HEADER(PPE_REMOTING_API, FRTTIFunction, RTTI::FMetaObject);

    RTTI::FName Name;
    RTTI::EFunctionFlags Flags{};
    PRTTITraits Result;
    VECTORINSITU(Remoting, PRTTIParameter, 4) Parameters;

    FRTTIFunction() = default;
    FRTTIFunction(FRTTITraitsCache& cache, const RTTI::FMetaFunction& value);
};
//----------------------------------------------------------------------------
class PPE_REMOTING_API FRTTIProperty final : public RTTI::FMetaObject {
    RTTI_CLASS_HEADER(PPE_REMOTING_API, FRTTIProperty, RTTI::FMetaObject);

    RTTI::FName Name;
    RTTI::EPropertyFlags Flags{};
    PRTTITraits Traits;

    FRTTIProperty() = default;
    FRTTIProperty(FRTTITraitsCache& cache, const RTTI::FMetaProperty& value);
};
//----------------------------------------------------------------------------
class PPE_REMOTING_API FRTTIClass final : public RTTI::FMetaObject {
    RTTI_CLASS_HEADER(PPE_REMOTING_API, FRTTIClass, RTTI::FMetaObject);

    RTTI::FName Name;
    RTTI::EClassFlags Flags{};
    VECTORINSITU(Remoting, PRTTIFunction, 8) Functions;
    VECTORINSITU(Remoting, PRTTIProperty, 8) Properties;
    PRTTIClass Parent;

    FRTTIClass() = default;
    FRTTIClass(FRTTITraitsCache& cache, const RTTI::FMetaClass& value);
};
//----------------------------------------------------------------------------
class PPE_REMOTING_API FRTTIEnum final : public RTTI::FMetaObject {
    RTTI_CLASS_HEADER(PPE_REMOTING_API, FRTTIEnum, RTTI::FMetaObject);

    RTTI::FName Name;
    RTTI::EEnumFlags Flags{};
    PRTTITraits Traits;
    VECTORINSITU(Remoting, RTTI::FName, 4) Keys;
    VECTORINSITU(Remoting, RTTI::FMetaEnumOrd, 4) Values;

    FRTTIEnum() = default;
    FRTTIEnum(FRTTITraitsCache& cache, const RTTI::FMetaEnum& value);
};
//----------------------------------------------------------------------------
class PPE_REMOTING_API FRTTITraits : public RTTI::FMetaObject {
    RTTI_CLASS_HEADER(PPE_REMOTING_API, FRTTITraits, RTTI::FMetaObject);

    FString Name;
    RTTI::ETypeFlags Flags{};
    RTTI::FTypeId TypeId{};
    u32 Alignment{0};
    u32 SizeInBytes{0};

protected:
    FRTTITraits() = default;
    void Initialize(const RTTI::ITypeTraits& traits);
};
//----------------------------------------------------------------------------
class PPE_REMOTING_API FRTTITraitsScalar final : public FRTTITraits {
    RTTI_CLASS_HEADER(PPE_REMOTING_API, FRTTITraitsScalar, FRTTITraits);

    PRTTIClass ObjectClass;
    PRTTIEnum EnumClass;

    FRTTITraitsScalar() = default;
    void Initialize(FRTTITraitsCache& cache, const RTTI::IScalarTraits& traits);
};
//----------------------------------------------------------------------------
class PPE_REMOTING_API FRTTITraitsTuple final : public FRTTITraits {
    RTTI_CLASS_HEADER(PPE_REMOTING_API, FRTTITraitsTuple, FRTTITraits);

    VECTORINSITU(Remoting, PRTTITraits, 3) Tuple;

    FRTTITraitsTuple() = default;
    void Initialize(FRTTITraitsCache& cache, const RTTI::ITupleTraits& traits);
};
//----------------------------------------------------------------------------
class PPE_REMOTING_API FRTTITraitsList final : public FRTTITraits {
    RTTI_CLASS_HEADER(PPE_REMOTING_API, FRTTITraitsList, FRTTITraits);

    PRTTITraits Item;

    FRTTITraitsList() = default;
    void Initialize(FRTTITraitsCache& cache, const RTTI::IListTraits& traits);
};
//----------------------------------------------------------------------------
class PPE_REMOTING_API FRTTITraitsDico final : public FRTTITraits {
    RTTI_CLASS_HEADER(PPE_REMOTING_API, FRTTITraitsDico, FRTTITraits);

    PRTTITraits Key;
    PRTTITraits Value;

    FRTTITraitsDico() = default;
    void Initialize(FRTTITraitsCache& cache, const RTTI::IDicoTraits& traits);
};
//----------------------------------------------------------------------------
class PPE_REMOTING_API FRTTIModule final : public RTTI::FMetaObject {
    RTTI_CLASS_HEADER(PPE_REMOTING_API, FRTTIModule, RTTI::FMetaObject);

    RTTI::FName Name;
    VECTOR(Remoting, PRTTIClass) Classes;
    VECTOR(Remoting, PRTTIEnum) Enums;

    FRTTIModule() = default;
    FRTTIModule(FRTTITraitsCache& cache, const RTTI::FMetaModule& transaction);
};
//----------------------------------------------------------------------------
class PPE_REMOTING_API FRTTITransaction final : public RTTI::FMetaObject {
    RTTI_CLASS_HEADER(PPE_REMOTING_API, FRTTITransaction, RTTI::FMetaObject);

    RTTI::FName Namespace;
    RTTI::ETransactionFlags Flags{};
    RTTI::ETransactionState State{};
    VECTORINSITU(Remoting, RTTI::PMetaObject, 8) TopObjects;

    FRTTITransaction() = default;
    explicit FRTTITransaction(const RTTI::FMetaTransaction& transaction);
};
//----------------------------------------------------------------------------
class PPE_REMOTING_API FRTTIEndpoint final : public FBaseEndpoint {
    RTTI_CLASS_HEADER(PPE_REMOTING_API, FRTTIEndpoint, FBaseEndpoint);
public:
    FRTTIEndpoint() NOEXCEPT;

    template <typename T>
    using TList = VECTORINSITU(Remoting, T, 2);

    /// Traits
    TList<FString> Traits() const NOEXCEPT;
    PRTTITraits Trait(const FString& name) const;

    /// Objects
    TList<FString> Objects() const NOEXCEPT;
    RTTI::PMetaObject Object(const FString& path) const;
    RTTI::FAny Object_Function(const FString& path, const FString& function, const VECTOR(Remoting, RTTI::FAny)& args) const;
    RTTI::FAny Object_Property(const FString& path, const FString& property) const;

    /// Transaction
    TList<FString> Namespaces() const NOEXCEPT;
    TList<PRTTITransaction> Transactions(const FString& namespace_) const;

    /// Modules
    TList<FString> Modules() const NOEXCEPT;
    PRTTIModule Module(const FString& name) const;

    /// Classes
    TList<FString> Classes() const NOEXCEPT;
    PRTTIClass Class(const FString& name) const;

    /// Enums
    TList<FString> Enums() const NOEXCEPT;
    PRTTIEnum Enum(const FString& name) const;

private:
    mutable TThreadSafe<FRTTITraitsCache, EThreadBarrier::CriticalSection> _cache;

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Remoting
} //!namespace PPE
